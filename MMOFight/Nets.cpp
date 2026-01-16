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

	std::unordered_map<SOCKET, Session*> session_map;
	std::vector<SOCKET> reserved_close_sockets;
}

void Nets::Cleanup() noexcept {
	for (auto& pair : session_map) {
		if (pair.second) {
			::closesocket(pair.second->socket);
			delete pair.second;
		}
	}
	session_map.clear();
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

	auto it = session_map.begin();
	while (it != session_map.end()) {
		FD_ZERO(&master_rset);

		Session* batch[FD_SETSIZE];
		int batch_count = 0;

		for (; it != session_map.end() && batch_count < FD_SETSIZE; ++it) {
			batch[batch_count++] = it->second;
			FD_SET(it->second->socket, &master_rset);
		}

		int select_result = ::select(0, &master_rset, nullptr, nullptr, &timeout);

		if (select_result > 0) {
			int events_to_process = select_result;
			total_events += select_result;

			for (int i = 0; i < batch_count && events_to_process > 0; ++i) {
				if (FD_ISSET(batch[i]->socket, &master_rset)) {
					events_to_process--;

					Session* s = batch[i];
					int free_size = s->recvQ.get_free_size();
					if (free_size <= 0) {
						reserved_close_sockets.push_back(s->socket);
						continue;
					}

					int bytes_to_recv = (free_size < s->recvQ.direct_enqueue_size()) ? free_size : s->recvQ.direct_enqueue_size();
					int received = ::recv(s->socket, (char*)s->recvQ.get_tail_ptr(), bytes_to_recv, 0);

					if (received > 0) {
						s->recvQ.move_tail(received);
						s->last_recv_time = Core::GetTick();
					}
					else if (received == 0) {
						reserved_close_sockets.push_back(s->socket);
					}
					else {
						int wsa_error = WSAGetLastError();
						if (wsa_error != WSAEWOULDBLOCK) {
							reserved_close_sockets.push_back(s->socket);
						}
					}
				}
			}
		}
	}
	return total_events;
}

void Nets::Flush() noexcept {
	for (auto &it : session_map)  { 
		SOCKET sock = it.first;
		Session* s = it.second; 
		int used = s->sendQ.get_used_size();
		if (used <= 0) continue;

		int bytes_to_send = (used < s->sendQ.direct_dequeue_size()) ? used : s->sendQ.direct_dequeue_size();
		int sent = ::send(sock, (const char*)s->sendQ.get_head_ptr(), bytes_to_send, 0);

		if (sent > 0) {
			s->sendQ.move_head(sent);
		}
		else if (sent < 0) {
			int wsa_error = WSAGetLastError();
			if (wsa_error != WSAEWOULDBLOCK) {
				reserved_close_sockets.push_back(sock);
			}
		}
	}

	if (!reserved_close_sockets.empty()) {
		std::sort(reserved_close_sockets.begin(), reserved_close_sockets.end());
		reserved_close_sockets.erase(std::unique(reserved_close_sockets.begin(), reserved_close_sockets.end()), reserved_close_sockets.end());

		for (SOCKET sock : reserved_close_sockets) {
			DisconnectSession(sock);
		}
		reserved_close_sockets.clear();
	}
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

		linger so_linger = { 1 , 0 };
		::setsockopt(new_socket, SOL_SOCKET, SO_LINGER, (char*)&so_linger, sizeof(so_linger));
		u_long non_blocking_mode = 1;
		::ioctlsocket(new_socket, FIONBIO, &non_blocking_mode);
		int flag = 1;
		::setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));

		Session* session = new Session();
		session->socket = new_socket;
		session->last_recv_time = Core::GetTick();

		session_map[new_socket] = session;

		accept_count++;
	}
}

void Nets::DisconnectSession(SOCKET sock) noexcept {
	auto it = session_map.find(sock);
	if (it == session_map.end()) return;

	Session* session = it->second;
	if (session) {
		::closesocket(session->socket);
		delete session;
	}
	session_map.erase(it);
}
