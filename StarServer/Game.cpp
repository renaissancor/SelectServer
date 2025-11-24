#include "stdafx.h"
#include "Game.h" 
#include "TCP.h"
#include "Console.h" 

Game::Manager::Manager() noexcept
	: _eventBuffer(nullptr) {
	memset(_players, 0, sizeof(_players));
}

Game::Manager::~Manager() noexcept {
	if (_eventBuffer) {
		delete _eventBuffer;
		_eventBuffer = nullptr; 
	}
}

bool Game::Manager::Initialize() noexcept {
	int i = 0;
	try {
		_eventBuffer = new RingBuffer(EVENT_BUFFER_CAPACITY); 
		for (; i < PLAYERS_MAX; ++i) {
			_players[i] = new Player();
		}
	}
	catch (...) {
		if (_eventBuffer) {
			delete _eventBuffer;
			_eventBuffer = nullptr; 
		}
		for (int j = 0; j < i; ++j) {
			delete _players[j];
			_players[j] = nullptr;
		}
		return false;
	}
	return true; 
}

void Game::Manager::Shutdown() noexcept {
	for (int i = 0; i < PLAYERS_MAX; ++i) {
		if (_players[i]) {
			delete _players[i];
			_players[i] = nullptr;
		}
	}
}

void Game::Manager::EnqueuePacketAsEvent(const Protocol::Packet& pkt) noexcept {
	// Convert from network packet to internal Event.
	Event e;
	e.type = static_cast<int>(pkt.type);   
	e.id = static_cast<int>(pkt.id);
	e.x = static_cast<int>(pkt.x);
	e.y = static_cast<int>(pkt.y);

	_eventBuffer->Enqueue(reinterpret_cast<const char*>(&e), sizeof(e));
}


void Game::Manager::LoginPlayer(const int index) noexcept {
	char LoginMessage[16] = { 0 };
	*reinterpret_cast<int*>(&LoginMessage[0]) = static_cast<int>(Event::Type::LOGIN);
	*reinterpret_cast<int*>(&LoginMessage[4]) = index;

	TCP::Manager::GetInstance().SendToSessionManager(index, LoginMessage, sizeof(LoginMessage));
}

void Game::Manager::CreatePlayer(const int index) noexcept {

	Player* newPlayer = GetPlayer(index);
	if (newPlayer == nullptr) return; // Invalid index 
	newPlayer->exist = true;
	newPlayer->x = rand() % GAME_WIDTH; // Random initial position
	newPlayer->y = rand() % GAME_HEIGHT;
	
	char NewPlayerCreatedMsg[16] = { 0 };
	*reinterpret_cast<int*>(&NewPlayerCreatedMsg[0]) = static_cast<int>(Event::Type::CREATE);
	*reinterpret_cast<int*>(&NewPlayerCreatedMsg[4]) = index;
	*reinterpret_cast<int*>(&NewPlayerCreatedMsg[8]) = newPlayer->x;
	*reinterpret_cast<int*>(&NewPlayerCreatedMsg[12]) = newPlayer->y; 

	for (int i = 0; i < PLAYERS_MAX; ++i) {
		Player* player = _players[i];
		if (player->exist == false) continue;
		if (i != index) {
			TCP::Manager::GetInstance().SendToSessionManager(i, NewPlayerCreatedMsg, sizeof(NewPlayerCreatedMsg));
		}
	}

	for (int i = 0; i < PLAYERS_MAX; ++i) {
		Player* existingPlayer = _players[i];
		if (existingPlayer->exist == false) continue;

		char ExistingPlayerMsg[16] = { 0 };
		*reinterpret_cast<int*>(&ExistingPlayerMsg[0]) = static_cast<int>(Event::Type::CREATE);
		*reinterpret_cast<int*>(&ExistingPlayerMsg[4]) = i;
		*reinterpret_cast<int*>(&ExistingPlayerMsg[8]) = existingPlayer->x;
		*reinterpret_cast<int*>(&ExistingPlayerMsg[12]) = existingPlayer->y;

		TCP::Manager::GetInstance().SendToSessionManager(index, ExistingPlayerMsg, sizeof(ExistingPlayerMsg));
	}
}

