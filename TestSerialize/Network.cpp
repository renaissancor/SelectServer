#include "stdafx.h"
#include "Session.h"
#include "Network.h"
#include "Express.h" 
#include "Logic.h" 

// Network.cpp 


bool Network::Initialize() noexcept {
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
	so_linger.l_onoff = 1; // Enable Linger, end by RST 
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

void Network::Poll() noexcept {
	fd_set rdSet = _readSet;
	fd_set wrSet = _writeSet;

	timeval timeout = { 0, 0 };
	int n = ::select(0, &rdSet, &wrSet, nullptr, &timeout);

	if (n == SOCKET_ERROR) return;

	if (FD_ISSET(_hListenSocket, &rdSet)) {
		AcceptConnections();
	}

	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.GetSocket() == INVALID_SOCKET) continue;

		if (FD_ISSET(session.GetSocket(), &rdSet)) {
			session.RecvTCP();
			session.Receive();
		}
	}
}

void Network::FlushAll() noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.GetSocket() == INVALID_SOCKET) continue;
		if (!session.GetSendBuffer().IsEmpty()) {
			session.SendTCP();
		}
	}
}

void Network::RunEngine() noexcept {

	Logic& logic = Logic::GetInstance(); 
	Express& express = Express::GetInstance();

	LARGE_INTEGER freq, start, now;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	const long long ticksPerFrame = freq.QuadPart / 60;
	int frameCount = 0;

	timeBeginPeriod(1); 
	for(;;) {
		QueryPerformanceCounter(&now);
		long long targetTick = start.QuadPart + (frameCount * freq.QuadPart) / 60;

		BuildFDSets(); 
		Poll(); 

		express.ProcessRecvPackets(); 
		logic.Update(); 

		// Flush outgoing packets
		FlushAll(); 

		QueryPerformanceCounter(&now);
		if (now.QuadPart < targetTick) {
			long long sleepTicks = targetTick - now.QuadPart;
			DWORD sleepMs = (DWORD)(sleepTicks * 1000 / freq.QuadPart);
			if (sleepMs > 0) Sleep(sleepMs);
		}

	} // Game Loop 
	timeEndPeriod(1);
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
