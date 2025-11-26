#pragma once

// Protocol.h

#pragma pack(push, 1)
struct PacketHeader {
	uint16_t len; 
};
#pragma pack(pop)

#pragma pack(push, 1)
struct PacketPayload {
	uint32_t ax;
	uint32_t ay;
	uint32_t bx;
	uint32_t by;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct Packet {
	PacketHeader header;
	PacketPayload payload;
};
#pragma pack(pop)
