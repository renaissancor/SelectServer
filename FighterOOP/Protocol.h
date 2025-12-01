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
struct PayloadSCCreateMyCharacter {
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
	uint8_t hp; // 100 
};

struct PacketSCCreateMyCharacter {
	PacketHeader header;
	PayloadSCCreateMyCharacter payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadSCCreateOtherCharacter {
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
	uint8_t hp;
};

struct PacketSCCreateOtherCharacter {
	PacketHeader header;
	PayloadSCCreateOtherCharacter payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadSCDeleteCharacter {
	uint32_t id;
};

struct PacketSCDeleteCharacter {
	PacketHeader header;
	PayloadSCDeleteCharacter payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadCSMoveStart {
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketCSMoveStart {
	PacketHeader header;
	PayloadCSMoveStart payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadSCMoveStart {
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketSCMoveStart {
	PacketHeader header;
	PayloadSCMoveStart payload;
};
#pragma pack(pop) 

#pragma	pack(push, 1)
struct PayloadCSMoveStop {
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketCSMoveStop {
	PacketHeader header;
	PayloadCSMoveStop payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadSCMoveStop {
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketSCMoveStop {
	PacketHeader header;
	PayloadSCMoveStop payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadCSAttack1 {
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketCSAttack1 {
	PacketHeader header;
	PayloadCSAttack1 payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadSCAttack1 {
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketSCAttack1 {
	PacketHeader header;
	PayloadSCAttack1 payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadCSAttack2 {
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketCSAttack2 {
	PacketHeader header;
	PayloadCSAttack2 payload;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PayloadSCAttack2 {
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketSCAttack2 {
	PacketHeader header;
	PayloadSCAttack2 payload;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PayloadCSAttack3 {
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketCSAttack3 {
	PacketHeader header;
	PayloadCSAttack3 payload;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PayloadSCAttack3 {
	uint32_t id;
	uint8_t direction;
	uint16_t x;
	uint16_t y;
};

struct PacketSCAttack3 {
	PacketHeader header;
	PayloadSCAttack3 payload;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PayloadSCDamage {
	uint32_t attackID;
	uint32_t targetID;
	uint8_t remainingHP;
};

struct PacketSCDamage {
	PacketHeader header;
	PayloadSCDamage payload;
};
#pragma pack(pop) 

#pragma pack(push, 1)
struct PayloadCSSync {
	uint16_t x;
	uint16_t y;
};

struct PacketCSSync {
	PacketHeader header;
	PayloadCSSync payload;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PayloadSCSync {
	uint32_t id;
	uint16_t x;
	uint16_t y;
};

struct PacketSCSync {
	PacketHeader header;
	PayloadSCSync payload;
};
#pragma pack(pop)

