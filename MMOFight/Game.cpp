#include "stdafx.h"
#include "Game.h" 
#include "Nets.h"

namespace Game {
	std::unordered_map<i32, Player*> players; 

	std::vector<i32> sectors[SECTOR_COUNT_X][SECTOR_COUNT_Y]; 
	std::vector<SectorEvent>  sector_events[SECTOR_COUNT_X][SECTOR_COUNT_Y];
	std::vector<SectorChange> sector_changes;
}

void Game::UpdatePosition(Player* player) noexcept {
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
}

void Game::HandleSectorChanges() noexcept {
	// For each sector change, remove from old sector and add to new sector 
	for (const SectorChange& sec_change : sector_changes) {
		i32 playerID = sec_change.playerID;
		Sector sector_old = sec_change.sector_old;
		Sector sector_new = sec_change.sector_new;
		// Remove from old sector 
		std::vector<i32>& old_sector_players = sectors[sector_old.x][sector_old.y]; 
		for(size_t i = 0; i < old_sector_players.size(); ++i) {
			if (old_sector_players[i] == playerID) {
				old_sector_players[i] = old_sector_players.back();
				old_sector_players.pop_back();
				break;
			}
		}
		// Add to new sector 
		std::vector<i32>& new_sector_players = sectors[sector_new.x][sector_new.y]; 
		new_sector_players.push_back(playerID); 

		for (i16 y = sec_change.sector_old.y - 1; y <= sec_change.sector_old.y + 1; ++y) {
			for (i16 x = sec_change.sector_old.x - 1; x <= sec_change.sector_old.x + 1; ++x) {
				if (x < 0 || x >= SECTOR_COUNT_X || y < 0 || y >= SECTOR_COUNT_Y) continue;
				if (x < sec_change.sector_new.x - 1 || x > sec_change.sector_new.x + 1 ||
					y < sec_change.sector_new.y - 1 || y > sec_change.sector_new.y + 1) {

				}
			}
		}

		for (i16 y = sec_change.sector_new.y - 1; y <= sec_change.sector_new.y + 1; ++y) {
			for (i16 x = sec_change.sector_new.x - 1; x <= sec_change.sector_new.x + 1; ++x) {
				if (x < 0 || x >= SECTOR_COUNT_X || y < 0 || y >= SECTOR_COUNT_Y) continue;
				if (x < sec_change.sector_old.x - 1 || x > sec_change.sector_old.x + 1 ||
					y < sec_change.sector_old.y - 1 || y > sec_change.sector_old.y + 1) {

				}
			}
		}

	}
	// Clear sector changes after handling 
	ClearSectorChanges();
}

void Game::DeliverSectorEvents(i16 sx, i16 sy) noexcept {
	std::vector<SectorEvent>& this_sector_events = sector_events[sx][sy]; 
	if (this_sector_events.empty()) return; 

	for (i16 dy = -1; dy <= 1; ++dy) {
		for (i16 dx = -1; dx <= 1; ++dx) {
			i16 nsx = sx + dx;
			i16 nsy = sy + dy;
			if (nsx < 0 || nsx >= SECTOR_COUNT_X || nsy < 0 || nsy >= SECTOR_COUNT_Y) continue; 

			std::vector<i32>& neighbor_sector_players_ids = sectors[nsx][nsy]; 
			if (neighbor_sector_players_ids.empty()) continue; 

			for (i32 playerID : neighbor_sector_players_ids) {
				auto player_it = players.find(playerID);
				if (player_it == players.end()) continue;
				Player* player = player_it->second;
				if (player->hp <= 0) continue;
				Nets::Session* session = player->session;
				for (const SectorEvent& sec_event : this_sector_events) {
					if (sec_event.senderID == playerID) continue; // Do not send to self 
					session->sendQ.enqueue((const char*)sec_event.packet.GetHeadPtr(), sec_event.packet.GetUsedSize()); 
				}
			} // for playerID 
		} // for dx 
	} // for dy
}

void Game::Update() noexcept {
	using Iter = std::unordered_map<i32, Player*>::iterator;
	ClearSectorEvents(); 
	Player* player = nullptr; 
	for (Iter it = players.begin(); it != players.end(); ) {
		player = it->second; 
		++it; 
		if (player->hp <= 0) continue; 
		UpdatePosition(player); 
		// Send Move Packet 

		SectorEvent move_event;
		move_event.senderID = player->sessionID;
		move_event.packet << (u08)0x89 << (u08)9 << (u08)PACKET_SC_MOVE_START
			<< (u32)player->sessionID << (u08)player->direction
			<< (u16)player->x << (u16)player->y;

		sector_events[player->sector_curr.x][player->sector_curr.y].emplace_back(std::move(move_event));
		
		// Sector Update Check 
		Sector new_sector = GetSector(player->x, player->y); 
		Sector old_sector = player->sector_curr;
		if (new_sector == old_sector) continue; 
		player->sector_prev = old_sector; 
		player->sector_curr = new_sector; 

		SectorChange sec_change; 
		sec_change.playerID = player->sessionID;
		sec_change.sector_old = old_sector;
		sec_change.sector_new = new_sector;
		sector_changes.push_back(sec_change);
	}

	for (i16 sx = 0; sx < SECTOR_COUNT_X; ++sx) 
		for (i16 sy = 0; sy < SECTOR_COUNT_Y; ++sy) 
			DeliverSectorEvents(sx, sy);

}