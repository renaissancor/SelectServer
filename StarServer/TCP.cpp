#include "stdafx.h"
#include "TCP.h"
#include "Game.h" 

TCP::Manager::Manager() noexcept = default;
TCP::Manager::~Manager() noexcept = default; 

bool TCP::Manager::Initialize() noexcept {
	
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

	_WSAStartupResult = ::WSAStartup(MAKEWORD(2, 2), &_wsaData);
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
	so_linger.l_onoff = 1; // Enable linger option, end by RST 
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
	
	int i = 0;
	try {
		for (; i < SESSION_MAX; ++i) {
			_sessions[i] = new Session();
		}
	}
	catch (...) {
		for (int j = 0; j < i; ++j) {
			delete _sessions[j];
			_sessions[j] = nullptr;
		}
		return Helper();
	}

	return true;
}

int TCP::Manager::AcceptNewConnection() noexcept {
	for (int newClient = 0;; ++newClient) {
		
		SOCKADDR_IN clientAddr = { 0 };
		int clientAddrLen = sizeof(clientAddr); 
		SOCKET hNewSocket = ::accept(_hListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);

		if (hNewSocket == INVALID_SOCKET) {
			int wsaError = WSAGetLastError();
			if (wsaError != WSAEWOULDBLOCK) _WSAGetLastErrorResult = wsaError;
			return newClient;
		}

		int newSessionIndex = -1; 
		for (int i = 0; i < SESSION_MAX; ++i) {
			int index = GetNewSessionIndex(); 
			if (_sessions[index]->hSocket == INVALID_SOCKET) {
				newSessionIndex = index;
				break;
			}
		} // Find available session slot 

		if (newSessionIndex == -1) {
			::closesocket(hNewSocket); 
			continue; // No available session slot 
		}

		Session &newSession = *_sessions[newSessionIndex];

		newSession.hSocket = hNewSocket;
		memcpy_s(&newSession.address, sizeof(sockaddr),
			&clientAddr, sizeof(clientAddr));

		newSession.recvBuffer.ClearBuffer();
		newSession.sendBuffer.ClearBuffer();

		FD_SET(hNewSocket, &_readSet);
		if (hNewSocket > _hMaxSocket) _hMaxSocket = hNewSocket; 
			
		u_long nonBlockingMode = 1;
		::ioctlsocket(hNewSocket, FIONBIO, &nonBlockingMode);
		
		char LoginMessage[16] = { 0 };
		*reinterpret_cast<int*>(&LoginMessage[0]) = static_cast<int>(Game::Event::Type::LOGIN);
		*reinterpret_cast<int*>(&LoginMessage[4]) = newSessionIndex;
		Game::EnqueueEvent(LoginMessage, sizeof(LoginMessage)); 

		char CreateMessage[16] = { 0 };
		*reinterpret_cast<int*>(&CreateMessage[0]) = static_cast<int>(Game::Event::Type::CREATE);
		*reinterpret_cast<int*>(&CreateMessage[4]) = newSessionIndex;
		Game::EnqueueEvent(CreateMessage, sizeof(CreateMessage));
	}
	return 0; // Should not reach here 
}

void TCP::Manager::HandleDisconnection(int sessionIndex) noexcept {
	Session& session = *_sessions[sessionIndex];
	if (session.hSocket != INVALID_SOCKET) {
		::closesocket(session.hSocket);
		session.hSocket = INVALID_SOCKET;
	}
	session.recvBuffer.ClearBuffer();
	session.sendBuffer.ClearBuffer();
}

int TCP::Manager::UpdateSet() noexcept {
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	FD_SET(_hListenSocket, &_readSet); 

	for (int i = 0; i < SESSION_MAX; ++i) {
		Session* session = _sessions[i];
		if (session->hSocket != INVALID_SOCKET) {
			FD_SET(session->hSocket, &_readSet);
			if (session->sendBuffer.GetUsedSize() > 0) {
				FD_SET(session->hSocket, &_writeSet);
			}
		}
	}
	timeval nonBlockTimeout = { 0, 0 };

	int selectCount = ::select((int)(_hMaxSocket + 1), &_readSet, &_writeSet, nullptr, &nonBlockTimeout);

	if (selectCount == SOCKET_ERROR) {
		_WSAGetLastErrorResult = WSAGetLastError();
		return -1;
	}
	return selectCount;
}

