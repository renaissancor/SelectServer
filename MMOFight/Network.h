#pragma once

#include "ring_buffer.h" 

namespace Net {
	WSADATA wsaData; 
	constexpr static const u_short SERVER_PORT = 5000;

	struct Session {
		SOCKET socket = INVALID_SOCKET;
		int id = -1; 
		ring_buffer RecvBuffer; 
		ring_buffer SendBuffer; 

	};



	bool Startup() noexcept;
	void Cleanup() noexcept; 
}

bool Net::Startup() noexcept {
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		fprintf_s(stderr, "WSAStartup failed with error: %d\n", result);
		return false; 
	}
	return true; 
} 

void Net::Cleanup() noexcept {
	WSACleanup(); 
}