#include "stdafx.h"
#include "Network.h"
#include "WindowMain.h"

// Network.cpp 

Network::Network() noexcept = default;
Network::~Network() noexcept = default;

bool Network::Initialize() noexcept { // Must be called after WindowMain::Initialize 
	HWND hWindow = WindowMain::GetInstance().GetWindowHandle(); 
	if (hWindow == NULL) return false;

	fwprintf_s(stdout, L"Enter Server IP Address : ");
	fwscanf_s(stdin, L"%s", _ipAddress, INET_ADDRSTRLEN);
	// fwprintf_s(stdout, L"%s\n", _ipAddress);
	// wcscpy_s(_ipAddress, SERVER_IP); // For Convenience 

	_results._InetPtonWResult = ::InetPtonW(AF_INET, _ipAddress, &_serverAddr.sin_addr);
	if (_results._InetPtonWResult != 1) {
		fwprintf_s(stderr, L"Invalid IP Address Format!\n");
		return false;
	}
	else {
		wchar_t output_ip[INET_ADDRSTRLEN];
		::InetNtopW(AF_INET, &_serverAddr.sin_addr, output_ip, INET_ADDRSTRLEN);
		fwprintf_s(stdout, L"IP Address Set to %s\n", output_ip);
	}

	_results._WSAStartupResult = ::WSAStartup(MAKEWORD(2, 2), &_wsa);
	if (_results._WSAStartupResult != 0) {
		_results._WSALastErrorResult = WSAGetLastError();
		fwprintf_s(stderr, L"WSAStartup Failed! Error Code : %d\n", WSAGetLastError());
		return false;
	}

	_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_socket == INVALID_SOCKET) {
		_results._SocketCreateResult = WSAGetLastError();
		_results._WSALastErrorResult = WSAGetLastError();
		fwprintf_s(stderr, L"Socket Create Failed! Error Code : %d\n", WSAGetLastError());
		::WSACleanup();
		return false;
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(SERVER_PORT);

	int opt = 1;
	_results._setsockoptResult = 
		::setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt));

	if (_results._setsockoptResult == SOCKET_ERROR) {
		_results._setsockoptResult = WSAGetLastError();
		_results._WSALastErrorResult = _results._setsockoptResult;
		fwprintf_s(stderr, L"Setsockopt Failed! Error Code : %d\n", _results._setsockoptResult);
		::closesocket(_socket);
		::WSACleanup();
		return false;
	}

	linger so_linger = { 0, };
	so_linger.l_onoff = 1; // Enable linger option, end by RST
	so_linger.l_linger = 0;
	_results._setsockoptResult = 
		::setsockopt(_socket, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger));
	if (_results._setsockoptResult == SOCKET_ERROR) {
		_results._setsockoptResult = WSAGetLastError();
		_results._WSALastErrorResult = _results._setsockoptResult;
		fwprintf_s(stderr, L"Setsockopt SO_LINGER Failed! Error Code : %d\n", _results._setsockoptResult);
		::closesocket(_socket);
		::WSACleanup();
		return false;
	}

	return true; 
}

bool Network::TryConnect() noexcept {
	HWND hWindow = WindowMain::GetInstance().GetWindowHandle(); 
	if (hWindow == NULL || _socket == INVALID_SOCKET) return false;

	long events = FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE;

	if(::WSAAsyncSelect(_socket, hWindow, WM_SOCKET_NOTIFY, events) == SOCKET_ERROR) {
		return false;
	}

	_results._connectResult = ::connect(_socket, (SOCKADDR*)&_serverAddr, sizeof(_serverAddr));
	if (_results._connectResult == SOCKET_ERROR) {
		int wsaError = WSAGetLastError();
		if (wsaError == WSAEWOULDBLOCK) {
			fwprintf_s(stdout, L"Connection attempt initiated (WSAEWOULDBLOCK).\n");
			_results._isConnectedToServer = 1;
			return true;
		}
		fwprintf_s(stderr, L"Socket Connect Failed! Error Code : %d\n", wsaError);
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
		_results._isConnectedToServer = 0;
		return false;
	}

	fwprintf_s(stdout, L"Connected to Server successfully (Immediate).\n");
	_results._isConnectedToServer = 1;
	return true;
}

