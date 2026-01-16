#include "stdafx.h"
#include "Game.h" 
#include "Nets.h"

namespace Game {
	std::unordered_map<i32, Player*> players; 

	std::vector<i32> sectors[SECTOR_COUNT_X][SECTOR_COUNT_Y]; 
}

void Game::Update() noexcept {
	Player* player = nullptr; 
	using Iter = std::unordered_map<i32, Player*>::iterator;
	for (Iter it = players.begin(); it != players.end(); ) {
		player = it->second; 
		++it; 
		if (player->hp <= 0) {
			// Handle Disconnected Player 
			continue; 
		}

		switch (player->action) {
			case Action::MOVE_LL:
				player->x -= X_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_LL;
				break;
			case Action::MOVE_LU:
				player->x -= X_MOVE_PER_FRAME;
				player->y -= Y_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_LU;
				break;
			case Action::MOVE_UU:
				player->y -= Y_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_UU;
				break;
			case Action::MOVE_RU:
				player->x += X_MOVE_PER_FRAME;
				player->y -= Y_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_RU;
				break;
			case Action::MOVE_RR:
				player->x += X_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_RR;
				break;
			case Action::MOVE_RD:
				player->x += X_MOVE_PER_FRAME;
				player->y += Y_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_RD;
				break;
			case Action::MOVE_DD:
				player->y += Y_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_DD;
				break;
			case Action::MOVE_LD:
				player->x -= X_MOVE_PER_FRAME;
				player->y += Y_MOVE_PER_FRAME;
				player->direction = Direction::MOVE_DIR_LD;
				break;
		}
		// Boundary Check 
		if (player->x < RANGE_MOVE_LEFT) player->x = RANGE_MOVE_LEFT;
		if (player->x > RANGE_MOVE_RIGHT) player->x = RANGE_MOVE_RIGHT;
		if (player->y < RANGE_MOVE_TOP) player->y = RANGE_MOVE_TOP;
		if (player->y > RANGE_MOVE_BOTTOM) player->y = RANGE_MOVE_BOTTOM; 

		// Sector update based on new position 
		Sector new_sector = GetSector(player->x, player->y); 
		if (new_sector != player->sector_curr) {
			// Remove from previous sector 
			if (player->sector_curr.x >= 0 && player->sector_curr.y >= 0) {
				std::vector<i32>& prev_sector_players = sectors[player->sector_curr.x][player->sector_curr.y];
				// TODO: O(N) removal; can be optimized with different data structure if needed 
				prev_sector_players.erase(std::remove(prev_sector_players.begin(), prev_sector_players.end(), player->sessionID), prev_sector_players.end());
			}
			// Add to new sector 
			sectors[new_sector.x][new_sector.y].push_back(player->sessionID);
			player->sector_prev = player->sector_curr;
			player->sector_curr = new_sector;
		}
	}
}