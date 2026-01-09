#pragma once

#include "ring_buffer.h" 

namespace Nets {
	constexpr static const u_short SERVER_PORT = 20000; 
	constexpr static const i32 MAX_SESSIONS = 16384;

	struct Session {
		i32 id = -1;
		i32 last_recv_time = 0;
		SOCKET socket = INVALID_SOCKET; 
		ring_buffer recvQ;
		ring_buffer sendQ;
	}; 

	extern std::map<SOCKET, Session*> sessions;
	extern int available_session_index;
	extern WSADATA WSAData;
	extern SOCKET listen_socket;
	extern sockaddr_in server_address;
	extern i32 WSALastError;
	extern fd_set master_rset;
	extern fd_set master_wset; 

	

	// functions 
	int GetAvailableSessionIndex() noexcept; 

	bool Startup() noexcept; 
	void Cleanup() noexcept; 

	bool Poll() noexcept;
	void Flush() noexcept; 
	int AcceptSessions() noexcept; 
}
