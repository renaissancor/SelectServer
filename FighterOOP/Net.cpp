#include "stdafx.h"
#include "Net.h"
#include "Game.h" 

void Net::HandleCSMoveStart(int sessionIndex, const PayloadCSMoveStart* payload) noexcept
{
	Game::Manager& game = Game::Manager::GetInstance();
	game.StartMovePlayer(sessionIndex, payload);

	PacketSCMoveStart packetSCMoveStart = {};
	game.SetPacketSCMoveStart(sessionIndex, packetSCMoveStart); 
	Net::Manager::GetInstance().BroadcastExcept(
		sessionIndex,
		reinterpret_cast<const char*>(&packetSCMoveStart),
		sizeof(PacketSCMoveStart)
	);
}

void Net::HandleCSMoveStop(int sessionIndex, const PayloadCSMoveStop* payload) noexcept
{
	Game::Manager& game = Game::Manager::GetInstance();
	game.StopMovePlayer(sessionIndex, payload);
	PacketSCMoveStop packetSCMoveStop = {};
	game.SetPacketSCMoveStop(sessionIndex, packetSCMoveStop); 
	Net::Manager::GetInstance().BroadcastExcept(
		sessionIndex,
		reinterpret_cast<const char*>(&packetSCMoveStop),
		sizeof(PacketSCMoveStop)
	);
}

void Net::HandleCSAttack1(int sessionIndex, const PayloadCSAttack1* payload) noexcept
{
	Game::Manager& game = Game::Manager::GetInstance();
	game.Attack1Player(sessionIndex, payload); 
	// Prepare PacketSCAttack1
	PacketSCAttack1 packetSCAttack1 = {};
	game.SetPacketSCAttack1(sessionIndex, packetSCAttack1); 
	Net::Manager::GetInstance().BroadcastExcept(
		sessionIndex,
		reinterpret_cast<const char*>(&packetSCAttack1),
		sizeof(PacketSCAttack1)
	);
}

void Net::HandleCSAttack2(int sessionIndex, const PayloadCSAttack2* payload) noexcept
{
	Game::Manager& game = Game::Manager::GetInstance();
	game.Attack2Player(sessionIndex, payload);
	// Prepare PacketSCAttack2
	PacketSCAttack2 packetSCAttack2 = {};
	game.SetPacketSCAttack2(sessionIndex, packetSCAttack2); 
	Net::Manager::GetInstance().BroadcastExcept(
		sessionIndex,
		reinterpret_cast<const char*>(&packetSCAttack2),
		sizeof(PacketSCAttack2)
	);
}

void Net::HandleCSAttack3(int sessionIndex, const PayloadCSAttack3* payload) noexcept
{
	Game::Manager& game = Game::Manager::GetInstance();
	game.Attack3Player(sessionIndex, payload);
	// Prepare PacketSCAttack3
	PacketSCAttack3 packetSCAttack3 = {};
	game.SetPacketSCAttack3(sessionIndex, packetSCAttack3); 
	Net::Manager::GetInstance().BroadcastExcept(
		sessionIndex,
		reinterpret_cast<const char*>(&packetSCAttack3),
		sizeof(PacketSCAttack3)
	);
}

void Net::Manager::Unicast(int sessionIndex, const char* data, int size) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;
	Session& session = _sessions[sessionIndex];
	if (session.socket == INVALID_SOCKET) return;
	session.sendBuffer.Enqueue(data, size);
	FD_SET(session.socket, &_writeSet);
}

void Net::Manager::Broadcast(const char* data, int size) noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.socket == INVALID_SOCKET) continue;
		session.sendBuffer.Enqueue(data, size);
		FD_SET(session.socket, &_writeSet);
	}
}

void Net::Manager::BroadcastExcept(int excludeSessionIndex, const char* data, int size) noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		if (i == excludeSessionIndex) continue;
		Session& session = _sessions[i];
		if (session.socket == INVALID_SOCKET) continue;
		session.sendBuffer.Enqueue(data, size);
		FD_SET(session.socket, &_writeSet);
	}
}

void Net::Manager::UpdateMaxSocketAfterDisconnection
	(SOCKET disconnectedSocket) noexcept {
	if (disconnectedSocket != _hMaxSocket) return;
	SOCKET newMaxSocket = _hListenSocket;
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.socket != INVALID_SOCKET) {
			if (session.socket > newMaxSocket) {
				newMaxSocket = session.socket;
			}
		}
	}
	_hMaxSocket = newMaxSocket;
}

