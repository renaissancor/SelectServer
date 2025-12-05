#include "stdafx.h"

#include "Session.h"
#include "Logic.h"

// Session.cpp 

void Session::Receive() noexcept // recv to Game L7 
{
	RingBuffer& recvBuffer = _recvBuffer; 
	Logic& logic = Logic::GetInstance(); 

	for(;;) {
		PacketHeader packetHeader = {};
		// peek header and check if enough data is available
		size_t peekHeaderBytes = recvBuffer.Peek(
			reinterpret_cast<char*>(&packetHeader),
			sizeof(PacketHeader)
		);
		if (peekHeaderBytes < sizeof(PacketHeader)) {
			return; // Not enough data for header 
		}
		// compute total packet size and validate it
		size_t payloadSize = static_cast<size_t>(packetHeader.size);
		size_t totalBytes = sizeof(PacketHeader) + payloadSize;

		if (payloadSize == 0 || totalBytes > 256) {
			fprintf(stderr, "Malformed packet detected (size=%zu) from session %d\n", payloadSize, _index);
			Close();
			return;
		}

		if (recvBuffer.GetUsedSize() < totalBytes) {
			return; // Not enough data for full packet 
		}
		
		Packet packet = {}; 
		size_t deqBytes = recvBuffer.Dequeue(
			packet.GetBuffer(),
			totalBytes
		);

		if (deqBytes < totalBytes) {
			fprintf(stderr, "Dequeue error: expected %zu got %zu\n", totalBytes, deqBytes);
			Close(); 
			return; // should NOT happen, malfunctioned dequeue 
		}
		packet.SetUsedSize(totalBytes); 
		logic.EnqueuePacket(_index, packet); 
	}
}

void Session::SendTCP() noexcept {
	RingBuffer& sendBuffer = _sendBuffer;
	for (;;) {
		if (sendBuffer.IsEmpty()) return;
		int directDequeueSize = static_cast<int>(sendBuffer.DirectDequeueSize());
		if (directDequeueSize == 0) return; // safety

		char* directDequeuePtr = sendBuffer.GetDirectDequeuePtr();
		int sendBytes = ::send(_socket, directDequeuePtr, directDequeueSize, 0);

		if (sendBytes > 0) {
			sendBuffer.MoveHead(sendBytes);
			continue; // try to send more
		}

		if (sendBytes == SOCKET_ERROR) {
			int errorCode = WSAGetLastError();
			if (errorCode == WSAEWOULDBLOCK) {
				// can't send now; try again when socket becomes writable
				return;
			}
			else {
				_wsaError = errorCode;
				// Register disconnection event
				Close(); // or mark for close
				return;
			}
		}

		if (sendBytes == 0) {
			// remote gracefully closed
			_wsaError = 0;
			Close();
			return;
		}
	}
}

void Session::RecvTCP() noexcept {
	RingBuffer& recvBuffer = _recvBuffer;
	int directEnqueueSize = static_cast<int>(recvBuffer.DirectEnqueueSize());
	if (directEnqueueSize <= 0) return;

	char* directEnqueuePtr = recvBuffer.GetDirectEnqueuePtr();
	int recvBytes = ::recv(_socket, directEnqueuePtr, directEnqueueSize, 0);

	if (recvBytes > 0) {
		recvBuffer.MoveTail(recvBytes);
		return;
	}

	if (recvBytes == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		if (errorCode == WSAEWOULDBLOCK) {
			return;
		}
		else {
			_wsaError = errorCode;
			Close();
		}
		return;
	}

	if (recvBytes == 0) {
		// peer closed connection
		_wsaError = 0;
		Close();
		return;
	}
}


void Session::Close() noexcept
{
	if (_socket == INVALID_SOCKET) return;
	::closesocket(_socket);
	_socket = INVALID_SOCKET;
	_recvBuffer.ClearBuffer();
	_sendBuffer.ClearBuffer();
}


