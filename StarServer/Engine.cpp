#include "stdafx.h"
#include "Engine.h"
#include "Console.h"
#include "TCP.h"
#include "Game.h" 

Engine::Engine()
{
	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_time_init);
}

Engine::~Engine() = default;

bool Engine::Initialize() noexcept {
	if (!Console::Manager::GetInstance().Initialize()) {
		return false;
	}
	if (!TCP::Manager::GetInstance().Initialize()) {
		Console::Manager::GetInstance().Shutdown(); 
		return false;
	}

	if (!Game::Manager::GetInstance().Initialize()) {
		TCP::Manager::GetInstance().Shutdown();
		Console::Manager::GetInstance().Shutdown();
		return false;
	}
	
	return true;
}

void Engine::Update() noexcept {
	TCP::Manager::GetInstance().Update();
	Game::Manager::GetInstance().Update(); 
}

void Engine::Render() const noexcept {
	Game::Manager::GetInstance().Render(); 

	char buffer[80];
	sprintf_s(buffer, "    Frame : %lld", _frameCount);
	Console::Manager::GetInstance().DrawLine(24, 0, buffer);
	sprintf_s(buffer, "FPS (Logic) : %u", _FPS_prev);
	Console::Manager::GetInstance().DrawLine(24, 30, buffer);
	sprintf_s(buffer, "FPS (Render) : %u", _FPS_prev_render);
	Console::Manager::GetInstance().DrawLine(24, 50, buffer);

	Console::Manager::GetInstance().Display();
}

void Engine::Run() noexcept {
	_running.Exchange(1);
	
	const long long ticks_per_second = GetFrequency();
	const long long tick_per_frame = ticks_per_second / FRAMES_PER_SECOND; 

	long long ticks_curr_second_start = GetTick();
	long long ticks_next_second_plan = ticks_curr_second_start + ticks_per_second;

	_frameCount = 0; 
	timeBeginPeriod(1);

	while (_running) {
		long long ticks_curr_frame = GetTick(); 
		if (ticks_curr_frame >= ticks_next_second_plan) {
			ticks_curr_second_start = ticks_next_second_plan;
			ticks_next_second_plan += ticks_per_second; 

			_FPS_prev = _FPS_curr;
			_FPS_curr = 0;

			_FPS_prev_render = _FPS_curr_render;
			_FPS_curr_render = 0;

			_frameCount = 0; 
		}
		
		_frameCount++;
		const long long ticks_curr_frame_limit =
			ticks_curr_second_start + ((long long)_frameCount * ticks_per_second) / FRAMES_PER_SECOND;

		Update();
		_FPS_curr++;

		ticks_curr_frame = GetTick();

		// Simulate some processing delay 
		if (ticks_curr_frame >= ticks_curr_frame_limit) continue;

		for (int i = 0; i < 12; ++i) Sleep(1); 

		_FPS_curr_render++; 
		Render();

		ticks_curr_frame = GetTick();
		if (ticks_curr_frame < ticks_curr_frame_limit) {
			long long ticks_to_sleep = ticks_curr_frame_limit - ticks_curr_frame;
			DWORD sleep_ms = static_cast<DWORD>((ticks_to_sleep / (ticks_per_second / 1000)));
			if (sleep_ms > 1) Sleep(sleep_ms);
		}
	}
	timeEndPeriod(1);
}

void Engine::Shutdown() noexcept {
	Game::Manager::GetInstance().Shutdown();
	TCP::Manager::GetInstance().Shutdown(); 
	Console::Manager::GetInstance().Shutdown();
}