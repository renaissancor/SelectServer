#include "echo_client.h"
// Client Echo 
using namespace std; 

constexpr const wchar_t* SERVER_IP = L"127.0.0.1"; // Loopback Address 
constexpr const unsigned short SERVER_PORT = 9000; 

volatile LONG running = 1; 

unsigned __stdcall recv_thread_func(void* param)
{
	SOCKET hSocket = (SOCKET)(SIZE_T)param; 
	char recv_buf[256] = { 0 };
	while (InterlockedCompareExchange(&running, 1, 1)) 
	{
		int ret = ::recv(hSocket, recv_buf, sizeof(recv_buf), 0);
		if (ret > 0) {
			fprintf_s(stdout, "\nFrom Server : %s\n", recv_buf);
			fflush(stdout); 
		}
		else if (ret == 0) {
			fprintf_s(stdout, "\nServer disconnected.\n");
			InterlockedExchange(&running, 0); 
			break;
		}
		else {
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) {
				Sleep(10);
				continue;
			}
			else {
				fprintf_s(stderr, "Recv Failed! Error Code : %d\n", err);
				InterlockedExchange(&running, 0); 
				break;
			}
		}
		memset(recv_buf, 0, sizeof(recv_buf));
	}
	return 0; 
}

int main(int argc, wchar_t *argv[])
{
	fprintf_s(stdout, "Process Client Start!\n");
	
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		fprintf_s(stderr, "WSAStartup Failed! Error Code : %d\n", WSAGetLastError());
		return -1;
	}

	SOCKET hClientSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
	if (hClientSocket == INVALID_SOCKET) {
		fprintf_s(stderr, "Socket Create Failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup();
		return -1;
	}

	SOCKADDR_IN server_addr = { 0 };
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	InetPton(AF_INET, SERVER_IP, &server_addr.sin_addr); // Only SERVER_IP 

	int connect_status = ::connect(hClientSocket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (connect_status == SOCKET_ERROR) {
		fprintf_s(stderr, "Socket Connect Failed! Error Code : %d\n", WSAGetLastError());
		closesocket(hClientSocket);
		::WSACleanup();
		return -1;
	}

	int opt = 1;
	::setsockopt(hClientSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&opt, sizeof(opt));

	// u_long mode = 1; ioctlsocket(hClientSocket, FIONBIO, &mode); 

	unsigned threadId = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(
		nullptr,
		0,
		&recv_thread_func,
		(void*)(SIZE_T)hClientSocket,
		0,
		&threadId
	);

	if (!hThread) {
		fprintf(stderr, "Failed to create recv thread\n");
		closesocket(hClientSocket);
		WSACleanup();
		return -1;
	}

	char send_buf[256] = { 0 };
	
	while (InterlockedCompareExchange(&running, 1, 1))
	{
		printf("> ");
		if (!fgets(send_buf, sizeof(send_buf), stdin)) {
			// EOF or error
			InterlockedExchange(&running, 0);
			break;
		}

		size_t len = strnlen_s(send_buf, sizeof(send_buf));
		if (len > 0 && send_buf[len - 1] == '\n') {
			send_buf[len - 1] = '\0';
		}

		if (strcmp(send_buf, "exit") == 0) {
			InterlockedExchange(&running, 0);
			// shutdown to unblock recv thread if it's blocking
			break;
		}
		int sent = ::send(hClientSocket, send_buf, (int)strlen(send_buf), 0);
		if (sent == SOCKET_ERROR) {
			fprintf(stderr, "Send Failed! Code : %d\n", WSAGetLastError());
			// continue; // decide whether to continue or break
		} 
		memset(send_buf, 0, sizeof(send_buf));
	}

	shutdown(hClientSocket, SD_BOTH);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	::closesocket(hClientSocket);
	::WSACleanup(); 

    return 0; 
}
