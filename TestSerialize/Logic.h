#pragma once

#include "Protocol.h"
#include "Packet.h"
#include <queue> 

enum class AtkType : uint8_t
{
	NO_ATTACK = 0,
	ATTACK1 = 1,
	ATTACK2 = 2,
	ATTACK3 = 3,
};

constexpr static const short RANGE_MOVE_TOP = 50;
constexpr static const short RANGE_MOVE_BOTTOM = 470;
constexpr static const short RANGE_MOVE_LEFT = 10;
constexpr static const short RANGE_MOVE_RIGHT = 630;

class Player {
	// Player data and methods 
private:
	bool _isAlive = false;
	bool _isMoving = false; 
	Direction _dir = Direction::STOP;
	AtkType _atk = AtkType::NO_ATTACK; 
	short _x = 0;
	short _y = 0;
	int _hp = 100;
public:
	inline void Create() noexcept {
		_isAlive = true;
		_isMoving = false;
		_dir = Direction::STOP;
		_atk = AtkType::NO_ATTACK;
		_x = rand() % 420 + 110; // 110 ~ 530 
		_y = rand() % 220 + 150; // 150 ~ 370 
		_hp = 100;
	}

	inline void Destroy() noexcept {
		_isAlive = false;
	}

	inline void MoveStart(Direction dir, short x, short y) noexcept {
		_isMoving = true; 
		_dir = dir;
		_x = x;
		_y = y;
	}

	inline void MoveStop(Direction dir, short x, short y) noexcept {
		_isMoving = false; 
		_dir = dir;
		_x = x;
		_y = y;
	}

	inline void Attack1(Direction dir, short x, short y) noexcept {
		_atk = AtkType::ATTACK1;
		_dir = dir;
		_x = x;
		_y = y;
	}

	inline void Attack2(Direction dir, short x, short y) noexcept {
		_atk = AtkType::ATTACK2;
		_dir = dir;
		_x = x;
		_y = y;
	}

	inline void Attack3(Direction dir, short x, short y) noexcept {
		_atk = AtkType::ATTACK3;
		_dir = dir;
		_x = x;
		_y = y;
	}
};

struct PacketEvent {
	int sessionIndex;
	Packet *packet;
};

class Logic { // Layer 7 Game Logic 
private:
	constexpr static size_t MAX_PLAYERS = 64; 
	std::queue<PacketEvent> _packetEvents;

	Player _players[MAX_PLAYERS]; 

public:
	void ProcessPacket(const int sessionIndex, Packet *packet) noexcept;
	void OnPlayerConnected(int sessionIndex) noexcept; 
	void OnPlayerDisconnected(int sessionIndex) noexcept; 

	static Logic& GetInstance() noexcept {
		static Logic instance;
		return instance;
	}

	inline Player& GetPlayer(size_t index) noexcept { return _players[index]; }
	
	inline void EnqueuePacket(const int sessionIndex, Packet *packet) noexcept {
		_packetEvents.push({sessionIndex, packet});
	}


	void ProcessPackets() noexcept {
		while (!_packetEvents.empty()) {
			PacketEvent ev = _packetEvents.front();
			_packetEvents.pop();
			ProcessPacket(ev.sessionIndex, ev.packet);
		}
	}
}; 