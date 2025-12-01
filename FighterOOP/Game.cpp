#include "stdafx.h"

#include "Net.h" 
#include "Game.h"

void Game::Manager::ConnectPlayer(int sessionIndex) noexcept {
	Game::Player& player = _players[sessionIndex];
	player._isAlive = true;
	player._isMoving = false;
	player._attackType = Game::NO_ATTACK;
	player._direction = Game::MOVE_DIR_DD; 
	player._hp = 100;
	short startX = rand() % 400 + 100;
	short startY = rand() % 300 + 100;
	player._x = startX;
	player._y = startY;
}

void Game::Manager::DisconnectPlayer(int sessionIndex) noexcept {
	Game::Player& player = _players[sessionIndex];
	player._isAlive = false;
}

void Game::Manager::StartMovePlayer(int sessionIndex,
	const PayloadCSMoveStart* payload) noexcept {
	Game::Player& player = _players[sessionIndex];
	player._isMoving = true;
	player._direction = static_cast<Game::Direction>(payload->direction);
	player._x = payload->x;
	player._y = payload->y; 
}

void Game::Manager::StopMovePlayer(int sessionIndex,
	const PayloadCSMoveStop* payload) noexcept {
	Game::Player& player = _players[sessionIndex];
	player._isMoving = false;
	player._direction = static_cast<Game::Direction>(payload->direction); 
	player._x = payload->x;
	player._y = payload->y;
}

void Game::Manager::Attack1Player(int sessionIndex,
	const PayloadCSAttack1* payload) noexcept {
	Game::Player& player = _players[sessionIndex];
	player._attackType = Game::ATTACK1;
	player._direction = static_cast<Game::Direction>(payload->direction);
	player._x = payload->x;
	player._y = payload->y;
}

void Game::Manager::Attack2Player(int sessionIndex,
	const PayloadCSAttack2* payload) noexcept {
	Game::Player& player = _players[sessionIndex];
	player._attackType = Game::ATTACK2;
	player._direction = static_cast<Game::Direction>(payload->direction);
	player._x = payload->x;
	player._y = payload->y;
}

void Game::Manager::Attack3Player(int sessionIndex,
	const PayloadCSAttack3* payload) noexcept {
	Game::Player& player = _players[sessionIndex];
	player._attackType = Game::ATTACK3;
	player._direction = static_cast<Game::Direction>(payload->direction);
	player._x = payload->x;
	player._y = payload->y;
}

void Game::Manager::SetPacketSCCreateMyCharacter(int sessionIndex,
	PacketSCCreateMyCharacter& outPacket) noexcept {

	Game::Player& player = _players[sessionIndex];

	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCCreateMyCharacter);
	outPacket.header.type = PACKET_SC_CREATE_MY_CHARACTER;

	outPacket.payload.id = sessionIndex;
	outPacket.payload.direction = static_cast<uint8_t>(player._direction);
	outPacket.payload.x = player._x;
	outPacket.payload.y = player._y;
	outPacket.payload.hp = static_cast<uint8_t>(player._hp);
}

void Game::Manager::SetPacketSCCreateOtherCharacter(int sessionIndex,
	PacketSCCreateOtherCharacter& outPacket) noexcept {
	Game::Player& player = _players[sessionIndex];
	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCCreateOtherCharacter);
	outPacket.header.type = PACKET_SC_CREATE_OTHER_CHARACTER;
	outPacket.payload.id = sessionIndex;
	outPacket.payload.direction = static_cast<uint8_t>(player._direction);
	outPacket.payload.x = player._x;
	outPacket.payload.y = player._y;
	outPacket.payload.hp = static_cast<uint8_t>(player._hp);
}

void Game::Manager::SetPacketSCDeleteCharacter(int sessionIndex,
	PacketSCDeleteCharacter& outPacket) noexcept {
	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCDeleteCharacter);
	outPacket.header.type = PACKET_SC_DELETE_CHARACTER;
	outPacket.payload.id = sessionIndex;
}

void Game::Manager::SetPacketSCMoveStart(int sessionIndex,
	PacketSCMoveStart& outPacket) noexcept {
	Game::Player& player = _players[sessionIndex];
	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCMoveStart);
	outPacket.header.type = Net::SC_MOVE_START;
	outPacket.payload.id = sessionIndex;
	outPacket.payload.direction = static_cast<uint8_t>(player._direction);
	outPacket.payload.x = player._x;
	outPacket.payload.y = player._y;
}

void Game::Manager::SetPacketSCMoveStop(int sessionIndex,
	PacketSCMoveStop& outPacket) noexcept {
	Game::Player& player = _players[sessionIndex];
	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCMoveStop);
	outPacket.header.type = Net::SC_MOVE_STOP;
	outPacket.payload.id = sessionIndex;
	outPacket.payload.x = player._x;
	outPacket.payload.y = player._y;
}

void Game::Manager::SetPacketSCAttack1(int sessionIndex,
	PacketSCAttack1& outPacket) noexcept {
	Game::Player& player = _players[sessionIndex];
	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCAttack1);
	outPacket.header.type = Net::SC_ATTACK1;
	outPacket.payload.id = sessionIndex;
	outPacket.payload.direction = static_cast<uint8_t>(player._direction);
	outPacket.payload.x = player._x;
	outPacket.payload.y = player._y;
}

