#pragma once

#include "GameInfo.h"
#include "packet.h" 

namespace Nets {
	struct Session; 
}

namespace Game {
	struct SectorEvent { // Sector Event 
		i32 senderID = -1;
		Packet packet; 
	};
	struct SectorChange { // size 8 bytes 
		i32 playerID = -1;
		Sector sector_old; 
		Sector sector_new; 
	};

	extern std::unordered_map<i32, Player*> players; 
	extern std::vector<i32>			 sectors[SECTOR_COUNT_X][SECTOR_COUNT_Y]; 
	extern std::vector<SectorEvent>  sector_events[SECTOR_COUNT_X][SECTOR_COUNT_Y];
	extern std::vector<SectorChange> sector_changes; 

	void Update() noexcept; 
	void UpdatePosition(Player* player) noexcept; 
	
	inline void ClearSectorEvents() noexcept {
		for (i16 sx = 0; sx < SECTOR_COUNT_X; ++sx) 
			for (i16 sy = 0; sy < SECTOR_COUNT_Y; ++sy) 
				sector_events[sx][sy].clear(); 
	}

	inline void ClearSectorChanges() noexcept {
		sector_changes.clear(); 
	}

	void HandleSectorChanges() noexcept; 
	void DeliverSectorEvents(i16 sx, i16 sy) noexcept; 
}


