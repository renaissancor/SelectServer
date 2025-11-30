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

void Network::Manager::RenewHighestSocket() noexcept {
	_hMaxSocket = _hListenSocket;
	for (const auto& session : _sessions) {
		if (session.socket == INVALID_SOCKET) continue;
		if (session.socket > _hMaxSocket) _hMaxSocket = session.socket;
	}
	fprintf_s(stdout, "Renewed Highest Socket: %llu\n", static_cast<unsigned long long>(_hMaxSocket));
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


void Network::Manager::Poll() noexcept {
	BuildFDSets();

	timeval tv = { 0, 0 };
	fd_set readSetCopy = _readSet;
	fd_set writeSetCopy = _writeSet;

	int result = ::select(
		static_cast<int>(_hMaxSocket) + 1,
		&readSetCopy,
		&writeSetCopy,
		NULL,
		&tv
	);

	if (result == SOCKET_ERROR) return;

	if (result > 0) {
		if (FD_ISSET(_hListenSocket, &readSetCopy)) {
			AcceptNewConnection();
		}

		std::vector<int> sessionsToProcess;
		for (int i = 0; i < SESSION_MAX; ++i) {
			if (_sessions[i].socket == INVALID_SOCKET) continue;

			SOCKET sock = _sessions[i].socket;

			if (sock == INVALID_SOCKET) continue;

			if (_sessions[i].socket == INVALID_SOCKET) continue;

			if (FD_ISSET(sock, &readSetCopy)) {
				sessionsToProcess.push_back(i);
			}
		}

		for (int idx : sessionsToProcess) {
			if (_sessions[idx].socket == INVALID_SOCKET) {
				fprintf(stdout, "[Poll] Skipping session %d (INVALID before Receive)\n", idx);
				continue;
			}
			Receive(idx);
		}

		ProcessRecvData();
		Update();

		std::vector<int> sessionsToFlush;
		for (int i = 0; i < SESSION_MAX; ++i) {
			if (_sessions[i].socket == INVALID_SOCKET) continue;
			SOCKET sock = _sessions[i].socket;
			if (sock == INVALID_SOCKET) continue;
			if (_sessions[i].socket == INVALID_SOCKET) continue;

			if (FD_ISSET(sock, &writeSetCopy)) {
				sessionsToFlush.push_back(i);
			}
		}

		for (int idx : sessionsToFlush) {
			if (_sessions[idx].socket == INVALID_SOCKET) continue;
			Flush(idx);
		}
	}
}

int Network::Manager::AcceptNewConnection() noexcept {
	int acceptedCount = 0;
	while (true) {

		SOCKADDR_IN clientAddr = { 0 };
		int clientAddrLen = sizeof(clientAddr);
		SOCKET hNewSocket = ::accept(_hListenSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
		
		if (hNewSocket == INVALID_SOCKET) {
			int wsaError = WSAGetLastError();
			if (wsaError != WSAEWOULDBLOCK) _WSAGetLastErrorResult = wsaError;
			RenewHighestSocket(); // before return! 
			return acceptedCount;
		}

		int newSessionIndex = GetAvailableSessionIndex(); 
		
		if (newSessionIndex == -1) { // Session Full 
			::closesocket(hNewSocket); 
			continue;
		}
		
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

		PacketSCCreateMyCharacter packetSCCreateMyCharacter = {};
		packetSCCreateMyCharacter.header.code = 0x89;
		packetSCCreateMyCharacter.header.size = sizeof(PayloadSCCreateMyCharacter);
		packetSCCreateMyCharacter.header.type = PACKET_SC_CREATE_MY_CHARACTER;

		packetSCCreateMyCharacter.payload.id = newSessionIndex; 
		packetSCCreateMyCharacter.payload.direction = PACKET_MOVE_DIR_LL;
		packetSCCreateMyCharacter.payload.x = 100;
		packetSCCreateMyCharacter.payload.y = 100;
		packetSCCreateMyCharacter.payload.hp = 100; 

		session.sendBuffer.Enqueue(
			reinterpret_cast<const char*>(&packetSCCreateMyCharacter),
			sizeof(PacketSCCreateMyCharacter)
		);

		PacketSCCreateOtherCharacter packetSCCreateOtherCharacter = {}; 

		packetSCCreateOtherCharacter.header.code = 0x89;
		packetSCCreateOtherCharacter.header.size = (uint8_t)sizeof(PayloadSCCreateOtherCharacter);
		packetSCCreateOtherCharacter.header.type = PACKET_SC_CREATE_OTHER_CHARACTER;

		packetSCCreateOtherCharacter.payload.id = newSessionIndex; 
		packetSCCreateOtherCharacter.payload.direction = PACKET_MOVE_DIR_LL;
		packetSCCreateOtherCharacter.payload.x = 100;
		packetSCCreateOtherCharacter.payload.y = 100;
		packetSCCreateOtherCharacter.payload.hp = 100;

		for (int i = 0; i < SESSION_MAX; ++i) {
			if (i == newSessionIndex) continue; 
			Session& otherSession = _sessions[i]; 
			if (otherSession.socket == INVALID_SOCKET) continue; 
			
			otherSession.sendBuffer.Enqueue(
				reinterpret_cast<const char*>(&packetSCCreateOtherCharacter),
				sizeof(PacketSCCreateOtherCharacter)
			);
		}

	}
}

void Network::Manager::HandleDisconnection(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;
	fprintf_s(stdout, "Disconnected Session %d...\n", sessionIndex); 
	auto& session = _sessions[sessionIndex];
	if (session.socket != INVALID_SOCKET) {
		SOCKET socketToClose = session.socket;
		FD_CLR(socketToClose, &_readSet);
		FD_CLR(socketToClose, &_writeSet);
		::closesocket(session.socket);
		session.socket = INVALID_SOCKET;
		session.recvBuffer.ClearBuffer();
		session.sendBuffer.ClearBuffer();
		RenewHighestSocket();
	}
}

void Network::Manager::Receive(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return;

	Session& session = _sessions[sessionIndex]; 
	if (session.socket == INVALID_SOCKET) {
		fprintf(stderr, "Receive called on invalid session %d\n", sessionIndex);
		return;
	}

	if (session.recvBuffer.GetFreeSize() == 0) {
		fprintf(stdout, "Session %d recv buffer full, skipping recv.\n", sessionIndex);
		return;
	}

	int result = session.recvBuffer.RecvFromTCP(session.socket);
	printf("[Receive] Session %d RecvFromTCP result = %d, recv buffer size = %d\n", 
		sessionIndex, result, session.recvBuffer.GetUsedSize());

	if (result > 0) return; 
	else if (result == 0) {
		fprintf(stdout, "Session %d recv returned 0 (connection closed)\n", sessionIndex);
		HandleDisconnection(sessionIndex);
		return; 
	} 
	
	int errorCode = -result;
	if (errorCode == WSAEWOULDBLOCK) {
		return; // No data available right now in non-blocking mode 
	} 
	else {
		_WSAGetLastErrorResult = result;
		fprintf(stderr, "Session %d recv error: WSAGetLastError = %d\n", sessionIndex, errorCode);
		HandleDisconnection(sessionIndex);
		return;
	}
}


void Network::Manager::ProcessRecvData() noexcept {
	// fprintf_s(stdout, "Function [ProcessRecvData] called\n");
	for (int i = 0; i < SESSION_MAX; ++i) {
		Session& session = _sessions[i]; 
		if (session.socket == INVALID_SOCKET) continue; 
		
		while (session.recvBuffer.IsEmpty() == false) {
			fprintf_s(stdout, "Recv Buffer Used Size for Session %d: %d bytes\n",
				i, session.recvBuffer.GetUsedSize());
			// Peek header 
			PacketHeader header = { 0, }; 
			int peekBytes = session.recvBuffer.Peek(
				reinterpret_cast<char*>(&header),
				sizeof(PacketHeader)
			);
			if (peekBytes < sizeof(PacketHeader)) break;

			// Print Packet 
			fprintf_s(stdout, "Session %d Packet Header: code=0x%02X, size=%u, type=0x%02X\n",
				i, header.code, header.size, header.type);

			// Basic header validation
			if (header.code != 0x89) {
				fprintf_s(stderr, "Session %d invalid header.code=0x%02X -> disconnect\n", i, header.code);
				HandleDisconnection(i);
				break;
			}
			uint8_t payloadSize = header.size;
			if (payloadSize == 0 || payloadSize > 64) {
				fprintf_s(stderr, "Session %d invalid header.size=%u -> disconnect\n", i, payloadSize);
				HandleDisconnection(i);
				break;
			}

			size_t totalSize = sizeof(PacketHeader) + static_cast<size_t>(payloadSize);
			if (session.recvBuffer.GetUsedSize() < totalSize) break; // Not enough data yet 

			// Wait until full packet arrives
			
			std::vector<char> packetBuf(totalSize);
            int deq = session.recvBuffer.Dequeue(packetBuf.data(), static_cast<int>(totalSize));
            if (deq < static_cast<int>(totalSize)) {
                fprintf_s(stderr, "Session %d dequeue failed (got %d of %zu) -> disconnect\n", i, deq, totalSize);
                HandleDisconnection(i);
                break;
            }

			PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(packetBuf.data());
			char* payloadPtr = packetBuf.data() + sizeof(PacketHeader);

			switch (pHeader->type) {
			case Packet::Type::CS_MOVE_START: {
				fprintf_s(stdout, "PACKET CS_MOVE_START from Session %d received.\n", i);
				if (payloadSize != sizeof(PayloadCSMoveStart)) {
					fprintf_s(stderr, "Session %d CS_MOVE_START size mismatch (%u != %zu) -> disconnect\n",
						i, payloadSize, sizeof(PayloadCSMoveStart));
					HandleDisconnection(i);
					break;
				}
				auto* p = reinterpret_cast<PayloadCSMoveStart*>(payloadPtr);
				Game::Player& player = _players[i];
				player._isMoving = true;
				player._direction = static_cast<Game::Direction>(p->direction);
				player._x = p->x;
				player._y = p->y;

				// Broadcast SC_MOVE_START
				PacketSCMoveStart out = {};
				out.header.code = 0x89;
				out.header.size = static_cast<uint8_t>(sizeof(PayloadSCMoveStart));
				out.header.type = Packet::Type::SC_MOVE_START;
				out.payload.id = i;
				out.payload.direction = player._direction;
				out.payload.x = player._x;
				out.payload.y = player._y;

				for (int j = 0; j < SESSION_MAX; ++j) {
					if (j == i) continue;
					Session& other = _sessions[j];
					if (other.socket == INVALID_SOCKET) continue;
					other.sendBuffer.Enqueue(reinterpret_cast<const char*>(&out), sizeof(out));
				}
			} break;

			case Packet::Type::CS_MOVE_STOP: {
				fprintf_s(stdout, "PACKET CS_MOVE_STOP from Session %d received.\n", i); 
				if (payloadSize != sizeof(PayloadCSMoveStop)) {
					fprintf_s(stderr, "Session %d CS_MOVE_STOP size mismatch (%u != %zu) -> disconnect\n",
						i, payloadSize, sizeof(PayloadCSMoveStop));
					HandleDisconnection(i);
					break;
				}
				auto* p = reinterpret_cast<PayloadCSMoveStop*>(payloadPtr);
				Game::Player& player = _players[i];
				player._isMoving = false;
				player._direction = static_cast<Game::Direction>(p->direction);
				player._x = p->x;
				player._y = p->y;

				PacketSCMoveStop out = {};
				out.header.code = 0x89;
				out.header.size = static_cast<uint8_t>(sizeof(PayloadSCMoveStop));
				out.header.type = Packet::Type::SC_MOVE_STOP;
				out.payload.id = i;
				out.payload.direction = player._direction;
				out.payload.x = player._x;
				out.payload.y = player._y;

				for (int j = 0; j < SESSION_MAX; ++j) {
					if (j == i) continue;
					Session& other = _sessions[j];
					if (other.socket == INVALID_SOCKET) continue;
					other.sendBuffer.Enqueue(reinterpret_cast<const char*>(&out), sizeof(out));
				}
			} break;

			case Packet::Type::CS_ATTACK1: {
			} break;
			case Packet::Type::CS_ATTACK2: {
			} break;
			case Packet::Type::CS_ATTACK3: {
			} break;
			case Packet::Type::CS_SYNC: {
			} break;
			default: {
				fprintf_s(stderr, "Session %d unknown packet type 0x%02X -> disconnect.", i, header.type);
				HandleDisconnection(i);
			} break;
			}


		}
	}
}

void Network::Manager::Update() noexcept {
	// Game logic update can be implemented here 
	for (int i = 0; i < SESSION_MAX; ++i) {
		Game::Player& player = _players[i];
		if (player._isAlive == false) continue; 
		if(player._isMoving) {
			// Update player position based on direction 
			switch (player._direction) {
			case Game::MOVE_DIR_LL:
				player._x -= Game::X_MOVE_PER_FRAME;
				if (player._x < Game::RANGE_MOVE_LEFT) 
					player._x = Game::RANGE_MOVE_LEFT;
				break;
			case Game::MOVE_DIR_LU:
				player._x -= Game::X_MOVE_PER_FRAME;
				player._y -= Game::Y_MOVE_PER_FRAME;
				if (player._x < Game::RANGE_MOVE_LEFT)
					player._x = Game::RANGE_MOVE_LEFT;
				if (player._y < Game::RANGE_MOVE_TOP)
					player._y = Game::RANGE_MOVE_TOP;
				break;
			case Game::MOVE_DIR_UU:
				player._y -= Game::Y_MOVE_PER_FRAME;
				if (player._y < Game::RANGE_MOVE_TOP)
					player._y = Game::RANGE_MOVE_TOP;
				break;
			case Game::MOVE_DIR_RU:
				player._x += Game::X_MOVE_PER_FRAME;
				player._y -= Game::Y_MOVE_PER_FRAME;
				if (player._x > Game::RANGE_MOVE_RIGHT)
					player._x = Game::RANGE_MOVE_RIGHT;
				if (player._y < Game::RANGE_MOVE_TOP)
					player._y = Game::RANGE_MOVE_TOP;
				break;
			case Game::MOVE_DIR_RR:
				player._x += Game::X_MOVE_PER_FRAME;
				if (player._x > Game::RANGE_MOVE_RIGHT)
					player._x = Game::RANGE_MOVE_RIGHT;
				break;
			case Game::MOVE_DIR_RD:
				player._x += 1;
				player._y += 1;
				if (player._x > Game::RANGE_MOVE_RIGHT)
					player._x = Game::RANGE_MOVE_RIGHT;
				if (player._y > Game::RANGE_MOVE_BOTTOM)
					player._y = Game::RANGE_MOVE_BOTTOM;
				break;
			case Game::MOVE_DIR_DD:
				player._y += 1;
				if (player._y > Game::RANGE_MOVE_BOTTOM)
					player._y = Game::RANGE_MOVE_BOTTOM;
				break;
			case Game::MOVE_DIR_LD:
				player._x -= 1;
				player._y += 1;
				if (player._x < Game::RANGE_MOVE_LEFT)
					player._x = Game::RANGE_MOVE_LEFT;
				if (player._y > Game::RANGE_MOVE_BOTTOM)
					player._y = Game::RANGE_MOVE_BOTTOM;
				break;
			default:
				break;
			}
			fprintf_s(stdout, "Player %d moved to (%d, %d)\n", i, player._x, player._y); 
		}
	}
}


void Network::Manager::Flush(int sessionIndex) noexcept {
	if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return; 

	Session& session = _sessions[sessionIndex];
	if (session.socket == INVALID_SOCKET) return;

	int sentBytes = session.sendBuffer.SendToTCP(session.socket);
	fprintf_s(stdout, "[Flush] Before: head=%d, tail=%d, capacity=%d, used=%d\n",
		session.sendBuffer.GetHeadIndex(),
		session.sendBuffer.GetTailIndex(),
		session.sendBuffer.GetCapacity(),
		session.sendBuffer.GetUsedSize());

	if (sentBytes < 0) {
		_WSAGetLastErrorResult = -sentBytes; 
		fprintf(stderr, "Session %d send error: WSAGetLastError = %d\n", sessionIndex, _WSAGetLastErrorResult);
		HandleDisconnection(sessionIndex);
		return;
	}
}


void Network::Manager::Shutdown() noexcept {
	if (_hListenSocket != INVALID_SOCKET) {
		::closesocket(_hListenSocket);
		_hListenSocket = INVALID_SOCKET;
	}
	::WSACleanup();
}

