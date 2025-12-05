#pragma once
#include <cstdint>

enum class Type : uint8_t
{
    SC_CREATE_MY_CHARACTER = 0,
    SC_CREATE_OTHER_CHARACTER = 1,
    SC_DELETE_CHARACTER = 2,

    MOVE_DIR_LL = 3,
    MOVE_DIR_LU = 4,
    MOVE_DIR_UU = 5,
    MOVE_DIR_RU = 6,
    MOVE_DIR_RR = 7,
    MOVE_DIR_RD = 8,
    MOVE_DIR_DD = 9,
    MOVE_DIR_LD = 10,

    CS_MOVE_START = 20,
    SC_MOVE_START = 21,
    CS_MOVE_STOP  = 22,
    SC_MOVE_STOP  = 23,

    CS_ATTACK1 = 30,
    SC_ATTACK1 = 31,
    CS_ATTACK2 = 32,
    SC_ATTACK2 = 33,
    CS_ATTACK3 = 34,
    SC_ATTACK3 = 35,

    SC_DAMAGE = 40,

    CS_SYNC = 250,
    SC_SYNC = 251,
};

enum class Dir : uint8_t
{
    LL = 0,
    LU = 1,
    UU = 2,
    RU = 3,
    RR = 4,
    RD = 5,
    DD = 6,
    LD = 7,
};

struct PacketHeader
{
    uint8_t code;
    uint8_t size;
    uint8_t type;
};