void Game::Manager::SetPacketSCAttack2(int sessionIndex,
	PacketSCAttack2& outPacket) noexcept {
	Game::Player& player = _players[sessionIndex];
	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCAttack2);
	outPacket.header.type = Net::SC_ATTACK2;
	outPacket.payload.id = sessionIndex;
	outPacket.payload.direction = static_cast<uint8_t>(player._direction);
	outPacket.payload.x = player._x;
	outPacket.payload.y = player._y;
}

void Game::Manager::SetPacketSCAttack3(int sessionIndex,
	PacketSCAttack3& outPacket) noexcept {
	Game::Player& player = _players[sessionIndex];
	outPacket.header.code = 0x89;
	outPacket.header.size = sizeof(PayloadSCAttack3);
	outPacket.header.type = Net::SC_ATTACK3;
	outPacket.payload.id = sessionIndex;
	outPacket.payload.direction = static_cast<uint8_t>(player._direction);
	outPacket.payload.x = player._x;
	outPacket.payload.y = player._y;
}

void Game::Manager::UpdateMovement() noexcept {
	for (int i = 0; i < PLAYER_MAX; ++i) {
		Game::Player& player = _players[i];
		if (player._isAlive == false) continue;
		if (player._isMoving) {
			int oldX = player._x;
			int oldY = player._y;

			switch (player._direction) {
			case Game::MOVE_DIR_LL:
				player._x -= Game::X_MOVE_PER_FRAME;
				break;
			case Game::MOVE_DIR_LU:
				player._x -= Game::X_MOVE_PER_FRAME;
				player._y -= Game::Y_MOVE_PER_FRAME;
				break;
			case Game::MOVE_DIR_UU:
				player._y -= Game::Y_MOVE_PER_FRAME;
				break;
			case Game::MOVE_DIR_RU:
				player._x += Game::X_MOVE_PER_FRAME;
				player._y -= Game::Y_MOVE_PER_FRAME;
				break;
			case Game::MOVE_DIR_RR:
				player._x += Game::X_MOVE_PER_FRAME;
				break;
			case Game::MOVE_DIR_RD:
				player._x += Game::X_MOVE_PER_FRAME;
				player._y += Game::Y_MOVE_PER_FRAME;
				break;
			case Game::MOVE_DIR_DD:
				player._y += Game::Y_MOVE_PER_FRAME;
				break;
			case Game::MOVE_DIR_LD:
				player._x -= Game::X_MOVE_PER_FRAME;
				player._y += Game::Y_MOVE_PER_FRAME;
				break;
			default:
				break;
			}

			bool outOfBounds =
				player._x < Game::RANGE_MOVE_LEFT ||
				player._x > Game::RANGE_MOVE_RIGHT ||
				player._y < Game::RANGE_MOVE_TOP ||
				player._y > Game::RANGE_MOVE_BOTTOM;

			if (outOfBounds) {
				player._x = oldX;
				player._y = oldY;
			}
			else {
				fprintf_s(stdout, "Player %d moved to (%d, %d)\n", i, player._x, player._y);
			}
		}
	}
}

void Game::Manager::UpdateAttackDamage() noexcept {
	// Placeholder for attack damage logic 
	Net::Manager& network = Net::Manager::GetInstance();

	for (int i = 0; i < PLAYER_MAX; ++i) {
		Game::Player& player = _players[i];
		if (player._isAlive == false) continue;
		if (player._attackType == Game::NO_ATTACK) continue;

		Game::AttackType currentAttackType = player._attackType;

		int attackRange = 60; // Example attack range 
		switch (currentAttackType) {
		case Game::ATTACK1: attackRange = 60; break;
		case Game::ATTACK2: attackRange = 80; break;
		case Game::ATTACK3: attackRange = 100; break;
		}

		fprintf_s(stdout, "Player %d performed attack type %d with range %d\n",
			i, static_cast<int>(currentAttackType), attackRange);

		for (int j = 0; j < PLAYER_MAX; ++j) {
			if (j == i) continue;
			Player& targetPlayer = _players[j];
			if (targetPlayer._isAlive == false) continue; 

			short dx = targetPlayer._x - player._x;
			short dy = targetPlayer._y - player._y;
			int distanceSquared = dx * dx + dy * dy;
			if (distanceSquared > attackRange * attackRange) continue; // Out of range 

			fprintf_s(stdout, "[DAMAGE]	Player %d attacked Player %d\n", i, j);

			// Apply damage
			_players[j]._hp -= 5;

			PacketSCDamage damagePacket = {}; 
			damagePacket.header.code = 0x89;
			damagePacket.header.size = sizeof(PayloadSCDamage);
			damagePacket.header.type = Net::SC_DAMAGE;

			damagePacket.payload.attackID = i;
			damagePacket.payload.targetID = j;
			damagePacket.payload.remainingHP = static_cast<uint8_t>(targetPlayer._hp);

			network.Broadcast(reinterpret_cast<const char*>(&damagePacket), sizeof(damagePacket));

		}
		player._attackType = Game::NO_ATTACK; // Reset attack type after processing 
	} // end of for loop 	
}

void Game::Manager::UpdateDeath() noexcept {
	for (int i = 0; i < PLAYER_MAX; ++i) {
		Game::Player& player = _players[i];
		if (player._isAlive && player._hp <= 0) {
			player._isAlive = false;
			fprintf_s(stdout, "Player %d has died.\n", i);
			Net::Manager::GetInstance().HandleDisconnection(i); 
		}
	}
}
