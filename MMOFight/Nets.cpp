#include "stdafx.h"
#include "Nets.h"

namespace Nets {
	WSADATA WSAData;
	i32 WSALastError = 0;
	fd_set master_rset;
	fd_set master_wset;
	SOCKET listen_socket = INVALID_SOCKET;
	sockaddr_in server_address = { 0 };

	std::map<SOCKET, Session*> sessions; 
	

}

bool Nets::Startup() noexcept {

	auto CleanHelper = [&]() noexcept -> bool {
		WSALastError = WSAGetLastError();
		::closesocket(listen_socket);
		::WSACleanup();
		return false;
		};

	WSAData = { 0 };
	const i32 wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (wsa_startup_result != 0) return false; 

	listen_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (listen_socket == INVALID_SOCKET) {
		WSALastError = WSAGetLastError();
		WSACleanup();
		return false; 
	}

	int flag = 1;
	int setsockopt_nagle_result = ::setsockopt(listen_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
	if (setsockopt_nagle_result == SOCKET_ERROR) return CleanHelper(); 

	u_long non_blocking_mode = 1;
	int ioctlsocket_result = ::ioctlsocket(listen_socket, FIONBIO, &non_blocking_mode);
	if (ioctlsocket_result == SOCKET_ERROR) return CleanHelper(); 

	linger so_linger = { 1 , 0 }; // Enable Linger, end by RST 
	int setsockopt_linger_result = ::setsockopt(listen_socket, SOL_SOCKET,
		SO_LINGER, (char*)&so_linger, sizeof(so_linger));
	if (setsockopt_linger_result == SOCKET_ERROR) return CleanHelper();

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	int bind_result = ::bind(listen_socket, 
		(SOCKADDR*)&server_address, sizeof(server_address)); 
	if (bind_result == SOCKET_ERROR) return CleanHelper();

	int listen_result = ::listen(listen_socket, SOMAXCONN);
	if (listen_result == SOCKET_ERROR) return CleanHelper(); 

	return true;
}

void Nets::Cleanup() noexcept {
	WSACleanup();
}

bool Nets::Poll() noexcept {
	
	bool has_data = false; 

	FD_ZERO(&master_rset);
	FD_ZERO(&master_wset); 

	FD_SET(listen_socket, &master_rset);
	timeval timeout = { 0, 0 }; // zero timeout for non-blocking select
	i32 select_result = ::select(0, &master_rset, &master_wset, nullptr, &timeout);
	if (select_result == SOCKET_ERROR) WSALastError = WSAGetLastError();
	if (select_result > 0) {
		has_data = true;
		if(FD_ISSET(listen_socket, &master_rset)) {
			AcceptSessions(); 
		}
	}

	i32 count_isset = 0;
	i32 index_start = 0;

	return has_data; 
}

void Nets::Flush() noexcept {
	
}

int Nets::AcceptSessions() noexcept {
	int accept_count = 0; 
	for (;;) {
		sockaddr client_address = { 0 };
		int client_address_len = sizeof(client_address);
		SOCKET new_socket = ::accept(listen_socket, &client_address, &client_address_len); 

		if (new_socket == INVALID_SOCKET) {
			int wsa_error = WSAGetLastError();
			if (wsa_error != WSAEWOULDBLOCK) WSALastError = wsa_error;
			return accept_count;
		}

		linger so_linger = { 1 , 0 }; // Close by RST 
		::setsockopt(new_socket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));

		u_long non_blocking_mode = 1;
		::ioctlsocket(new_socket, FIONBIO, &non_blocking_mode);
		int flag = 1;
		::setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
		
		Session* session = new Session();
		session->socket = new_socket;
		sessions.insert(std::make_pair(new_socket, session)); 

		accept_count++; 

		// Register New Player Create Event 
	}
}
