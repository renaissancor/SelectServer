#pragma once

// Session.h 

class Session {
private:
	bool _closed = true; 
	int _index = -1; 
	SOCKET _socket = INVALID_SOCKET; 
	sockaddr _sockaddr = { 0 }; 
	RingBuffer _recvBuffer; 
	RingBuffer _sendBuffer; 
	int _wsaError = 0; 

public:
	inline SOCKET GetSocket() const noexcept { return _socket; }
	inline void SetSocket(SOCKET socket) noexcept {  _socket = socket; }
	inline const sockaddr& GetSockAddr() const noexcept { return _sockaddr; }
	inline void SetSockAddr(const sockaddr& addr) noexcept { _sockaddr = addr; }
	inline RingBuffer& GetRecvBuffer() noexcept { return _recvBuffer; }
	inline RingBuffer& GetSendBuffer() noexcept { return _sendBuffer; }
	inline void SetIndex(int index) noexcept { _index = index; } 
	inline int GetIndex() const noexcept { return _index; } 

	void Receive() noexcept; // recv to Game L7 

	inline void Send(const Packet& packet) noexcept { // Enqueue to send buffer Layer 5 
		_sendBuffer.Enqueue(packet.GetBuffer(), packet.GetUsedSize());
	}

public:
	void Close() noexcept; 
	void SendTCP() noexcept; // ::send   to TCP Layer 4 
	void RecvTCP() noexcept; // ::recv from TCP Layer 4
};