void TCP::Manager::ProcessSessionRecv(int sessionIndex) noexcept {
	Session* session = _sessions[sessionIndex];
	if (session->hSocket == INVALID_SOCKET || !FD_ISSET(session->hSocket, &_readSet)) return;

	char recvBuffer[128] = { 0 };
	int bytesReceived = ::recv(session->hSocket, recvBuffer, sizeof(recvBuffer), 0);

	if (bytesReceived > 0) {
		session->recvBuffer.Enqueue(recvBuffer, bytesReceived);
	}

	else {
		int wsaError = WSAGetLastError();
		if (bytesReceived == 0 || wsaError != WSAEWOULDBLOCK) {
			_WSAGetLastErrorResult = wsaError;
			HandleDisconnection(sessionIndex);

			char RemoveMessage[16] = { 0 };
			*reinterpret_cast<int*>(&RemoveMessage[0]) = static_cast<int>(Game::Event::Type::REMOVE);
			*reinterpret_cast<int*>(&RemoveMessage[4]) = sessionIndex; 
			Game::EnqueueEvent(RemoveMessage, sizeof(RemoveMessage));
		}
	}
}


void TCP::Manager::ProcessSessionSend(int sessionIndex) noexcept {
	Session* session = _sessions[sessionIndex];
	if (session->hSocket == INVALID_SOCKET || !FD_ISSET(session->hSocket, &_writeSet)) return;

	char sendBuffer[4096] = { 0 };
	int bytesToSend = session->sendBuffer.Dequeue(sendBuffer, sizeof(sendBuffer));

	if (bytesToSend > 0) {
		int bytesSent = ::send(session->hSocket, sendBuffer, bytesToSend, 0);

		if (bytesSent == SOCKET_ERROR) {
			int wsaError = WSAGetLastError();
			if (wsaError != WSAEWOULDBLOCK) {
				_WSAGetLastErrorResult = wsaError;
				HandleDisconnection(sessionIndex);
			}
		}
		else if (bytesSent < bytesToSend) {
			session->sendBuffer.Enqueue(sendBuffer + bytesSent, bytesToSend - bytesSent);
		}
	}
}

void TCP::Manager::ProcessPacket() noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session* session = _sessions[i];
		if (session->hSocket == INVALID_SOCKET) continue;
		while (session->recvBuffer.GetUsedSize() >= sizeof(Protocol::Packet)) {
			Protocol::Packet pkt;
			session->recvBuffer.Peek(reinterpret_cast<char*>(&pkt), sizeof(Protocol::Packet));
			session->recvBuffer.Dequeue(reinterpret_cast<char*>(&pkt), sizeof(Protocol::Packet));
			Game::Manager::GetInstance().EnqueuePacketAsEvent(pkt);
		}
	}
}

void TCP::Manager::Update() noexcept {
	int selectCount = UpdateSet();
	if (selectCount <= 0) return; 

	if (FD_ISSET(_hListenSocket, &_readSet)) {
		AcceptNewConnection();
	}

	for (int i = 0; i < SESSION_MAX; ++i) {
		if (_sessions[i]->hSocket == INVALID_SOCKET) continue;

		if (FD_ISSET(_sessions[i]->hSocket, &_readSet)) {
			ProcessSessionRecv(i);
		}

		if (_sessions[i]->sendBuffer.GetUsedSize() > 0 && FD_ISSET(_sessions[i]->hSocket, &_writeSet)) {
			ProcessSessionSend(i);
		}
	}
	ProcessPacket();
}

void TCP::Manager::Shutdown() noexcept {
	if (_hListenSocket != INVALID_SOCKET) {
		::closesocket(_hListenSocket);
		_hListenSocket = INVALID_SOCKET;
	}
	for (int i = 0; i < SESSION_MAX; ++i) {
		if (_sessions[i]) {
			if (_sessions[i]->hSocket != INVALID_SOCKET) {
				::closesocket(_sessions[i]->hSocket);
				_sessions[i]->hSocket = INVALID_SOCKET;
			}
			delete _sessions[i];
			_sessions[i] = nullptr;
		}
	}
	::WSACleanup();
}