#include "stdafx.h"
#include "Logic.h"

// Logic.cpp 

void Logic::ProcessPacket(const int sessionIndex, Packet& packet) noexcept {

	Type packetType = packet.GetType();
	switch (packetType) {
	case Type::CS_MOVE_START: {
		// Process move start packet
		// Extract data from packet and update player state
		break;
	}
	case Type::CS_MOVE_STOP: {
		// Process move stop packet
		// Extract data from packet and update player state
		break;
	}
	case Type::CS_ATTACK1: {
		// Process attack1 packet
		// Extract data from packet and update player state
		break;
	}
	case Type::CS_ATTACK2: {
		// Process attack2 packet
		// Extract data from packet and update player state
		break;
	}
	case Type::CS_ATTACK3: {
		// Process attack3 packet
		// Extract data from packet and update player state
		break;
	}
	case Type::CS_SYNC: {
		// Process sync packet
		// Extract data from packet and update player state
		break;
	}
	default:
		// Unknown packet ID, handle error or ignore
		break;
	}
}