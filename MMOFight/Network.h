#pragma once

#include "ring_buffer.h" 

namespace Net {
	constexpr static const u_short SERVER_PORT = 20000; 
	constexpr static const int MAX_SESSIONS = 24576; 

	struct Session {
		uint32_t id = -1;
		DWORD last_recv_time = 0; 
		SOCKET socket = INVALID_SOCKET; 
		ring_buffer recvQ;
		ring_buffer sendQ;
	}; 

	Session sessions[MAX_SESSIONS];

	WSADATA WSAData; 
	SOCKET listen_socket = INVALID_SOCKET; 
	sockaddr_in server_address = { 0 }; 
	int WSALastError = 0; 

	fd_set master_recv_set;
	fd_set master_send_set;

	// functions 
	bool Startup() noexcept; 
	void Cleanup() noexcept; 

	bool Poll() noexcept;
	void Flush() noexcept; 
}

bool Net::Startup() noexcept {
	WSAData = { 0 }; 
	const int wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &WSAData); 
	if (wsa_startup_result != 0) {
		WSALastError = wsa_startup_result;
		return false;
	}


	return true; 
} 

void Net::Cleanup() noexcept {
	WSACleanup(); 
}

bool Net::Poll() noexcept {
	FD_ZERO(&master_recv_set);
	FD_ZERO(&master_send_set); 

	timeval timeout = { 0, 0 }; // zero timeout for non-blocking select
	int select_result = select(0, &master_recv_set, &master_send_set, nullptr, &timeout);
	if (select_result == SOCKET_ERROR) WSALastError = WSAGetLastError();
	if (select_result > 0) return true; 
	if (select_result == SOCKET_ERROR) WSALastError = WSAGetLastError();
	return false; 
}

void Net::Flush() noexcept { 
	for (int i = 0; i < MAX_SESSIONS; ++i) {
		Session& session = sessions[i]; 
		if (session.socket == INVALID_SOCKET) continue;
		// send data in sendQ 
		::send(
			session.socket, 
			session.sendQ.get_head_ptr(), 
			session.sendQ.get_used_size(), 
			0);
	}
}