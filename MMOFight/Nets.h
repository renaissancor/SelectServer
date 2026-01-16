#pragma once
#include <unordered_map>
#include <vector>
#include "ring_buffer.h" 

namespace Nets {
    constexpr static const u_short SERVER_PORT = 20000;

    struct Session {
        i32 id = -1;
        SOCKET socket = INVALID_SOCKET;
        i64 last_recv_time = 0;
        ring_buffer recvQ;
        ring_buffer sendQ;
    };

    extern std::unordered_map<SOCKET, Session*> session_map;
    extern std::vector<SOCKET> reserved_close_sockets;

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
    void DisconnectSession(SOCKET sock) noexcept;
}