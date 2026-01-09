#pragma once

#include "WinAtomic.h" 

namespace Core {
	constexpr static const long long FRAME_PER_SECOND = 50;

	Win::Atomic64 running{ -1 };

	LARGE_INTEGER frequency;
	LARGE_INTEGER time_init;

	long long update_per_second = 0; 
	long long select_per_second = 0; 
	long long cpu_spin_per_second = 0; 

	inline void StopRunning() noexcept { running.Store(0); }
	inline const long long GetTick() noexcept {
		LARGE_INTEGER currTime;
		QueryPerformanceCounter(&currTime);
		return currTime.QuadPart;
	}

	bool Initialize() noexcept;
	void Shutdown() noexcept;
	void Run() noexcept;
	void Update() noexcept;
	void Control() noexcept; 
	void Monitor() noexcept; 
}
