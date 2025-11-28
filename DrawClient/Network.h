#pragma once

// Network.h 

class Network {
private:
	constexpr static const wchar_t* SERVER_IP = L"127.0.0.1";
	constexpr static const u_short SERVER_PORT = 25000;
	constexpr static const int RECV_BUFFER_SIZE = 32;
	constexpr static const int SEND_BUFFER_SIZE = 32;

	struct NetworkResults {
		int _isConnectedToServer = 0;

		int _WSALastErrorResult = 0;
		int _InetPtonWResult = 0;
		int _WSAStartupResult = 0;
		int _SocketCreateResult = 0;
		int _setsockoptResult = 0;

		int _connectResult = 0;
		int _sendResult = 0;
		int _recvResult = 0;
	};

private:
	Network() noexcept;
	~Network() noexcept; 
	Network(const Network&) = delete;
	Network& operator=(const Network&) = delete;

private:
	// Use WSAAsyncSelect for this project 
	WSADATA _wsa = { 0 };
	SOCKET _socket = INVALID_SOCKET; 
	SOCKADDR_IN _serverAddr = { 0 };

	wchar_t _ipAddress[INET_ADDRSTRLEN] = { 0 };
	RingBuffer _bufferRecv = RingBuffer(RECV_BUFFER_SIZE);
	RingBuffer _bufferSend = RingBuffer(RECV_BUFFER_SIZE); 
	NetworkResults _results;

	bool _isWritable = false; // After FD_WRITE event is received 

public:
	inline static Network& GetInstance() noexcept {
		static Network instance;
		return instance;
	}

	bool Initialize() noexcept; 
	bool TryConnect() noexcept;
	void Shutdown() noexcept;

	void RecvData() noexcept;
	void SendData() noexcept; 

	void ProcessWSA(WPARAM wParam, LPARAM lParam) noexcept;
	void HandlePacket() noexcept; 
	void SendDrawPacket(const int ax, const int ay, const int bx, const int by) noexcept;

};