#pragma once

namespace Protocol {
#pragma pack(push, 1)
	struct Packet {
		uint32_t type;
		uint32_t id;
		uint32_t x;
		uint32_t y;
	};
#pragma pack(pop) 
}