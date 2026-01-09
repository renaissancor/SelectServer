#include "stdafx.h"
#include "Game.h" 
#include "Nets.h"

namespace Game {
	std::map<i32, Player*> players; 
}

void Game::Update() noexcept {
	Player* player = nullptr; 
	using Iter = std::map<i32, Player*>::iterator; 
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

		if(player->action >= Action::MOVE_LL && player->action <= Action::MOVE_LD) {
			// Sector Update 

		}


	}

}