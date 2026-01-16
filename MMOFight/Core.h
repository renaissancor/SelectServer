#pragma once

#include "WinAtomic.h" 

namespace Core {
	constexpr static const long long FRAME_PER_SECOND = 50;

	extern Win::Atomic64 running;
	extern LARGE_INTEGER frequency;
	extern LARGE_INTEGER time_init;

	extern long long update_per_second;
	extern long long select_per_second;
	extern long long cpu_spin_per_second;

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
