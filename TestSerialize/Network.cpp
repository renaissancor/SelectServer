#include "stdafx.h"
#include "Session.h"
#include "Network.h"
#include "Logic.h" 

// Network.cpp 

void Network::BuildFDSets() noexcept {
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	FD_SET(_hListenSocket, &_readSet);
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
			if(FD_ISSET(session.GetSocket(), &rdSet)) {
				session.RecvTCP();
				session.Receive(); 
			}
		} 

		// Update Game Logic Here 
		logic.ProcessPackets(); 

		for (int i = 0; i < SESSION_MAX; ++i) {
			Session& session = _sessions[i];
			if(FD_ISSET(session.GetSocket(), &wrSet)) {
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
	int acceptedCount = 0; 
	for (;;) {

	}
}
