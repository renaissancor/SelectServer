#include "stdafx.h"
#include "Engine.h" 
#include "WindowMain.h"
#include "Network.h"

Engine::Engine() noexcept : 
	_frequency({ 0 }), _time_init({ 0 })
{
	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_time_init);
}

bool Engine::Initialize() noexcept {
	if (!WindowMain::GetInstance().Initialize()) {
		return false;
	}
	if (!Network::GetInstance().Initialize()) {
		WindowMain::GetInstance().Shutdown();
		return false;
	}

	return true;
}

void Engine::Update() noexcept {
	// Update logic can be added here
	WindowMain& window = WindowMain::GetInstance(); 
	Network& network = Network::GetInstance(); 

	network.RecvData();
	network.HandlePacket(); 

	network.SendData(); 
		


}

void Engine::Render() const noexcept {
	WindowMain& window = WindowMain::GetInstance();

	HDC hBackBufferDC = window.GetBackBufferDC();
	HPEN hPen = window.GetPen(PEN_TYPE::RED);
	HBRUSH hBrush = window.GetBrush(BRUSH_TYPE::BLUE);

	SelectObject(hBackBufferDC, hPen);
	SelectObject(hBackBufferDC, hBrush);

	window.DrawRecvPacket(); 

	window.Present();
}

void Engine::Run() noexcept {
	// Connect to server before entering main loop
	if (!Network::GetInstance().TryConnect()) {
		fwprintf_s(stderr, L"Failed to connect to server. Exiting...\n");
		return;
	}
	MSG msg = { 0 };

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		Update();
		Render();

		if (_msg.message == WM_QUIT) break; 
		if (GetAsyncKeyState(VK_ESCAPE)) break; 
	}
}

void Engine::Shutdown() noexcept {
	Network::GetInstance().Shutdown(); 
	WindowMain::GetInstance().Shutdown();
}