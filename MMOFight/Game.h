#pragma once

#include "Types.h"

namespace Nets {
	struct Session; 
}

namespace Game {
	constexpr i16 RANGE_MOVE_TOP    = 0; 
	constexpr i16 RANGE_MOVE_LEFT   = 0;
	constexpr i16 RANGE_MOVE_RIGHT  = 6400; 
	constexpr i16 RANGE_MOVE_BOTTOM = 6400; 

	constexpr i16 SECTOR_SIZE_X  = 200;
	constexpr i16 SECTOR_SIZE_Y  = 200;
	constexpr i16 SECTOR_COUNT_X = (RANGE_MOVE_RIGHT  / SECTOR_SIZE_X);  // 32 
	constexpr i16 SECTOR_COUNT_Y = (RANGE_MOVE_BOTTOM / SECTOR_SIZE_Y); // 32 

	constexpr i16 ATTACK1_RANGE_X = 80;
	constexpr i16 ATTACK2_RANGE_X = 90; 
	constexpr i16 ATTACK3_RANGE_X = 100; 

	constexpr i16 ATTACK1_RANGE_Y = 10;
	constexpr i16 ATTACK2_RANGE_Y = 15;
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
		i16 x = -1; 
		i16 y = -1; 
		inline bool operator==(const Sector& other) const noexcept { return x == other.x && y == other.y; }
		inline bool operator!=(const Sector& other) const noexcept { return x != other.x || y != other.y; } 
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

	inline Sector GetSector(i16 x, i16 y) noexcept {
		Sector sector; 
		sector.x = x / SECTOR_SIZE_X; 
		sector.y = y / SECTOR_SIZE_Y; 
		if (sector.x < 0) sector.x = 0; 
		if (sector.x >= SECTOR_COUNT_X) sector.x = SECTOR_COUNT_X - 1; 
		if (sector.y < 0) sector.y = 0; 
		if (sector.y >= SECTOR_COUNT_Y) sector.y = SECTOR_COUNT_Y - 1; 
		return sector; 
	}


	extern std::unordered_map<i32, Player*> players; 
	extern std::vector<i32> sectors[SECTOR_COUNT_X][SECTOR_COUNT_Y]; 

	void Update() noexcept; 
	void GetNeighborPlayers(const Player* player, std::vector<Player*>& out_players) noexcept; 
}


