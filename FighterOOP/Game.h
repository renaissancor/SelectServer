#pragma once

namespace Game {
	constexpr static const size_t PLAYER_MAX = 64; // Net::SESSION_MAX;

	constexpr static const short RANGE_MOVE_TOP = 50;
	constexpr static const short RANGE_MOVE_BOTTOM = 470;
	constexpr static const short RANGE_MOVE_LEFT = 10;
	constexpr static const short RANGE_MOVE_RIGHT = 630;
	constexpr static const short RANGE_XY_ERROR = 50;

	constexpr static const short X_MOVE_PER_FRAME = 3;
	constexpr static const short Y_MOVE_PER_FRAME = 2;

	enum Direction : uint8_t {
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

	enum AttackType : uint8_t {
		NO_ATTACK = 0,
		ATTACK1 = 1,
		ATTACK2 = 2,
		ATTACK3 = 3,
	};

	struct Player {
		bool _isAlive = false;
		bool _isMoving = false;
		Game::Direction _direction = Game::STOP;
		Game::AttackType _attackType = Game::NO_ATTACK;
		short _x = 0;
		short _y = 0;
		int _hp = 100;
	};

	inline bool CheckAttackDirection(
		const Game::Player* attacker, const Game::Player* target) noexcept {
		short dx = target->_x - attacker->_x;
		short dy = target->_y - attacker->_y;
		switch (attacker->_direction) {
		case Game::MOVE_DIR_LL:
			return dx < 0;
		case Game::MOVE_DIR_LU:
			return dx < 0 && dy < 0;
		case Game::MOVE_DIR_UU:
			return dy < 0;
		case Game::MOVE_DIR_RU:
			return dx > 0 && dy < 0;
		case Game::MOVE_DIR_RR:
			return dx > 0;
		case Game::MOVE_DIR_RD:
			return dx > 0 && dy > 0;
		case Game::MOVE_DIR_DD:
			return dy > 0;
		case Game::MOVE_DIR_LD:
			return dx < 0 && dy > 0;
		default:
			return false;
		}
	}

	class Manager {
	private:
		Player _players[Game::PLAYER_MAX];

	public:
		inline static Manager& GetInstance() noexcept {
			static Manager instance;
			return instance;
		}

	public:
		void ConnectPlayer(int sessionIndex) noexcept;
		void DisconnectPlayer(int sessionIndex) noexcept; 

		void StartMovePlayer(int sessionIndex, const PayloadCSMoveStart* payload) noexcept;
		void StopMovePlayer(int sessionIndex, const PayloadCSMoveStop* payload) noexcept; 
		void Attack1Player(int sessionIndex, const PayloadCSAttack1* payload) noexcept;
		void Attack2Player(int sessionIndex, const PayloadCSAttack2* payload) noexcept;
		void Attack3Player(int sessionIndex, const PayloadCSAttack3* payload) noexcept;

		void SetPacketSCCreateMyCharacter(int sessionIndex, PacketSCCreateMyCharacter& outPacket) noexcept;
		void SetPacketSCCreateOtherCharacter(int sessionIndex, PacketSCCreateOtherCharacter& outPacket) noexcept;
		void SetPacketSCDeleteCharacter(int sessionIndex, PacketSCDeleteCharacter& outPacket) noexcept;
		void SetPacketSCMoveStart(int sessionIndex, PacketSCMoveStart& outPacket) noexcept; 
		void SetPacketSCMoveStop(int sessionIndex, PacketSCMoveStop& outPacket) noexcept;
		void SetPacketSCAttack1(int sessionIndex, PacketSCAttack1& outPacket) noexcept;
		void SetPacketSCAttack2(int sessionIndex, PacketSCAttack2& outPacket) noexcept;
		void SetPacketSCAttack3(int sessionIndex, PacketSCAttack3& outPacket) noexcept;

		void UpdateMovement() noexcept;
		void UpdateAttackDamage() noexcept; 
		void UpdateDeath() noexcept; 
	};
}