bool Net::Manager::Initialize() noexcept {
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

int Net::Manager::AcceptNewConnections() noexcept {
	int acceptedCount = 0;
	while (true) {

		SOCKADDR_IN clientAddr = { 0 };
		int clientAddrLen = sizeof(clientAddr);
		SOCKET hNewSocket = ::accept(_hListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);

		if (hNewSocket == INVALID_SOCKET) {
			int wsaError = WSAGetLastError();
			if (wsaError != WSAEWOULDBLOCK) _WSAGetLastErrorResult = wsaError;
			
			return acceptedCount;
		}

		int newSessionIndex = GetAvailableSessionIndex();

		if (newSessionIndex == -1) { // Session Full 
			::closesocket(hNewSocket);
			continue;
		}

		UpdateMaxSocketAfterConnection(hNewSocket);
		acceptedCount++;

		u_long nonBlockingMode = 1;
		::ioctlsocket(hNewSocket, FIONBIO, &nonBlockingMode);

		int flag = 1;
		::setsockopt(hNewSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

		linger so_linger = { 1 , 0 }; // { 1, 0 }; // Close by RST 
		::setsockopt(hNewSocket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));

		// TCP Setup Complete 

		Session& session = _sessions[newSessionIndex];
		session.socket = hNewSocket;

		Game::Manager& gameManager = Game::Manager::GetInstance(); 
		gameManager.ConnectPlayer(newSessionIndex);

		fprintf_s(stdout, "Accepted New Connection: Session %d, Socket %llu\n",
			newSessionIndex, static_cast<unsigned long long>(hNewSocket));

		PacketSCCreateMyCharacter packetSCCreateMyCharacter = {};
		gameManager.SetPacketSCCreateMyCharacter(newSessionIndex, packetSCCreateMyCharacter);

		session.sendBuffer.Enqueue(
			reinterpret_cast<const char*>(&packetSCCreateMyCharacter),
			sizeof(PacketSCCreateMyCharacter)
		);

		PacketSCCreateOtherCharacter packetSCCreateOtherCharacter = {};
		gameManager.SetPacketSCCreateOtherCharacter(newSessionIndex, packetSCCreateOtherCharacter);

		BroadcastExcept(
			newSessionIndex,
			reinterpret_cast<const char*>(&packetSCCreateOtherCharacter),
			sizeof(PacketSCCreateOtherCharacter)
		);

		for (int i = 0; i < SESSION_MAX; ++i) {
			if (i == newSessionIndex) continue;
			Session& otherSession = _sessions[i];
			if (otherSession.socket == INVALID_SOCKET) continue;
			PacketSCCreateOtherCharacter packetSCCreateOtherCharacterForNewSession = {};
			gameManager.SetPacketSCCreateOtherCharacter(i, packetSCCreateOtherCharacterForNewSession);
			session.sendBuffer.Enqueue(
				reinterpret_cast<const char*>(&packetSCCreateOtherCharacterForNewSession),
				sizeof(PacketSCCreateOtherCharacter)
			);
		}
	}
}

void Net::Manager::HandleDisconnection(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;
	fprintf_s(stdout, "Disconnected Session %d...\n", sessionIndex);
	Session& sessionToClose = _sessions[sessionIndex];
	if (sessionToClose.socket == INVALID_SOCKET) {
		fprintf(stderr, "HandleDisconnection called on invalid session %d\n", sessionIndex);
		return;
	}

	SOCKET socketToClose = sessionToClose.socket;
	FD_CLR(socketToClose, &_readSet);
	FD_CLR(socketToClose, &_writeSet);
	::closesocket(socketToClose);
	sessionToClose.socket = INVALID_SOCKET;
	sessionToClose.recvBuffer.ClearBuffer();
	sessionToClose.sendBuffer.ClearBuffer();
	UpdateMaxSocketAfterDisconnection(socketToClose);
	
	UpdateMaxSocketAfterDisconnection(sessionToClose.socket); 

	Game::Manager& gameManager = Game::Manager::GetInstance();
	gameManager.DisconnectPlayer(sessionIndex);
	PacketSCDeleteCharacter packetSCDeleteCharacter = {};
	gameManager.SetPacketSCDeleteCharacter(sessionIndex, packetSCDeleteCharacter);

	BroadcastExcept(
		sessionIndex,
		reinterpret_cast<const char*>(&packetSCDeleteCharacter),
		sizeof(PacketSCDeleteCharacter)
	);
}

void Net::Manager::BuildFDSets() noexcept {
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	FD_SET(_hListenSocket, &_readSet);
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.socket != INVALID_SOCKET) {
			FD_SET(session.socket, &_readSet);
			if (session.sendBuffer.IsEmpty() == false) {
				FD_SET(session.socket, &_writeSet);
			}
		}
	}
}

void Net::Manager::Poll() noexcept {
	timeval timeout = { 0, 0 }; // Non-blocking 
	fd_set readSetCopy = _readSet;
	fd_set writeSetCopy = _writeSet;
	int selectResult = ::select(
		static_cast<int>(_hMaxSocket + 1),
		&readSetCopy,
		&writeSetCopy,
		NULL,
		&timeout
	);
	if (selectResult == SOCKET_ERROR) {
		_WSAGetLastErrorResult = WSAGetLastError();
		fprintf(stderr, "select error: WSAGetLastError = %d\n", _WSAGetLastErrorResult);
		return;
	}
	if (FD_ISSET(_hListenSocket, &readSetCopy)) {
		AcceptNewConnections();
	}
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i];
		if (session.socket == INVALID_SOCKET) continue;
		if (FD_ISSET(session.socket, &readSetCopy)) {
			RecvFromTCP(i);
			ProcessReceivedData(i);
		}
		if (FD_ISSET(session.socket, &writeSetCopy)) {
			Flush(i);
		}
	}
}

