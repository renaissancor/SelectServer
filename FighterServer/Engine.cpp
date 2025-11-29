#include "stdafx.h"
#include "Engine.h"
#include "Network.h"

Engine::Engine()
{
	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_time_init);
}

Engine::~Engine() = default;

bool Engine::Initialize() noexcept {
	if (Network::Manager::GetInstance().Initialize() == false) return false; 
	fprintf_s(stdout, "Network Initialization Bind Listen Complete\n"); 
	return true;
}

void Engine::Shutdown() noexcept {
}

void Engine::Run() noexcept {
	_running.Exchange(1); 

	const long long ticks_per_second = GetFrequency();
	const long long tick_per_frame = ticks_per_second / FRAME_PER_SECOND;

	long long ticks_curr_second_start = GetTick();
	long long ticks_next_second_plan = ticks_curr_second_start + ticks_per_second;

	_frameCount = 0;
	timeBeginPeriod(1); 

	Network::Manager& network = Network::Manager::GetInstance(); 

	while (_running) {
		long long ticks_curr_frame = GetTick();
		if (ticks_curr_frame >= ticks_next_second_plan) {
			ticks_curr_second_start = ticks_next_second_plan;
			ticks_next_second_plan += ticks_per_second;

			_FPS_prev = _FPS_curr;
			_FPS_curr = 0;

			_frameCount = 0;
		}

		_frameCount++;
		const long long ticks_curr_frame_limit =
			ticks_curr_second_start + ((long long)_frameCount * ticks_per_second) / FRAME_PER_SECOND;

		// TODO 
		network.BuildFDSets(); 
		network.Poll(); 
		network.ProcessRecvData(); 
		network.Flush(); 

		
		_FPS_curr++;

		ticks_curr_frame = GetTick();
		if (ticks_curr_frame < ticks_curr_frame_limit) {
			long long ticks_to_sleep = ticks_curr_frame_limit - ticks_curr_frame;
			DWORD sleep_ms = static_cast<DWORD>((ticks_to_sleep / (ticks_per_second / 1000)));
			if (sleep_ms > 1) Sleep(sleep_ms);
		}
	}
	timeEndPeriod(1);
}