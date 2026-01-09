#pragma once

using u08 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using i08 = signed char;
using i16 = signed short;
using i32 = signed int;
using i64 = signed long long;

using f32 = float;
using f64 = double;

constexpr const u08 PACKET_SC_CREATE_MY_CHARACTER = 0;
constexpr const u08 PACKET_SC_CREATE_OTHER_CHARACTER = 1;
constexpr const u08 PACKET_SC_DELETE_CHARACTER = 2;

constexpr const u08 PACKET_MOVE_DIR_LL = 0;
constexpr const u08 PACKET_MOVE_DIR_LU = 1;
constexpr const u08 PACKET_MOVE_DIR_UU = 2;
constexpr const u08 PACKET_MOVE_DIR_RU = 3;
constexpr const u08 PACKET_MOVE_DIR_RR = 4;
constexpr const u08 PACKET_MOVE_DIR_RD = 5;
constexpr const u08 PACKET_MOVE_DIR_DD = 6;
constexpr const u08 PACKET_MOVE_DIR_LD = 7;

constexpr const u08 PACKET_CS_MOVE_START = 10; // Client -> Server 
constexpr const u08 PACKET_SC_MOVE_START = 11; // Server -> Client 
constexpr const u08 PACKET_CS_MOVE_STOP = 12;  // Client -> Server 
constexpr const u08 PACKET_SC_MOVE_STOP = 13;  // Server -> Client 

constexpr const u08 PACKET_CS_ATTACK1 = 20;  // Client -> Server 
constexpr const u08 PACKET_SC_ATTACK1 = 21;  // Server -> Client
constexpr const u08 PACKET_CS_ATTACK2 = 22;  // Client -> Server 
constexpr const u08 PACKET_SC_ATTACK2 = 23;  // Server -> Client 
constexpr const u08 PACKET_CS_ATTACK3 = 24;  // Client -> Server
constexpr const u08 PACKET_SC_ATTACK3 = 25;  // Server -> Client

constexpr const u08 PACKET_SC_DAMAGE = 30;  // Server -> Client 

constexpr const u08 PACKET_CS_SYNC = 250;  // Not used 
constexpr const u08 PACKET_SC_SYNC = 251;  // used, relocate 

constexpr const u08 PACKET_CODE = 0x89; 

#pragma pack(push, 1)
struct PacketHeader {
	u08 code; // 0x89 
	u08 size; // Packet size 
	u08 type; // Packet type 
};
#pragma pack(pop) 