void Network::Shutdown() noexcept {
	if (_socket != INVALID_SOCKET) {
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
	::WSACleanup();
}

void Network::ProcessWSA(WPARAM wParam, LPARAM lParam) noexcept {
	int event = WSAGETSELECTEVENT(lParam);
	int error = WSAGETSELECTERROR(lParam);
	if (error != 0) {
		fwprintf_s(stderr, L"WSAAsyncSelect Error: %d\n", error);
		_results._WSALastErrorResult = error;
		return;
	}
	switch (event) {
	case FD_CONNECT:
		fwprintf_s(stdout, L"FD_CONNECT event received.\n");
		_results._isConnectedToServer = 1;
		break;
	case FD_READ:
		fwprintf_s(stdout, L"FD_READ event received.\n"); // Debug Here 
		RecvData();
		HandlePacket();
		break;
	case FD_WRITE:
		fwprintf_s(stdout, L"FD_WRITE event received.\n");
		// Handle writing data to the socket here 
		_isWritable = true; 
		SendData(); 
		break;
	case FD_CLOSE:
		fwprintf_s(stdout, L"FD_CLOSE event received.\n");
		_results._isConnectedToServer = 0;
		::closesocket(_socket);
		_socket = INVALID_SOCKET;
		break;
	default:
		fwprintf_s(stderr, L"Unknown WSA event: %d\n", event);
		break;
	}
}

void Network::RecvData() noexcept {
	// Implementation for receiving data from the server
	if (_socket == INVALID_SOCKET) return; 

	char recvBuffer[RECV_BUFFER_SIZE]; 
	for (;;) {
		int bytesRecv = ::recv(_socket, recvBuffer, RECV_BUFFER_SIZE, 0);
		if (bytesRecv > 0) {
			fprintf_s(stdout, "Bytes Received: %d\n", bytesRecv);
			_bufferRecv.Enqueue(recvBuffer, bytesRecv);
		}
		else if (bytesRecv == 0) {
			fwprintf_s(stdout, L"Connection closed by the server.\n");
			_results._isConnectedToServer = 0;
			::closesocket(_socket);
			_socket = INVALID_SOCKET;
			break;
		}
		else {
			int wsaError = WSAGetLastError();
			if (wsaError == WSAEWOULDBLOCK) {
				// No more data to read
				break;
			}
			else {
				fwprintf_s(stderr, L"Recv Failed! Error Code : %d\n", wsaError);
				_results._WSALastErrorResult = wsaError;
				break;
			}
		}
	}
}

void Network::HandlePacket() noexcept {
	const size_t PACKET_HEADER_SIZE = sizeof(PacketHeader); // Example header size 
	const size_t PACKET_PAYLOAD_SIZE = sizeof(PacketPayload);
	const size_t PACKET_SIZE = PACKET_HEADER_SIZE + PACKET_PAYLOAD_SIZE; 

	while (true) {
		if(_bufferRecv.GetUsedSize() < PACKET_HEADER_SIZE) break;

		PacketHeader header = { 0 }; 
		int peekHeaderSize = _bufferRecv.Peek((char*)&header, PACKET_HEADER_SIZE);
		if (peekHeaderSize != PACKET_HEADER_SIZE) break; 

		Packet packet = { 0 };
		int peekPayloadSize = _bufferRecv.Peek((char*)&packet, PACKET_PAYLOAD_SIZE);
		if (peekPayloadSize != PACKET_PAYLOAD_SIZE) break; 

		_bufferRecv.Dequeue((char*)&packet, PACKET_SIZE); 

		int ax = packet.payload.ax;
		int ay = packet.payload.ay;
		int bx = packet.payload.bx;
		int by = packet.payload.by;

		// fwprintf_s(stdout, L"Received DRAW: (%d, %d) -> (%d, %d)\n", ax, ay, bx, by); 
		WindowMain::GetInstance().EnqueueLineToDraw(ax, ay, bx, by);
	}
}


void Network::SendDrawPacket(const int ax, const int ay, const int bx, const int by) noexcept {
	Packet packet = { 0 };
	packet.header.len = sizeof(PacketPayload);
	packet.payload.ax = static_cast<uint32_t>(ax);
	packet.payload.ay = static_cast<uint32_t>(ay);
	packet.payload.bx = static_cast<uint32_t>(bx);
	packet.payload.by = static_cast<uint32_t>(by);
	_bufferSend.Enqueue(reinterpret_cast<const char*>(&packet), sizeof(Packet));

	
	packet.payload.ax += 100; 
	packet.payload.bx += 100;
	_bufferSend.Enqueue(reinterpret_cast<const char*>(&packet), sizeof(Packet)); 

	packet.payload.ay += 100;
	packet.payload.by += 100;
	_bufferSend.Enqueue(reinterpret_cast<const char*>(&packet), sizeof(Packet));

	
	packet.payload.ax -= 100;
	packet.payload.bx -= 100;
	_bufferSend.Enqueue(reinterpret_cast<const char*>(&packet), sizeof(Packet)); 

	// SendData(); // Later inside Engine Loop 
}

void Network::SendData() noexcept {
	// WSAAsyncSelect with FD_WRITE will notify when ready to send data
	if (_socket == INVALID_SOCKET || !_isWritable) return;

	if (_bufferSend.GetUsedSize() == 0) return;
	char sendBuffer[SEND_BUFFER_SIZE] = { 0 };

	int bytesToSend = min(_bufferSend.GetUsedSize(), SEND_BUFFER_SIZE);
	_bufferSend.Dequeue(sendBuffer, bytesToSend);
	int bytesSent = ::send(_socket, sendBuffer, bytesToSend, 0);

	if (bytesSent == SOCKET_ERROR) {
		int wsaError = WSAGetLastError();
		if (wsaError == WSAEWOULDBLOCK) {
			_bufferSend.Enqueue(sendBuffer, bytesToSend);
		}
		else {
			fwprintf_s(stderr, L"Send Failed! Error Code : %d\n", wsaError);
			_results._WSALastErrorResult = wsaError;
		}
	}
}

