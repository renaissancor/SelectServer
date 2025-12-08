#pragma once
#include <cstdint>

enum class Type : uint8_t
{
    SC_CREATE_MY_CHARACTER = 0,
    SC_CREATE_OTHER_CHARACTER = 1,
    SC_DELETE_CHARACTER = 2,

    CS_MOVE_START = 10,
    SC_MOVE_START = 11,
    CS_MOVE_STOP  = 12,
    SC_MOVE_STOP  = 13,

    CS_ATTACK1 = 20,
    SC_ATTACK1 = 21,
    CS_ATTACK2 = 22,
    SC_ATTACK2 = 23,
    CS_ATTACK3 = 24,
    SC_ATTACK3 = 25,

    SC_DAMAGE = 30,

    CS_SYNC = 250,
    SC_SYNC = 251,
};

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

struct PacketHeader
{
    uint8_t code;
    uint8_t size;
    uint8_t type;
};
