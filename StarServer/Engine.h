#pragma once

// Engine.h 

class Engine : public Singleton<Engine>
{
private:
	Engine();
	~Engine();
	friend class Singleton<Engine>;

	constexpr static const uint64_t FRAMES_PER_SECOND = 50;

	WinAtomic<long long> _running; 

	uint64_t _frameCount = 0;
	uint32_t _FPS_prev = 0;
	uint32_t _FPS_curr = 0;
	uint32_t _FPS_prev_render = 0;
	uint32_t _FPS_curr_render = 0;
	double _FPS_updated = 0;

	LARGE_INTEGER _frequency;
	LARGE_INTEGER _time_init;

public:
	inline void StopEngine() noexcept { _running.Exchange(0); }
	inline const long long GetFrequency() const noexcept { return _frequency.QuadPart; }
	/*
	inline const long long GetTimeLL() const noexcept {
		LARGE_INTEGER time_current;
		QueryPerformanceCounter(&time_current);
		return ((time_current.QuadPart - _time_init.QuadPart) * 1000) / _frequency.QuadPart;
	}
	*/
	inline const long long GetTick() const noexcept {
		LARGE_INTEGER time_current;
		QueryPerformanceCounter(&time_current);
		return time_current.QuadPart - _time_init.QuadPart;
	}

	bool Initialize() noexcept;
	void Shutdown() noexcept;
	void Run() noexcept;
	void Update() noexcept;
	void Render() const noexcept;
};