void Net::Manager::FlushAll() noexcept {
	for (int i = 0; i < SESSION_MAX; ++i) {
		if (_sessions[i].socket == INVALID_SOCKET) continue;
		Flush(i);
	}
}

void Net::Manager::RecvFromTCP(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;
	Session& session = _sessions[sessionIndex];
	if (session.socket == INVALID_SOCKET) return;

	if (session.recvBuffer.GetFreeSize() == 0) {
		fprintf(stdout, "Session %d recv buffer full, skipping recv.\n", sessionIndex);
		return;
	} 

	int result = session.recvBuffer.RecvTCP(session.socket);
	
	if (result > 0) return; // Receive Successful 
	int errorCode = -result;
	if (errorCode == WSAEWOULDBLOCK) {
		return; // No data available right now in non-blocking mode 
	}
	else if (result == 0) { // (errorCode == 0) 
		fprintf(stdout, "Session %d recv returned 0 (connection closed)\n", sessionIndex);
		HandleDisconnection(sessionIndex); // Connection Closed Gracefully 
		return;
	} else {
		_WSAGetLastErrorResult = errorCode;
		fprintf(stderr, "Session %d recv error: WSAGetLastError = %d\n", sessionIndex, errorCode);
		HandleDisconnection(sessionIndex);
		return;
	}
}

void Net::Manager::ProcessReceivedData(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;
	Session& session = _sessions[sessionIndex];
	if (session.socket == INVALID_SOCKET) return; 
	Game::Manager& gameManager = Game::Manager::GetInstance();

	while (session.recvBuffer.IsEmpty() == false) {
		PacketHeader header = { 0, };
		int peekBytes = session.recvBuffer.Peek(
			reinterpret_cast<char*>(&header),
			sizeof(PacketHeader)
		);
		if (peekBytes < sizeof(PacketHeader)) break;

		if (header.code != 0x89) {
			fprintf_s(stderr, "Session %d invalid header.code=0x%02X -> disconnect\n", sessionIndex, header.code);
			HandleDisconnection(sessionIndex);
			return;
		}

		uint8_t payloadSize = header.size;
		size_t totalSize = sizeof(PacketHeader) + static_cast<size_t>(payloadSize);
		if (session.recvBuffer.GetUsedSize() < totalSize) break; // Not enough data yet 
		
		std::vector<char> packetBuf(totalSize);
		int deq = session.recvBuffer.Dequeue(packetBuf.data(), static_cast<int>(totalSize));
		if (deq < static_cast<int>(totalSize)) {
			fprintf_s(stderr, "Session %d dequeue failed (got %d of %zu) -> disconnect\n", sessionIndex, deq, totalSize);
			HandleDisconnection(sessionIndex);
			break;
		}

		PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(packetBuf.data());
		char* payloadPtr = packetBuf.data() + sizeof(PacketHeader);

		switch (pHeader->type) {
			case Net::Type::CS_MOVE_START: {
				HandleCSMoveStart(sessionIndex,
					reinterpret_cast<PayloadCSMoveStart*>(payloadPtr));
				} break;
			case Net::Type::CS_MOVE_STOP: {
				HandleCSMoveStop(sessionIndex,
					reinterpret_cast<PayloadCSMoveStop*>(payloadPtr));
				} break;
			case Net::Type::CS_ATTACK1: {
				HandleCSAttack1(sessionIndex,
				reinterpret_cast<PayloadCSAttack1*>(payloadPtr));
				} break;
			case Net::Type::CS_ATTACK2: {
				HandleCSAttack2(sessionIndex,
				reinterpret_cast<PayloadCSAttack2*>(payloadPtr));
			} break;
			case Net::Type::CS_ATTACK3: {
				HandleCSAttack3(sessionIndex,
				reinterpret_cast<PayloadCSAttack3*>(payloadPtr));
			} break;
			case Net::Type::CS_SYNC: {
				// Will not happen 
			} break;
			default: {
				fprintf_s(stderr, "Session %d unknown packet type 0x%02X -> disconnect.", sessionIndex, header.type);
				HandleDisconnection(sessionIndex);
			} break;
		} // switch case 
	}
}

void Net::Manager::Flush(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;

	Session& session = _sessions[sessionIndex];
	if (session.socket == INVALID_SOCKET) return;

	int sentBytes = session.sendBuffer.SendTCP(session.socket); 

	if (sentBytes < 0) {
		_WSAGetLastErrorResult = -sentBytes;
		fprintf(stderr, "Session %d send error: WSAGetLastError = %d\n", sessionIndex, _WSAGetLastErrorResult);
		HandleDisconnection(sessionIndex);
	}
}


void Net::Manager::Shutdown() noexcept {
	if (_hListenSocket != INVALID_SOCKET) {
		::closesocket(_hListenSocket);
		_hListenSocket = INVALID_SOCKET;
	}
	::WSACleanup();
}