#pragma once
#include <unordered_map>
#include <vector>
#include "ring_buffer.h" 

namespace Nets {
    constexpr static const u_short SERVER_PORT = 20000;
	constexpr size_t MAX_SESSIONS = 16384;

    struct Session {
        size_t id = -1;
        SOCKET socket = INVALID_SOCKET;
        i64 last_recv_time = 0;
        ring_buffer recvQ;
        ring_buffer sendQ;
    };

	extern Session* sessions[MAX_SESSIONS]; 
	extern std::vector<size_t> session_index_freelist;
	extern std::vector<size_t> session_index_to_close; 

    extern WSADATA WSAData;
    extern SOCKET listen_socket;
    extern sockaddr_in server_address;
    extern i32 WSALastError;
    extern fd_set master_rset;
    extern fd_set master_wset;

    bool Startup() noexcept;
    void Cleanup() noexcept;

    int Poll() noexcept;
    void Flush() noexcept;
    int AcceptSessions() noexcept;
    void DisconnectSession(size_t idx) noexcept;
}