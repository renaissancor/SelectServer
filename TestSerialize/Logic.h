#pragma once

#include "Packet.h"


enum class DirType : uint8_t
{
	LL = 0,
	LU = 1,
	UU = 2,
	RU = 3,
	RR = 4,
	RD = 5,
	DD = 6,
	LD = 7,
	STOP = 8,
};

enum class AtkType : uint8_t
{
	NO_ATTACK = 0,
	ATTACK1 = 1,
	ATTACK2 = 2,
	ATTACK3 = 3,
};

class Player {
	// Player data and methods 
private:
	bool _isAlive = false;
	DirType _dir = DirType::STOP;
	AtkType _atk = AtkType::NO_ATTACK; 
	short _x = 0;
	short _y = 0;
	int _hp = 100;
public:

};

struct PacketEvent {
	int sessionIndex;
	Packet packet;
};

class Logic { // Layer 7 Game Logic 
private:
	constexpr static size_t MAX_PLAYERS = 64; 
	std::queue<PacketEvent> _packets;

	Player _players[MAX_PLAYERS]; 
	void ProcessPacket(const int sessionIndex, Packet& packet) noexcept;

public:
	static Logic& GetInstance() noexcept {
		static Logic instance;
		return instance;
	}

	inline Player& GetPlayer(size_t index) noexcept { return _players[index]; }
	
	inline void EnqueuePacket(const int sessionIndex, Packet& packet) noexcept {
		_packets.push({sessionIndex, packet});
	}

	void ProcessPackets() noexcept {
		while (!_packets.empty()) {
			PacketEvent ev = _packets.front();
			_packets.pop();
			ProcessPacket(ev.sessionIndex, ev.packet);
		}
	}
}; 