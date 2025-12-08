#include "stdafx.h"
#include "Stub.h"
#include "Logic.h" 

void ProcessPacket(int sessionIndex, Packet* packet) noexcept 
{
	if (packet == nullptr) return;
	uint8_t dir;
	short x, y;
	switch (packet->GetType()) {
	case Type::CS_MOVE_START:
		packet->SkipHeader();
		*packet >> dir >> x >> y;
		Logic::GetInstance().OnMoveStart(sessionIndex, static_cast<Direction>(dir), x, y);
		break;
	case Type::CS_MOVE_STOP:
		packet->SkipHeader();
		*packet >> dir >> x >> y;
		Logic::GetInstance().OnMoveStop(sessionIndex, static_cast<Direction>(dir), x, y);
		break;
	case Type::CS_ATTACK1:
		packet->SkipHeader();
		*packet >> dir >> x >> y;
		Logic::GetInstance().OnAttack1(sessionIndex, static_cast<Direction>(dir), x, y);
		break;
	case Type::CS_ATTACK2:
		packet->SkipHeader();
		*packet >> dir >> x >> y;
		Logic::GetInstance().OnAttack2(sessionIndex, static_cast<Direction>(dir), x, y);
		break;
	case Type::CS_ATTACK3:
		packet->SkipHeader();
		*packet >> dir >> x >> y;
		Logic::GetInstance().OnAttack3(sessionIndex, static_cast<Direction>(dir), x, y);
		break;
	default:
		break;
	}
		
}