#include "stdafx.h"
#include "Session.h"
#include "Network.h"
#include "Logic.h" 

// Network.cpp 

void Network::UpdateMaxSocketAfterDisconnection(SOCKET disconnectedSocket) noexcept {
	if (disconnectedSocket != _hMaxSocket) return;
	SOCKET newMaxSocket = _hListenSocket;
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.GetSocket() != INVALID_SOCKET) {
			if (session.GetSocket() > newMaxSocket) {
				newMaxSocket = session.GetSocket();
			}
		}
	}
	_hMaxSocket = newMaxSocket;
}

int Network::GetNewSessionIndex() noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		_lastSessionIndex = (_lastSessionIndex + 1) % SESSION_MAX;
		if (_sessions[_lastSessionIndex].GetSocket() == INVALID_SOCKET) {
			return _lastSessionIndex;
		}
	}
	return -1;
}

void Network::BuildFDSets() noexcept {
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	if (_hListenSocket != INVALID_SOCKET) {
		FD_SET(_hListenSocket, &_readSet);
	}
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		SOCKET sessionSocket = session.GetSocket();
		if (sessionSocket != INVALID_SOCKET) {
			FD_SET(sessionSocket, &_readSet);
			RingBuffer& sendBuffer = session.GetSendBuffer();
			if (!sendBuffer.IsEmpty()) {
				FD_SET(sessionSocket, &_writeSet);
			}
		}
	}
}

void Network::RunEngine() noexcept {
	Logic& logic = Logic::GetInstance(); 
	for(;;) {
		BuildFDSets(); 
		fd_set rdSet = _readSet;
		fd_set wrSet = _writeSet; 
		int n = ::select(0, &rdSet, &wrSet, nullptr, nullptr);

		if (FD_ISSET(_hListenSocket, &rdSet)) {
			AcceptConnections(); // accept as many as possible (non-blocking)
		}
		
		for (int i = 0; i < SESSION_MAX; ++i) {
			Session& session = _sessions[i];
			SOCKET socket = session.GetSocket();
			if (socket == INVALID_SOCKET) continue;
			if(FD_ISSET(session.GetSocket(), &rdSet)) {
				session.RecvTCP();
				session.Receive(); 
			}
		} 

		// Update Game Logic Here 
		logic.ProcessPackets(); 

		for (int i = 0; i < SESSION_MAX; ++i) {
			Session& session = _sessions[i];
			SOCKET s = session.GetSocket();
			if (s == INVALID_SOCKET) continue;
			if (FD_ISSET(s, &wrSet)) {
				session.SendTCP();
			}
		}
	}
}

void Network::Shutdown() noexcept {
	::closesocket(_hListenSocket);
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.GetSocket() != INVALID_SOCKET) {
			session.Close();
		}
	}
	::WSACleanup();
}

int Network::AcceptConnections() noexcept {
	Logic& logic = Logic::GetInstance(); 
	int acceptedCount = 0; 
	for (;;) {
		SOCKADDR_IN clientAddr = { 0 };
		int clientAddrLen = sizeof(clientAddr);
		SOCKET hNewSocket = ::accept(_hListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);

		if (hNewSocket == INVALID_SOCKET) {
			int wsaError = WSAGetLastError();
			if (wsaError != WSAEWOULDBLOCK) _WSAGetLastErrorResult = wsaError;
			return acceptedCount;
		}

		int newSessionIndex = GetNewSessionIndex();

		if (newSessionIndex == -1) { // Session Full 
			::closesocket(hNewSocket);
			continue;
		}

		UpdateMaxSocketAfterConnection(hNewSocket); 
		acceptedCount++; 

		u_long nonBlockingMode = 1; 
		::ioctlsocket(hNewSocket, FIONBIO, &nonBlockingMode); // set non-blocking mode 
		int flag = 1;
		::setsockopt(hNewSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)); // disable Nagle's algorithm 
		linger so_linger = { 1 , 0 }; // { 1, 0 }; // Close by RST
		::setsockopt(hNewSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger)); 

		Session& session = _sessions[newSessionIndex]; 
		session.SetSocket(hNewSocket); 
		session.SetIndex(newSessionIndex); 

		fprintf_s(stdout, "Accepted New Connection: Session %d, Socket %llu\n",
			newSessionIndex, static_cast<unsigned long long>(hNewSocket));

		// Handle new connection in Logic Layer 7 
		logic.OnPlayerConnected(newSessionIndex); 
	}
}

void Network::DisconnectSession(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;
	Session& session = _sessions[sessionIndex];
	SOCKET oldSock = session.GetSocket();
	if (oldSock == INVALID_SOCKET) return;

	session.Close();
	UpdateMaxSocketAfterDisconnection(oldSock); // use old socket value
	fprintf_s(stdout, "Disconnected Session %d\n", sessionIndex);
	Logic::GetInstance().OnPlayerDisconnected(sessionIndex);
}
