#pragma once

#include "Types.h"

namespace Nets {
	struct Session;
}

namespace Game {
	constexpr i16 RANGE_MOVE_TOP = 0; 
	constexpr i16 RANGE_MOVE_LEFT = 0;
	constexpr i16 RANGE_MOVE_RIGHT = 6400; 
	constexpr i16 RANGE_MOVE_BOTTOM = 6400; 

	constexpr i16 ATTACK1_RANGE_X = 80;
	constexpr i16 ATTACK2_RANGE_X = 90; 
	constexpr i16 ATTACK3_RANGE_X = 100; 

	constexpr i16 ATTACK1_RANGE_Y = 10;
	constexpr i16 ATTACK2_RANGE_Y = 10;
	constexpr i16 ATTACK3_RANGE_Y = 20; 

	constexpr i32 ATTACK1_DAMAGE = 1; 
	constexpr i32 ATTACK2_DAMAGE = 2;
	constexpr i32 ATTACK3_DAMAGE = 3;

	constexpr i16 X_MOVE_PER_FRAME = 3; 
	constexpr i16 Y_MOVE_PER_FRAME = 2; 

	constexpr i16 RANGE_XY_ERROR = 50; 

	enum class EventType : u08 {
		CREATE = 1, 
		REMOVE,
		ATTACK1,
		ATTACK2,
		ATTACK3,
	};

	struct Event {
		int id; // Session Index 
		EventType type; 
		short x;
		short y; 
	};

	struct Sector {
		
	};

	enum Action : u08 {
		NONE = 0,
		MOVE_LL = 1,
		MOVE_LU = 2,
		MOVE_UU = 3,
		MOVE_RU = 4,
		MOVE_RR = 5,
		MOVE_RD = 6,
		MOVE_DD = 7,
		MOVE_LD = 8,
	}; 

	enum Direction : u08 {
		MOVE_DIR_LL = 0,
		MOVE_DIR_LU = 1,
		MOVE_DIR_UU = 2,
		MOVE_DIR_RU = 3,
		MOVE_DIR_RR = 4,
		MOVE_DIR_RD = 5,
		MOVE_DIR_DD = 6,
		MOVE_DIR_LD = 7,
		STOP = 8,
	};

	struct Player {
		Nets::Session* session; 
		i32 sessionID = -1;
		Action action;
		Direction direction;
		Direction direction_move;

		i16 hp = 100; 
		i16 x = 0;
		i16 y = 0; 

		Sector sector_curr; 
		Sector sector_prev; 
	};

	extern std::map<i32, Player*> players; 

	void Update() noexcept; 
}


