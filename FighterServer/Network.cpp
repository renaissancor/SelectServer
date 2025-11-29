#include "stdafx.h"
#include "Network.h" 

Network::Manager::Manager() {

}

Network::Manager::~Manager() {

}


bool Network::Manager::Initialize() noexcept {
	int _WSAStartupResult = 0;
	int _ListenSocketCreateResult = 0;
	int _setsockoptNagleResult = 0;
	int _setsockoptLingerResult = 0;
	int _ioctlsocketResult = 0;
	int _ListenSocketBindResult = 0;
	int _ListenSocketListenResult = 0;
	
	auto Helper = [&]() noexcept -> bool {
		_WSAGetLastErrorResult = WSAGetLastError();
		::closesocket(_hListenSocket);
		::WSACleanup();
		return false;
		};

	_WSAStartupResult = ::WSAStartup(MAKEWORD(2, 2), &_wsa);
	if (_WSAStartupResult != 0) return false;

	_hListenSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_hListenSocket == INVALID_SOCKET) {
		_WSAGetLastErrorResult = WSAGetLastError();
		::WSACleanup();
		return false;
	}

	int flag = 1;
	_setsockoptNagleResult = ::setsockopt(_hListenSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	if (_setsockoptNagleResult == SOCKET_ERROR)
		return Helper();

	u_long nonBlockingMode = 1;
	_ioctlsocketResult = ::ioctlsocket(_hListenSocket, FIONBIO, &nonBlockingMode);
	if (_ioctlsocketResult == SOCKET_ERROR)
		return Helper();

	linger so_linger = { 0, };
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	_setsockoptLingerResult = ::setsockopt(_hListenSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (_setsockoptLingerResult == SOCKET_ERROR)
		return Helper();

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(SERVER_PORT);
	_serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	_ListenSocketBindResult = ::bind(_hListenSocket,
		(SOCKADDR*)&_serverAddr, sizeof(_serverAddr));

	if (_ListenSocketBindResult == SOCKET_ERROR)
		return Helper();

	_ListenSocketListenResult = ::listen(_hListenSocket, SOMAXCONN);
	if (_ListenSocketListenResult == SOCKET_ERROR)
		return Helper();

	FD_ZERO(&_readSet);
	FD_SET(_hListenSocket, &_readSet);

	_hMaxSocket = _hListenSocket;

	return true;
}

void Network::Manager::RenewHighestSocket() noexcept {
	_hMaxSocket = _hListenSocket;
	for (const auto& session : _sessions) {
		if (session.socket == INVALID_SOCKET) continue;
		if (session.socket > _hMaxSocket) _hMaxSocket = session.socket;
	}
}

void Network::Manager::BuildFDSets() noexcept {
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);

	FD_SET(_hListenSocket, &_readSet);

	for (int i = 0; i < SESSION_MAX; ++i) {
		SOCKET sock = _sessions[i].socket;
		if (sock != INVALID_SOCKET) {
			FD_SET(sock, &_readSet);
			if (_sessions[i].sendBuffer.GetUsedSize() > 0) {
				FD_SET(sock, &_writeSet);
			}
		}
	}
}

