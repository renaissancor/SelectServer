#pragma once

// Protocol.h 

constexpr const uint8_t PACKET_SC_CREATE_MY_CHARACTER = 0;
constexpr const uint8_t PACKET_SC_CREATE_OTHER_CHARACTER = 1; 
constexpr const uint8_t PACKET_SC_DELETE_CHARACTER = 2;

constexpr const uint8_t PACKET_MOVE_DIR_LL = 0; 
constexpr const uint8_t PACKET_MOVE_DIR_LU = 1;
constexpr const uint8_t PACKET_MOVE_DIR_UU = 2;
constexpr const uint8_t PACKET_MOVE_DIR_RU = 3;
constexpr const uint8_t PACKET_MOVE_DIR_RR = 4;
constexpr const uint8_t PACKET_MOVE_DIR_RD = 5;
constexpr const uint8_t PACKET_MOVE_DIR_DD = 6;
constexpr const uint8_t PACKET_MOVE_DIR_LD = 7; 

constexpr const uint8_t PACKET_CS_MOVE_START = 10; // Client -> Server 
constexpr const uint8_t PACKET_SC_MOVE_START = 11; // Server -> Client 
constexpr const uint8_t PACKET_CS_MOVE_STOP = 12;  // Client -> Server 
constexpr const uint8_t PACKET_SC_MOVE_STOP = 13;  // Server -> Client 

constexpr const uint8_t PACKET_CS_ATTACK1 = 20;  // Client -> Server 
constexpr const uint8_t PACKET_SC_ATTACK1 = 21;  // Server -> Client
constexpr const uint8_t PACKET_CS_ATTACK2 = 22;  // Client -> Server 
constexpr const uint8_t PACKET_SC_ATTACK2 = 23;  // Server -> Client 
constexpr const uint8_t PACKET_CS_ATTACK3 = 24;  // Client -> Server
constexpr const uint8_t PACKET_SC_ATTACK3 = 25;  // Server -> Client

constexpr const uint8_t PACKET_SC_DAMAGE = 30;  // Server -> Client 

constexpr const uint8_t PACKET_CS_SYNC = 250;  // Not used 
constexpr const uint8_t PACKET_SC_SYNC = 251;  // used, relocate 

#pragma pack(push, 1)
struct PacketHeader {
	uint8_t code; // 0x89 
	uint8_t size; // Packet size 
	uint8_t type; // Packet type 
}; 
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketSCCreateMyCharacter {
	uint32_t id; 
	uint8_t direction; 
	uint16_t x;
	uint16_t y; 
	uint8_t hp; // 100 
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketSCCreateOtherCharacter {
	PacketHeader header;
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
	uint8_t hp;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketSCDeleteCharacter {
	PacketHeader header;
	uint32_t id;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketCSMoveStart {
	PacketHeader header;
	uint8_t direction; 
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketSCMoveStart {
	PacketHeader header;
	uint32_t id; 
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketSCMoveStop {
	PacketHeader header;
	uint32_t id; 
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketCSAttack1 {
	PacketHeader header;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketSCAttack1 {
	PacketHeader header;
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketCSAttack2 {
	PacketHeader header;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketSCAttack2 {
	PacketHeader header;
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketCSAttack3 {
	PacketHeader header;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketSCAttack3 {
	PacketHeader header;
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketSCDamage {
	PacketHeader header;
	uint32_t attackID;
	uint32_t targetID;
	uint8_t remainingHP;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PacketCSSync {
	PacketHeader header;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketSCSync {
	PacketHeader header;
	uint32_t id;
	uint16_t x;
	uint16_t y;
};
#pragma pack(pop)

