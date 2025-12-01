#pragma once

// Network.h 

namespace Packet {

	enum Type : uint8_t {
		SC_CREATE_MY_CHARACTER = 0,
		SC_CREATE_OTHER_CHARACTER = 1,
		SC_DELETE_CHARACTER = 2,
		CS_MOVE_START = 10,
		SC_MOVE_START = 11,
		CS_MOVE_STOP = 12,
		SC_MOVE_STOP = 13,
		CS_ATTACK1 = 20,
		SC_ATTACK1 = 21,
		CS_ATTACK2 = 22,
		SC_ATTACK2 = 23,
		CS_ATTACK3 = 24,
		SC_ATTACK3 = 25,
		SC_DAMAGE = 30,
		CS_SYNC = 250,
		SC_SYNC = 251,
	}; 

	enum Direction : uint8_t {
		MOVE_DIR_LL = 0,
		MOVE_DIR_LU = 1,
		MOVE_DIR_UU = 2,
		MOVE_DIR_RU = 3,
		MOVE_DIR_RR = 4,
		MOVE_DIR_RD = 5,
		MOVE_DIR_DD = 6,
		MOVE_DIR_LD = 7,
	};

	

}

namespace Game {
	constexpr static const short RANGE_MOVE_TOP = 50; 
	constexpr static const short RANGE_MOVE_BOTTOM = 470;
	constexpr static const short RANGE_MOVE_LEFT = 10;
	constexpr static const short RANGE_MOVE_RIGHT = 630;
	constexpr static const short RANGE_XY_ERROR = 50; 

	constexpr static const short X_MOVE_PER_FRAME = 3;
	constexpr static const short Y_MOVE_PER_FRAME = 2; 

	enum Direction : uint8_t {
		MOVE_DIR_LL = 0,
		MOVE_DIR_LU = 1,
		MOVE_DIR_UU = 2,
		MOVE_DIR_RU = 3,
		MOVE_DIR_RR = 4,
		MOVE_DIR_RD = 5,
		MOVE_DIR_DD = 6,
		MOVE_DIR_LD = 7,
		STOP = 8, 
	};

	enum AttackType : uint8_t {
		NO_ATTACK = 0,
		ATTACK1 = 1,
		ATTACK2 = 2,
		ATTACK3 = 3,
	};

	struct Player {
		bool _isAlive = false; 
		bool _isMoving = false; 
		Game::Direction _direction = Game::STOP; 
		short _x = 0;
		short _y = 0;
		int _hp = 100;
		Game::AttackType _attackType = Game::NO_ATTACK; 
	};
}

namespace Network {
	constexpr static const u_short SERVER_PORT = 5000;
	constexpr static const int RECV_BUFFER_SIZE = 4096;
	constexpr static const int SEND_BUFFER_SIZE = 4096;
	constexpr static const size_t SESSION_MAX = 64; // FD_SETSIZE;

	struct Session {
		volatile SOCKET socket = INVALID_SOCKET; 
		sockaddr sockaddr = { 0 }; 
		RingBuffer recvBuffer; 
		RingBuffer sendBuffer; 
	};

	class Manager {
	private:
		Manager();
		~Manager();
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;

	private:
		WSADATA _wsa = { 0 };
		SOCKET _hListenSocket = INVALID_SOCKET;
		SOCKET _hMaxSocket = INVALID_SOCKET;
		SOCKADDR_IN _serverAddr = { 0 };

		Session _sessions[SESSION_MAX];
		Game::Player _players[SESSION_MAX]; 

		size_t _sessionIDCount = 0; 

		fd_set _readSet = { 0 };
		fd_set _writeSet = { 0 }; 

		int _WSAGetLastErrorResult = 0;
		int _lastSessionIndex = -1; 

	public:
		inline static Manager& GetInstance() noexcept {
			static Manager instance;
			return instance;
		}
		inline int GetAvailableSessionIndex() const noexcept {
			for (int i = 0; i < SESSION_MAX; ++i) {
				if (_sessions[i].socket == INVALID_SOCKET) return i;
			}
			return -1;
		}

		void RenewHighestSocket() noexcept;
		bool Initialize() noexcept; 

		void BuildFDSets() noexcept;
		void Receive(int sessionIndex) noexcept;
		void Flush(int sessionIndex) noexcept; 
		void Poll() noexcept; 
		void ProcessRecvData() noexcept; 
		void UpdateAttack() noexcept; 
		void UpdateMove() noexcept; 

		int  AcceptNewConnection() noexcept;
		void HandleDisconnection(int sessionIndex) noexcept;

		void Shutdown() noexcept;
	}; // class Manager 

}
