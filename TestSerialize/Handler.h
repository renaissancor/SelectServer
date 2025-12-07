#pragma once

// Remote Procedure Call from L7 to L5 

void Handle_CS_MOVE_START(int index, Packet* packet) noexcept;

void Handle_CS_MOVE_STOP(int index, Packet* packet) noexcept; 
