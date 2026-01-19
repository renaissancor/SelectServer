#include "stdafx.h"
#include "Nets.h"
#include "Core.h" 

namespace Nets {
	WSADATA WSAData;
	i32 WSALastError = 0;
	fd_set master_rset;
	fd_set master_wset;
	SOCKET listen_socket = INVALID_SOCKET;
	sockaddr_in server_address = { 0 };

	Session* sessions[MAX_SESSIONS] = { nullptr }; 
	std::vector<size_t> session_index_freelist; 
	std::vector<size_t> session_index_to_close;
}

bool Nets::Startup() noexcept {
	auto cleanup_on_failure = []() noexcept -> bool {
		WSALastError = WSAGetLastError(); 
		WSACleanup();
		return false; 
	};
	
	int wsa_startup_result = WSAStartup(MAKEWORD(2, 2), &WSAData); 

	if (wsa_startup_result != 0) return cleanup_on_failure(); 
	listen_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_socket == INVALID_SOCKET) return cleanup_on_failure();
	u_long non_blocking_mode = 1;
	::ioctlsocket(listen_socket, FIONBIO, &non_blocking_mode);
	int optval = 1;
	::setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);
	if (::bind(listen_socket, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
		return cleanup_on_failure();
	}
	if (::listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
		return cleanup_on_failure();
	}

	// Initialize free list 
	session_index_freelist.reserve(MAX_SESSIONS);
	session_index_to_close.reserve(1024); 
	for (size_t i = 0; i < MAX_SESSIONS; ++i) {
		session_index_freelist.push_back(i);
	}

	return true;
}

void Nets::Cleanup() noexcept {
	for (Session* session : sessions) {
		if (session) {
			::closesocket(session->socket);
			delete session;
		}
	}
	::closesocket(listen_socket); 
	WSACleanup();
}

int Nets::Poll() noexcept {
	const timeval timeout = { 0, 0 };
	i32 total_events = 0;

	FD_ZERO(&master_rset);
	FD_SET(listen_socket, &master_rset);
	if (::select(0, &master_rset, nullptr, nullptr, &timeout) > 0) {
		AcceptSessions();
	}

	for (int i = 0; i < MAX_SESSIONS; i += FD_SETSIZE) {
		FD_ZERO(&master_rset);
		int max_at_batch = 0; 
		int active_count = 0; 
		for (int j = i; j < i + FD_SETSIZE; ++j) {
			if (j >= MAX_SESSIONS) break; 
			Session* s = sessions[j];
			if (s) {
				FD_SET(s->socket, &master_rset);
				max_at_batch = j; 
				active_count++;
			}
		}
		if (active_count == 0) continue;

		int select_result = ::select(0, &master_rset, nullptr, nullptr, &timeout); 
		if (select_result > 0) {
			for (int j = i; j <= max_at_batch; ++j) {
				Session* s = sessions[j];
				if (s && FD_ISSET(s->socket, &master_rset)) {
					int free_size = s->recvQ.get_free_size();
					if (free_size > 0) {
						int bytes_to_recv = (free_size < s->recvQ.direct_enqueue_size()) ? free_size : s->recvQ.direct_enqueue_size();
						int received = ::recv(s->socket, (char*)s->recvQ.get_tail_ptr(), bytes_to_recv, 0);
						if (received > 0) {
							s->recvQ.move_tail(received);
							s->last_recv_time = Core::GetTick();
							total_events++;
						}
						else if (received == 0) {
							session_index_to_close.push_back(j); 
						}
						else if (received < 0) {
							int wsa_error = WSAGetLastError();
							if (wsa_error != WSAEWOULDBLOCK) {
								session_index_to_close.push_back(j);
							}
						}
					}
				}
			}
		}
	}

	return total_events;
}

void Nets::Flush() noexcept {
	for (int i = 0; i < MAX_SESSIONS; ++i) {
		Session* s = sessions[i];
		if (s == nullptr) continue; 

		int free_size = s->sendQ.get_used_size();
		if (free_size > 0) {
			int bytes_to_send = (free_size < s->sendQ.direct_dequeue_size()) ? free_size : s->sendQ.direct_dequeue_size();
			int sent = ::send(s->socket, (char*)s->sendQ.get_head_ptr(), bytes_to_send, 0);
			if (sent > 0) {
				s->sendQ.move_head(sent);
			}
		}
	}

	if (session_index_to_close.empty()) return; 
	std::sort(session_index_to_close.begin(), session_index_to_close.end());
	session_index_to_close.erase(
		std::unique(session_index_to_close.begin(), session_index_to_close.end()), 
		session_index_to_close.end());

	for (size_t idx : session_index_to_close) {
		if (sessions[idx] == nullptr) continue;
		::closesocket(sessions[idx]->socket);
		delete sessions[idx];
		sessions[idx] = nullptr;
		session_index_freelist.push_back(idx);
	}
	session_index_to_close.clear();
}

int Nets::AcceptSessions() noexcept {
	int accept_count = 0;
	for (;;) {
		sockaddr_in client_address = { 0 };
		int client_address_len = sizeof(client_address);
		SOCKET new_socket = ::accept(listen_socket, (SOCKADDR*)&client_address, &client_address_len);

		if (new_socket == INVALID_SOCKET) {
			int wsa_error = WSAGetLastError();
			if (wsa_error == WSAEWOULDBLOCK) return accept_count;
			return -1;
		}

		if (session_index_freelist.empty()) {
			::closesocket(new_socket);
			continue;
		}

		size_t free_idx = session_index_freelist.back();
		session_index_freelist.pop_back(); 

		linger so_linger = { 1 , 0 };
		::setsockopt(new_socket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
		u_long non_blocking_mode = 1;
		::ioctlsocket(new_socket, FIONBIO, &non_blocking_mode);
		int flag = 1;
		::setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

		Session* session = new Session();
		session->id = free_idx; 
		session->socket = new_socket;
		session->last_recv_time = Core::GetTick();

		sessions[free_idx] = session; 
		accept_count++;
	}
}

void Nets::DisconnectSession(size_t idx) noexcept {
	if (idx >= MAX_SESSIONS || sessions[idx] == nullptr) return;

	::closesocket(sessions[idx]->socket);
	delete sessions[idx];
	sessions[idx] = nullptr;
	session_index_freelist.push_back(idx);
}