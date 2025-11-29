#pragma once 

// Engine.h

class Engine {
	constexpr static const uint64_t FRAME_PER_SECOND = 50;

private:
	Engine();
	~Engine();
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;
private:
	WinAtomic<long long> _running;

	uint64_t _frameCount = 0;
	uint32_t _FPS_prev = 0;
	uint32_t _FPS_curr = 0;
	uint32_t _FPS_prev_render = 0;
	uint32_t _FPS_curr_render = 0;

	LARGE_INTEGER _frequency;
	LARGE_INTEGER _time_init;

public:
	inline void StopEngine() noexcept { _running.Exchange(0); }
	inline const long long GetFrequency() const noexcept { return _frequency.QuadPart; }
	inline const long long GetTick() const noexcept {
		LARGE_INTEGER time_current;
		QueryPerformanceCounter(&time_current);
		return time_current.QuadPart - _time_init.QuadPart;
	}

	inline static Engine& GetInstance() noexcept {
		static Engine instance;
		return instance;
	}

	bool Initialize() noexcept;
	void Shutdown() noexcept;
	void Run() noexcept;

};