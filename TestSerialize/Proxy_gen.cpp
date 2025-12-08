// Auto-generated Proxy.cpp
#include "stdafx.h"
#include "Proxy.h"
#include "Packet.h"
#include "Protocol.h"
#include "Express.h"

void SendCreateMyCharacter(int toSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y, uint8_t hp) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)10 << (uint8_t)Type::SC_CREATE_MY_CHARACTER;
    pkt << id;
    pkt << dir;
    pkt << x;
    pkt << y;
    pkt << hp;
    Express::GetInstance().Unicast(toSession, &pkt);
}

void SendCreateOtherCharacter(int toSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y, uint8_t hp) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)10 << (uint8_t)Type::SC_CREATE_OTHER_CHARACTER;
    pkt << id;
    pkt << dir;
    pkt << x;
    pkt << y;
    pkt << hp;
    Express::GetInstance().Unicast(toSession, &pkt);
}

void SendDeleteCharacter(int exceptSession, uint32_t id) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)4 << (uint8_t)Type::SC_DELETE_CHARACTER;
    pkt << id;
    Express::GetInstance().BroadcastExcept(exceptSession, &pkt);
}

void SendMoveStart(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)9 << (uint8_t)Type::SC_MOVE_START;
    pkt << id;
    pkt << dir;
    pkt << x;
    pkt << y;
    Express::GetInstance().BroadcastExcept(exceptSession, &pkt);
}

void SendMoveStop(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)9 << (uint8_t)Type::SC_MOVE_STOP;
    pkt << id;
    pkt << dir;
    pkt << x;
    pkt << y;
    Express::GetInstance().BroadcastExcept(exceptSession, &pkt);
}

void SendAttack1(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)9 << (uint8_t)Type::SC_ATTACK1;
    pkt << id;
    pkt << dir;
    pkt << x;
    pkt << y;
    Express::GetInstance().BroadcastExcept(exceptSession, &pkt);
}

void SendAttack2(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)9 << (uint8_t)Type::SC_ATTACK2;
    pkt << id;
    pkt << dir;
    pkt << x;
    pkt << y;
    Express::GetInstance().BroadcastExcept(exceptSession, &pkt);
}

void SendAttack3(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)9 << (uint8_t)Type::SC_ATTACK3;
    pkt << id;
    pkt << dir;
    pkt << x;
    pkt << y;
    Express::GetInstance().BroadcastExcept(exceptSession, &pkt);
}

void SendDamage(uint32_t attackerID, uint32_t targetID, uint8_t hp) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)9 << (uint8_t)Type::SC_DAMAGE;
    pkt << attackerID;
    pkt << targetID;
    pkt << hp;
    Express::GetInstance().Broadcast(&pkt);
}

void SendSync(int toSession, uint32_t id, uint16_t x, uint16_t y) noexcept {
    Packet pkt;
    pkt << (uint8_t)0x89 << (uint8_t)8 << (uint8_t)Type::SC_SYNC;
    pkt << id;
    pkt << x;
    pkt << y;
    Express::GetInstance().Unicast(toSession, &pkt);
}
