#pragma once

// Protocol.h 

constexpr const uint8_t PACKET_SC_CREATE_MY_CHARACTER = 0;
constexpr const uint8_t PACKET_SC_CREATE_OTHER_CHARACTER = 1; 
constexpr const uint8_t PACKET_SC_DELETE_CHARACTER = 2;
constexpr const uint8_t PACKET_CS_MOVE_START = 10; 

constexpr const uint8_t PACKET_MOVE_DIR_LL = 0; 
constexpr const uint8_t PACKET_MOVE_DIR_LU = 1;
constexpr const uint8_t PACKET_MOVE_DIR_UU = 2;
constexpr const uint8_t PACKET_MOVE_DIR_RU = 3;
constexpr const uint8_t PACKET_MOVE_DIR_RR = 4;
constexpr const uint8_t PACKET_MOVE_DIR_RD = 5;
constexpr const uint8_t PACKET_MOVE_DIR_DD = 6;
constexpr const uint8_t PACKET_MOVE_DIR_LD = 7; 

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