void Game::Manager::MovePlayer(const int index, int x, int y) noexcept {
	Player* movedPlayer = GetPlayer(index);
	if (movedPlayer == nullptr) return; // Invalid index 
	movedPlayer->x = x;
	movedPlayer->y = y;
	// Boundary check 
	if (movedPlayer->x < 0) movedPlayer->x = 0;
	if (movedPlayer->x >= GAME_WIDTH) movedPlayer->x = GAME_WIDTH - 1;
	if (movedPlayer->y < 0) movedPlayer->y = 0;
	if (movedPlayer->y >= GAME_HEIGHT) movedPlayer->y = GAME_HEIGHT - 1; 
	if (movedPlayer->x == x && movedPlayer->y == y) return; // No movement 

	for (int i = 0; i < PLAYERS_MAX; ++i) {
		Player* other = _players[i];
		if (other->exist == false) continue;
		// Initialize player data

		char MoveMessage[16] = { 0 };
		*reinterpret_cast<int*>(&MoveMessage[0]) = static_cast<int>(Event::Type::MOVE);
		*reinterpret_cast<int*>(&MoveMessage[4]) = index;
		*reinterpret_cast<int*>(&MoveMessage[8]) = movedPlayer->x;
		*reinterpret_cast<int*>(&MoveMessage[12]) = movedPlayer->y;

		TCP::Manager::GetInstance().SendToSessionManager(i, MoveMessage, sizeof(MoveMessage));
	}
}

void Game::Manager::RemovePlayer(const int index) noexcept {
	Player* player = GetPlayer(index);
	if (player == nullptr) return; // Invalid index 
	player->exist = false;
	
	for (int i = 0; i < PLAYERS_MAX; ++i) {
		Player* other = _players[i];
		if (other->exist == false) continue;
		// Initialize player data
		char RemoveMessage[16] = { 0 };
		*reinterpret_cast<int*>(&RemoveMessage[0]) = static_cast<int>(Event::Type::REMOVE);
		*reinterpret_cast<int*>(&RemoveMessage[4]) = index;
		TCP::Manager::GetInstance().SendToSessionManager(i, RemoveMessage, sizeof(RemoveMessage));
	}
}

void Game::Manager::Update() noexcept {
	// Game logic update can be implemented here 
	for (;;) {
		Event event = {};
		int eventSize = sizeof(Event);
		int peekedSize = _eventBuffer->Peek((char*)&event, eventSize);
		if (peekedSize < eventSize) break; // Not enough data 
		_eventBuffer->Dequeue((char*)&event, eventSize); // Remove event from buffer 
		Player* player = GetPlayer(event.id);
		if (player == nullptr) continue; // Invalid player index 
		switch (event.type) {
		case Event::Type::LOGIN:
			// Handle Create ID event 
			LoginPlayer(event.id); 
			break;
		case Event::Type::CREATE:
			// Handle Create User event 
			CreatePlayer(event.id);
			break;
		case Event::Type::MOVE:
			// Update player position 
			MovePlayer(event.id, event.x, event.y); 
			break;
		case Event::Type::REMOVE:
			RemovePlayer(event.id); 
			break;
		default:
			// Unknown event type 
			break;
		}
	}

	for (int i = 0; i < PLAYERS_MAX; ++i) {
		Player* player = _players[i];
		if (player->exist == false) continue; // Unassigned player slot 
	}
	
}

void Game::Manager::Render() const noexcept {
	for (int i = 0; i < PLAYERS_MAX; ++i) {
		const Player* player = _players[i];
		if (player->exist == false) continue; 
		Console::Manager::GetInstance().Draw(player->y, player->x, 'A' + (i % 26), 0x0A);
	}
	
}
