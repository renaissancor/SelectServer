#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SERVER_IP "192.168.20.25"
#define SERVER_PORT 9999
// 백로그 큐(예: 200)보다 훨씬 많이 시도
#define MAX_ATTEMPTS 20000 

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &servAddr.sin_addr);

    long long success_cnt = 0;
    long long fail_cnt = 0;

    std::cout << "Attempting " << MAX_ATTEMPTS << " connections (fast)..." << std::endl;

    for (int i = 0; i < MAX_ATTEMPTS; ++i) {
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket == INVALID_SOCKET) {
            fail_cnt++;
            continue;
        }

        // 1. 연결 시도
        if (connect(clientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
            // 큐가 가득 차면 'WSAECONNREFUSED' (10061) 발생
            // std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
            fail_cnt++;
        }
        else {
            success_cnt++;
            // 2. 중요: 큐에 넣고 바로 닫음 (테스트 목적)
            // 실제로는 닫지 않아도 큐에 쌓이지만, 핸들 낭비 방지
        }
        closesocket(clientSocket); // 바로 닫기
    }

    std::cout << "================================================" << std::endl;
    std::cout << "Test finished." << std::endl;
    std::cout << "Successful connections (queued): " << success_cnt << std::endl;
    std::cout << "Failed connections (queue full): " << fail_cnt << std::endl;
    std::cout << "================================================" << std::endl;

    WSACleanup();
    return 0;
}