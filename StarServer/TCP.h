#pragma once

namespace TCP {
	constexpr static const u_short SERVER_PORT = 3000;

	constexpr static const int SESSION_MAX = 128; // Select Model Limit 

	struct Session {
		SOCKET hSocket = INVALID_SOCKET;
		sockaddr address = {};
		RingBuffer recvBuffer; 
		RingBuffer sendBuffer;
	};


	class Manager {
	private:
		Manager() noexcept;
		~Manager() noexcept;
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;

	private:
		WSADATA _wsaData;
		SOCKET _hListenSocket = INVALID_SOCKET;
		SOCKET _hMaxSocket = INVALID_SOCKET;
		SOCKADDR_IN _serverAddr = { 0 };

		int _WSAGetLastErrorResult = 0;

		int _lastSessionIndex = -1;

		Session* _sessions[SESSION_MAX] = { nullptr }; 
		
		fd_set _readSet;
		fd_set _writeSet;
	public: 
		inline static Manager& GetInstance() noexcept {
			static Manager instance;
			return instance;
		}
		inline int GetNewSessionIndex() noexcept {
			_lastSessionIndex = (_lastSessionIndex + 1) % SESSION_MAX;
			return _lastSessionIndex; 
		}

		inline int SendToSessionManager(int sessionIndex, const char* data, int size) const noexcept {
			if (sessionIndex < 0 || sessionIndex >= SESSION_MAX) return 0;
			Session* session = _sessions[sessionIndex];
			if (session == nullptr) return 0;
			return session->sendBuffer.Enqueue(data, size);
		}

		bool Initialize() noexcept;
		
		int  AcceptNewConnection() noexcept; 
		void HandleDisconnection(int sessionIndex) noexcept; 
		int UpdateSet() noexcept;
		void ProcessSessionRecv(int sessionIndex) noexcept;
		void ProcessSessionSend(int sessionIndex) noexcept;
		void ProcessPacket() noexcept;

		void Update() noexcept; 

		void Shutdown() noexcept;
	};

	inline int SendToSession(int sessionIndex, const char* data, int size) noexcept {
		return Manager::GetInstance().SendToSessionManager(sessionIndex, data, size);
	}

} // namespace TCP