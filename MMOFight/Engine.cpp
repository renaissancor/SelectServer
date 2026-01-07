#include "stdafx.h"
#include "Engine.h"
#include "Network.h" 

#include <Windows.h>
#include <immintrin.h>

bool Core::Initialize() noexcept {
    // SetThreadAffinityMask(GetCurrentThread(), 1);
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&time_init); 
    return true; 
}

void Core::Shutdown() noexcept {
	
}

void Core::Run() noexcept {
    const long long ticks_per_second = frequency.QuadPart;
    const long long ticks_per_frame = ticks_per_second / FRAME_PER_SECOND;

    long long tick_prev = GetTick();
    long long tick_next_sec_plan = tick_prev + ticks_per_second;
    long long frame_accumulator = 0;

    uint32_t logic_count_in_sec = 0;
    uint32_t select_count_in_sec = 0;
    uint32_t spin_count_in_sec = 0;

    while (running.Load() > 0) {
        long long tick_curr = GetTick();
        long long delta_tick = tick_curr - tick_prev;
        tick_prev = tick_curr;

        frame_accumulator += delta_tick;
        spin_count_in_sec++;

        // --- 1. Network I/O (Busy wait) ---
        if (Net::Poll()) {
            select_count_in_sec++;
        }

        if (tick_curr >= tick_next_sec_plan) {
            update_per_second = logic_count_in_sec;
            select_per_second = select_count_in_sec;
            cpu_spin_per_second = spin_count_in_sec;

            logic_count_in_sec = 0;
            select_count_in_sec = 0;
            spin_count_in_sec = 0;

            tick_next_sec_plan += ticks_per_second;
            if (tick_curr > tick_next_sec_plan) {
                tick_next_sec_plan = tick_curr + ticks_per_second;
            }
        }

        if (frame_accumulator > ticks_per_frame * 5) {
            frame_accumulator = ticks_per_frame * 5;
        }

        while (frame_accumulator >= ticks_per_frame) {
            Update();
            logic_count_in_sec++;
            frame_accumulator -= ticks_per_frame;
        }

        Net::Flush(); 
        Control();
        Monitor();

        _mm_pause(); 
    }
}

void Core::Update() noexcept {
}

void Core::Control() noexcept { 
    


}

void Core::Monitor() noexcept { 
    static long long last_print = 0;
    long long curr = GetTick();
    if (curr - last_print > frequency.QuadPart) {
        std::cout << "[Monitor] UPS: " << update_per_second
            << " | NW_Poll: " << select_per_second
            << " | Spin: " << cpu_spin_per_second << std::endl;
        last_print = curr;
    }
}

int main() {
	timeBeginPeriod(1);
	Core::Initialize();
	Core::running.Store(1);
	Core::Run();
	Core::Shutdown();
	timeEndPeriod(1);
}

// Game server main loop in busy wait style, 100% CPU usage 
