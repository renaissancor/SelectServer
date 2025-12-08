// Auto-generated Proxy.h
#pragma once
#include <cstdint>

void SendCreateMyCharacter(int toSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y, uint8_t hp) noexcept;
void SendCreateOtherCharacter(int toSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y, uint8_t hp) noexcept;
void SendDeleteCharacter(int exceptSession, uint32_t id) noexcept;
void SendMoveStart(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept;
void SendMoveStop(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept;
void SendAttack1(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept;
void SendAttack2(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept;
void SendAttack3(int exceptSession, uint32_t id, uint8_t dir, uint16_t x, uint16_t y) noexcept;
void SendDamage(uint32_t attackerID, uint32_t targetID, uint8_t hp) noexcept;
void SendSync(int toSession, uint32_t id, uint16_t x, uint16_t y) noexcept;