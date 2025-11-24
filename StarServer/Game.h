#pragma once

// Game.h 

namespace Game {
	constexpr static const int GAME_WIDTH = 81;
	constexpr static const int GAME_HEIGHT = 24;

	constexpr static const int PLAYERS_MAX = 128; // Same as TCP::SESSION_MAX 
	constexpr static const int EVENT_BUFFER_CAPACITY = 65536 * 2; // 64KB 

	struct Event {
		enum Type : uint32_t {
			LOGIN = 1,
			CREATE = 2,
			MOVE = 3,
			REMOVE = 4,
		};
		uint32_t type;
		uint32_t id;     // playerIndex
		uint32_t x;
		uint32_t y;
	};

	struct Player {
		bool exist = false; // -1 means unassigned 
		int x = 0;
		int y = 0;
		Player() noexcept = default; 
	};

	class Manager {
	private:
		Manager() noexcept; 
		~Manager() noexcept; 
		Manager(Manager const&) = delete;
		Manager const& operator=(Manager const&) = delete;

	private:
		Player* _players[PLAYERS_MAX] = { nullptr };
		RingBuffer* _eventBuffer = nullptr; 

		inline Player* GetPlayer(int index) const noexcept {
			if (index < 0 || index >= PLAYERS_MAX) return nullptr;
			return _players[index];
		}

	public:
		inline static Manager& GetInstance() noexcept {
			static Manager instance;
			return instance;
		}

		inline void EnqueueEvent2Manager(const char* data, int size) noexcept {
			_eventBuffer->Enqueue(data, size);
		}


		bool Initialize() noexcept;
		void Shutdown() noexcept;
		void LoginPlayer(const int index) noexcept; 
		void CreatePlayer(const int index) noexcept;
		void MovePlayer(const int index, int x, int y) noexcept;
		void RemovePlayer(const int index) noexcept;

		void EnqueuePacketAsEvent(const Protocol::Packet& pkt) noexcept;

		void Update() noexcept; 
		void Render() const noexcept; 
	}; 

	inline static void EnqueueEvent(const char* data, int size) noexcept {
		Manager::GetInstance().EnqueueEvent2Manager(data, size);
	}


}
