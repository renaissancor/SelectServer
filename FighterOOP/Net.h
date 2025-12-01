#pragma once

namespace Net {
	constexpr static const u_short SERVER_PORT = 5000;
	constexpr static const int RECV_BUFFER_SIZE = 4096;
	constexpr static const int SEND_BUFFER_SIZE = 4096;
	constexpr static const size_t SESSION_MAX = 64; // FD_SETSIZE;

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

	struct Session {
		volatile SOCKET socket = INVALID_SOCKET;
		sockaddr sockaddr = { 0 };
		RecvRingBuffer recvBuffer;
		SendRingBuffer sendBuffer;
	};

	void HandleCSMoveStart(int sessionIndex, const PayloadCSMoveStart* payload) noexcept;
	void HandleCSMoveStop(int sessionIndex, const PayloadCSMoveStop* payload) noexcept;
	void HandleCSAttack1(int sessionIndex, const PayloadCSAttack1* payload) noexcept;
	void HandleCSAttack2(int sessionIndex, const PayloadCSAttack2* payload) noexcept;
	void HandleCSAttack3(int sessionIndex, const PayloadCSAttack3* payload) noexcept; 

	class Manager {
	private:
		WSADATA _wsa = { 0 };
		SOCKET _hListenSocket = INVALID_SOCKET;
		SOCKET _hMaxSocket = INVALID_SOCKET;
		SOCKADDR_IN _serverAddr = { 0 };

		Session _sessions[SESSION_MAX];
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

	private:
		inline void UpdateMaxSocketAfterConnection(SOCKET newSocket) noexcept {
			if (newSocket > _hMaxSocket) _hMaxSocket = newSocket;
		}
		void UpdateMaxSocketAfterDisconnection(SOCKET disconnectedSocket) noexcept; 

	public:
		bool Initialize() noexcept; 
		int AcceptNewConnections() noexcept; 
		void HandleDisconnection(int sessionIndex) noexcept; 

		void Unicast(int sessionIndex, const char* data, int size) noexcept;
		void Broadcast(const char* data, int size) noexcept; 
		void BroadcastExcept(int excludeSessionIndex, const char* data, int size) noexcept; 

		void BuildFDSets() noexcept;
		void Poll() noexcept; 


		void RecvFromTCP(int sessionIndex) noexcept; 
		void ProcessReceivedData(int sessionIndex) noexcept; 
		void Flush(int sessionIndex) noexcept; 
		void FlushAll() noexcept;	

		void Shutdown() noexcept; 
	};

}