#include "stdafx.h"
#include "Logic.h"
#include "Proxy.h"  

// Logic.cpp 

void Logic::OnPlayerConnected(int sessionIndex) noexcept {
	// Initialize player state for the new connection
	Player& player = _players[static_cast<size_t>(sessionIndex)];
	// Set initial values for player
	player.Create(); 

	// Send SC_CREATE_MY_CHARACTER packet to the new player 
	// Send SC_CREATE_MY_CHARACTER packet to the new player 
	// Send SC_CREATE_OTHER_CHARACTER packets to existing players 

	SendCreateMyCharacter(sessionIndex, sessionIndex,
		static_cast<uint8_t>(player.GetDir()),
		player.GetX(), player.GetY(), player.GetHP());

	for (int i = 0; i < MAX_PLAYERS; ++i) {
		if (i == sessionIndex) continue;
		if (!_players[i].IsAlive()) continue;

		SendCreateOtherCharacter(i, sessionIndex,
			static_cast<uint8_t>(player.GetDir()),
			player.GetX(), player.GetY(), player.GetHP());

		SendCreateOtherCharacter(sessionIndex, i,
			static_cast<uint8_t>(_players[i].GetDir()),
			_players[i].GetX(), _players[i].GetY(), _players[i].GetHP());
	}

}

void Logic::OnPlayerDisconnected(int sessionIndex) noexcept {
	// Clean up player state for the disconnected player
	Player& player = _players[static_cast<size_t>(sessionIndex)];
	player.Destroy(); 
	// Send SC_DELETE_CHARACTER packet to all remaining players 
	SendDeleteCharacter(sessionIndex, sessionIndex); // Broadcast automatic 
}

void Logic::OnMoveStart(int sessionIndex, uint8_t dir, uint16_t x, uint16_t y) noexcept {
	Player& player = _players[static_cast<size_t>(sessionIndex)];
	player.MoveStart(static_cast<Direction>(dir), static_cast<short>(x), static_cast<short>(y));
	SendMoveStart(sessionIndex, sessionIndex, dir, x, y); // Broadcast automatic
}

void Logic::OnMoveStop(int sessionIndex, uint8_t dir, uint16_t x, uint16_t y) noexcept {
	Player& player = _players[static_cast<size_t>(sessionIndex)];
	player.MoveStop(static_cast<Direction>(dir), static_cast<short>(x), static_cast<short>(y));
	SendMoveStop(sessionIndex, sessionIndex, dir, x, y); // Broadcast automatic
}

void Logic::OnAttack1(int sessionIndex, uint8_t dir, uint16_t x, uint16_t y) noexcept {
	Player& player = _players[static_cast<size_t>(sessionIndex)];
	player.Attack1(static_cast<Direction>(dir), static_cast<short>(x), static_cast<short>(y));
	SendAttack1(sessionIndex, sessionIndex, dir, x, y); // Broadcast automatic
}

void Logic::OnAttack2(int sessionIndex, uint8_t dir, uint16_t x, uint16_t y) noexcept {
	Player& player = _players[static_cast<size_t>(sessionIndex)];
	player.Attack2(static_cast<Direction>(dir), static_cast<short>(x), static_cast<short>(y));
	SendAttack2(sessionIndex, sessionIndex, dir, x, y); // Broadcast automatic
}

void Logic::OnAttack3(int sessionIndex, uint8_t dir, uint16_t x, uint16_t y) noexcept {
	Player& player = _players[static_cast<size_t>(sessionIndex)];
	player.Attack3(static_cast<Direction>(dir), static_cast<short>(x), static_cast<short>(y)); 
	SendAttack3(sessionIndex, sessionIndex, dir, x, y); // Broadcast automatic
	
}


void Logic::Update() noexcept {

    // Update Movement 
    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        Player& player = _players[i];
        if (!player.IsAlive()) continue;
        if (!player.IsMoving()) continue;

        short oldX = player.GetX();
        short oldY = player.GetY();
        short newX = oldX;
        short newY = oldY;

        Direction dir = player.GetDir();

        switch (dir) {
        case Direction::MOVE_DIR_LL:
            newX -= 3;
            break;
        case Direction::MOVE_DIR_LU:
            newX -= 3;
            newY -= 2;
            break;
        case Direction::MOVE_DIR_UU:
            newY -= 2;
            break;
        case Direction::MOVE_DIR_RU:
            newX += 3;
            newY -= 2;
            break;
        case Direction::MOVE_DIR_RR:
            newX += 3;
            break;
        case Direction::MOVE_DIR_RD:
            newX += 3;
            newY += 2;
            break;
        case Direction::MOVE_DIR_DD:
            newY += 2;
            break;
        case Direction::MOVE_DIR_LD:
            newX -= 3;
            newY += 2;
            break;
        default:
            break;
        }
        if (newX < RANGE_MOVE_LEFT || newX > RANGE_MOVE_RIGHT ||
            newY < RANGE_MOVE_TOP || newY > RANGE_MOVE_BOTTOM) {
            continue;
        }

        player.SetPosition(newX, newY);
    }

    // Update Damage 

	UpdateAttackDamage(); 

    // Update Death 
    for (size_t i = 0; i < MAX_PLAYERS; ++i) {
        Player& player = _players[i];
        if (!player.IsAlive()) continue;
        if (player.GetHP() <= 0) {
            player.Destroy();
            SendDeleteCharacter(-1, static_cast<uint32_t>(i));
        }
    }
}


void Logic::UpdateAttackDamage() noexcept {


	for (int i = 0; i < MAX_PLAYERS; ++i) {
		Player& player = _players[i];

		if (player.IsAlive() == false) continue;
		AtkType atkType = player.GetAtk();
		if (atkType == AtkType::NO_ATTACK) continue;
		player.ClearAtk(); // Reset attack type after processing

		int attackRange = 60; // Example attack range 
		switch (atkType) {
		case AtkType::ATTACK1: attackRange = 60; break;
		case AtkType::ATTACK2: attackRange = 80; break;
		case AtkType::ATTACK3: attackRange = 100; break;
		}

		fprintf_s(stdout, "Player %d performed attack type %d with range %d\n",
			i, static_cast<int>(atkType), attackRange);

		for (int j = 0; j < MAX_PLAYERS; ++j) {
			if (j == i) continue;
			Player& targetPlayer = _players[j];
			if (targetPlayer.IsAlive() == false) continue;

			short dx = targetPlayer.GetX() - player.GetX();
			short dy = targetPlayer.GetY() - player.GetY();
			int distanceSquared = dx * dx + dy * dy;
			if (distanceSquared > attackRange * attackRange) continue; // Out of range 
			bool isInAttackDirection = CheckAttackDirection(player, targetPlayer);
			if (!isInAttackDirection) continue;

			fprintf_s(stdout, "[DAMAGE]	Player %d attacked Player %d\n", i, j);

			// Apply damage
			targetPlayer.Damage(5);
			
			SendDamage(i, j, static_cast<uint8_t>(targetPlayer.GetHP())); 
		}
	}
}
