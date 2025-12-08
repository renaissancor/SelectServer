#pragma once

// Network.h 

class Network {
public:
	constexpr static const u_short SERVER_PORT = 5000;
	constexpr static const size_t RECV_BUFFER_SIZE = 4096;
	constexpr static const size_t SEND_BUFFER_SIZE = 4096;
	constexpr static const size_t SESSION_MAX = 64; // FD_SETSIZE;

private:
	Session _sessions[SESSION_MAX];
	std::vector<int> _closeSessionEvent;

	WSADATA _wsa = { 0 };
	SOCKET _hListenSocket = INVALID_SOCKET;
	SOCKET _hMaxSocket = INVALID_SOCKET;
	SOCKADDR_IN _serverAddr = { 0 };

	size_t _sessionIDCount = 0;
	fd_set _readSet = { 0 };
	fd_set _writeSet = { 0 };

	int _WSAGetLastErrorResult = 0;
	int _lastSessionIndex = -1;

private:
	Network() noexcept = default;
	~Network() noexcept = default;
	Network(const Network& other) = delete;
	Network& operator=(const Network& other) = delete;
	Network(Network&& other) = delete;
	Network& operator=(Network&& other) = default; // allow this one only  
public:
	static Network& GetInstance() noexcept {
		static Network instance;
		return instance;
	}

	inline Session& GetSession(int index) noexcept { return _sessions[index % SESSION_MAX]; }
	inline void UpdateMaxSocketAfterConnection(SOCKET newSocket) noexcept {
		if (newSocket > _hMaxSocket) _hMaxSocket = newSocket;
	}
	void UpdateMaxSocketAfterDisconnection(SOCKET disconnectedSocket) noexcept;
	int GetNewSessionIndex() noexcept;

public:
	void BroadcastExcept(int excludeSessionIndex, const char* data, int size) noexcept;
	void Unicast(int sessionIndex, const char* data, int size) noexcept;
	bool Initialize() noexcept;
	int AcceptConnections() noexcept; 
	void DisconnectSession(int sessionIndex) noexcept; 
	void Flush(int sessionIndex) noexcept;
	void RecvFromTCP(int sessionIndex) noexcept;
	void ProcessReceivedData(int sessionIndex) noexcept;
	void HandleDisconnection(int sessionIndex) noexcept;

	void BuildFDSets() noexcept;
	void Poll() noexcept;
	void FlushAll() noexcept; 

	void RunEngine() noexcept; 
	void Shutdown() noexcept; 
}; 