#pragma once

// RPCExpress.h 

class Packet; 

class Express {
private:
	Express() noexcept = default;
	~Express() noexcept = default;
	Express(const Express& other) = delete;
	Express& operator=(const Express& other) = delete;
	Express(Express&& other) = default;
	Express& operator=(Express&& other) = default;

private:
	std::queue<std::pair<int, Packet*>> _recvPackets;
	std::queue<Packet*> _sendPackets;

public:
	static Express& GetInstance() noexcept {
		static Express instance;
		return instance;
	}
	inline void EnqueueRecvPacket(int sessionIndex, Packet* packet) noexcept {
		_recvPackets.push({sessionIndex, packet});
	}
	inline void EnqueueSendPacket(Packet* packet) noexcept {
		_sendPackets.push(packet);
	} 

	void ProcessRecvPackets() noexcept; 

	void Unicast(int sessionIndex, Packet* packet) noexcept; 
	void BroadcastExcept(int excludeSessionIndex, Packet* packet) noexcept;
	void Broadcast(Packet* packet) noexcept; 
};