int Network::Manager::AcceptNewConnection() noexcept {
	for (int newSession = 0;; ++newSession) {

		SOCKADDR_IN clientAddr = { 0 };
		int clientAddrLen = sizeof(clientAddr);
		SOCKET hNewSocket = ::accept(_hListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
		
		if (hNewSocket == INVALID_SOCKET) {
			int wsaError = WSAGetLastError();
			if (wsaError != WSAEWOULDBLOCK) _WSAGetLastErrorResult = wsaError;
			return newSession;
		}

		int newSessionIndex = GetAvailableSessionIndex(); 
		
		if (newSessionIndex == -1) { // Session Full 
			::closesocket(hNewSocket); 
			continue;
		}

		u_long nonBlockingMode = 1;
		::ioctlsocket(hNewSocket, FIONBIO, &nonBlockingMode);

		int flag = 1;
		::setsockopt(hNewSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

		linger so_linger = { 1, 0 }; // Close by RST 
		::setsockopt(hNewSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
		
		Session& session = _sessions[newSessionIndex]; 
		session.socket = hNewSocket; 

		PacketHeader header = {}; 
		PacketSCCreateMyCharacter packetSCCreateMyCharacter = {};
		header.code = 0x89;
		header.size = sizeof(PacketSCCreateMyCharacter); 
		fprintf_s(stdout, "%zu\n", sizeof(PacketSCCreateMyCharacter)); 
		header.type = PACKET_SC_CREATE_MY_CHARACTER;

		packetSCCreateMyCharacter.id = newSessionIndex; 
		packetSCCreateMyCharacter.direction = PACKET_MOVE_DIR_LL;
		packetSCCreateMyCharacter.x = 100;
		packetSCCreateMyCharacter.y = 100;
		packetSCCreateMyCharacter.hp = 100; 

		session.sendBuffer.Enqueue(
			reinterpret_cast<const char*>(&header),
			sizeof(PacketHeader)
		);

		session.sendBuffer.Enqueue(
			reinterpret_cast<const char*>(&packetSCCreateMyCharacter),
			sizeof(PacketSCCreateMyCharacter)
		);

		PacketSCCreateOtherCharacter packetSCCreateOtherCharacter = {}; 

		packetSCCreateOtherCharacter.header.code = 0x89;
		packetSCCreateOtherCharacter.header.size = sizeof(PacketSCCreateOtherCharacter);
		packetSCCreateOtherCharacter.header.type = PACKET_SC_CREATE_OTHER_CHARACTER;

		packetSCCreateOtherCharacter.id = newSessionIndex; 
		packetSCCreateOtherCharacter.direction = PACKET_MOVE_DIR_LL;
		packetSCCreateOtherCharacter.x = 574;
		packetSCCreateOtherCharacter.y = 320;
		packetSCCreateOtherCharacter.hp = 100;

		for (int i = 0; i < SESSION_MAX; ++i) {
			if (i == newSessionIndex) continue; 
			Session& otherSession = _sessions[i]; 
			if (otherSession.socket == INVALID_SOCKET) continue; 
			otherSession.sendBuffer.Enqueue(
				reinterpret_cast<const char*>(&packetSCCreateOtherCharacter),
				sizeof(PacketSCCreateOtherCharacter)
			);
		}

		RenewHighestSocket(); // for select 
	}
}

void Network::Manager::HandleDisconnection(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;
	auto& session = _sessions[sessionIndex];
	if (session.socket != INVALID_SOCKET) {
		::closesocket(session.socket);
		session.socket = INVALID_SOCKET;
		session.recvBuffer.ClearBuffer();
		session.sendBuffer.ClearBuffer();
	}
}

void Network::Manager::Receive(int sessionIndex) noexcept {
	Session& session = _sessions[sessionIndex]; 
	// if (session.socket == INVALID_SOCKET) return; 
	// if (FD_ISSET(session.socket, &_readSet) == 0) return; 

	int bytesCanRecv = static_cast<int>(session.recvBuffer.DirectEnqueueSize()); 
	char* recvPtr = reinterpret_cast<char*>(session.recvBuffer.GetBufferTailPtr()); 

	int bytesRecv = ::recv(session.socket, recvPtr, bytesCanRecv, 0); 

	if (bytesRecv == SOCKET_ERROR) {
		_WSAGetLastErrorResult = WSAGetLastError();
		if (_WSAGetLastErrorResult == WSAEWOULDBLOCK) return; 
		else {
			HandleDisconnection(sessionIndex); 
			return; 
		}
	}
	else if (bytesRecv == 0) {
		HandleDisconnection(sessionIndex); 
		return; 
	}
	else {
		session.recvBuffer.MoveTail(static_cast<size_t>(bytesRecv)); 
	}
}

void Network::Manager::Poll() noexcept {
	timeval tv = { 0, 0 };
	int result = ::select(
		static_cast<int>(_hMaxSocket) + 1, 
		&_readSet, 
		&_writeSet, 
		NULL,
		&tv
	);

	if (result == SOCKET_ERROR) return; 

	if (result > 0) {
		if (FD_ISSET(_hListenSocket, &_readSet)) {
			AcceptNewConnection();
		}
		ReceiveAll();
	}

}

void Network::Manager::ReceiveAll() noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		auto& session = _sessions[i];
		if (session.socket == INVALID_SOCKET) continue; 
		if (FD_ISSET(session.socket, &_readSet) == 0) continue; 
		Receive(i); 
	}
}

void Network::Manager::ProcessRecvData() noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i]; 
		if (session.socket == INVALID_SOCKET) continue; 
		
		while (session.recvBuffer.GetUsedSize() >= sizeof(PacketHeader)) {
			PacketHeader header = { 0, }; 
			int peekBytes = session.recvBuffer.Peek(
				reinterpret_cast<char*>(&header),
				sizeof(PacketHeader)
			);
			if (peekBytes < sizeof(PacketHeader)) break;
			uint8_t totalPacketSize = header.size;

			if (totalPacketSize < sizeof(PacketHeader) 
				|| totalPacketSize > 256 /* max safe buffer size */) {
				HandleDisconnection(i);
				break;
			}

			if (session.recvBuffer.GetUsedSize() < totalPacketSize) {
				break; 
			}

			char buffer[256];
			session.recvBuffer.Dequeue(buffer, totalPacketSize);

		}



	}
}

void Network::Manager::Flush() noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		auto& session = _sessions[i];
		if (session.socket == INVALID_SOCKET) continue; 
		if (FD_ISSET(session.socket, &_writeSet) == 0) continue; 
		
		while (session.sendBuffer.GetUsedSize() > 0) {
			int bytesToSend = static_cast<int>(session.sendBuffer.GetUsedSize()); 
			const char* sendPtr = reinterpret_cast<const char*>(session.sendBuffer.GetBufferHeadPtr());
			int bytesSent = ::send(session.socket, sendPtr, bytesToSend, 0);

			if (bytesSent == SOCKET_ERROR) {
				_WSAGetLastErrorResult = WSAGetLastError(); 
				if (_WSAGetLastErrorResult == WSAEWOULDBLOCK) break; 
				else {
					HandleDisconnection(i); 
					break; 
				}
			}

			if (bytesSent > 0) 
				session.sendBuffer.MoveHead(static_cast<size_t>(bytesSent)); 
			if (bytesSent == 0) break; 
		}
	}
}



void Network::Manager::Shutdown() noexcept {
	if (_hListenSocket != INVALID_SOCKET) {
		::closesocket(_hListenSocket);
		_hListenSocket = INVALID_SOCKET;
	}
	::WSACleanup();
}




