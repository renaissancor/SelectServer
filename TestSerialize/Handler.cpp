#include "stdafx.h"
#include "Handler.h" 
#include "Logic.h" 

// Remote Procedure Call from L7 to L5 

void Handle_CS_MOVE_START(int index, Packet* packet) noexcept {
	uint8_t code, size, type;
	*packet >> code >> size >> type;

	Direction dir;
	short x;
	short y;
	*packet >> dir >> x >> y;

	Logic& logic = Logic::GetInstance();
	Player& player = logic.GetPlayer(static_cast<size_t>(index));
	player.MoveStart(dir, x, y);

}

void Handle_CS_MOVE_STOP(int index, Packet* packet) noexcept {
	uint8_t code, size, type;
	*packet >> code >> size >> type;

	Direction dir;
	short x;
	short y;
	*packet >> dir >> x >> y;

	Logic& logic = Logic::GetInstance();
	Player& player = logic.GetPlayer(static_cast<size_t>(index));
	player.MoveStop(dir, x, y); 
}

void Handle_CS_ATTACK1(int index, Packet* packet) noexcept {
	uint8_t code, size, type;
	*packet >> code >> size >> type;

	Direction dir;
	short x;
	short y;

	*packet >> dir >> x >> y; 

	Logic& logic = Logic::GetInstance();
	Player& player = logic.GetPlayer(static_cast<size_t>(index));

}

void Handle_CS_ATTACK2(int index, Packet* packet) noexcept {
	uint8_t code, size, type;
	*packet >> code >> size >> type;

	Direction dir;
	short x;
	short y;
	*packet >> dir >> x >> y; 

	Logic& logic = Logic::GetInstance();
	Player& player = logic.GetPlayer(static_cast<size_t>(index));
}

void Handle_CS_ATTACK3(int index, Packet* packet) noexcept {
	uint8_t code, size, type;
	*packet >> code >> size >> type;
	Direction dir;
	short x;
	short y;
	*packet >> dir >> x >> y; 
	Logic& logic = Logic::GetInstance();
	Player& player = logic.GetPlayer(static_cast<size_t>(index));
}
