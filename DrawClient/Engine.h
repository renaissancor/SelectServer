#pragma once

// Engine.h 

class Engine {
private:
	Engine() noexcept; 
	~Engine() = default;
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

private:
	constexpr static const int TARGET_FPS = 100; 

	MSG _msg = { 0 };

	long long _frameCountTotal = 0; 
	int _frameCount = 0;
	int _prevUpdateFPS = 0;
	int _currUpdateFPS = 0;
	int _prevRenderFPS = 0;
	int _currRenderFPS = 0;

	LARGE_INTEGER _frequency = { 0 };
	LARGE_INTEGER _time_init = { 0 }; 

public:
	inline static Engine& GetInstance() noexcept {
		static Engine instance;
		return instance;
	}

	inline long long GetTick() const noexcept {
		LARGE_INTEGER time_now = { 0 };
		QueryPerformanceCounter(&time_now);
		return (time_now.QuadPart - _time_init.QuadPart);
	}

	bool Initialize() noexcept;

	void Update() noexcept;

	void Render() const noexcept;

	void Run() noexcept;

	void Shutdown() noexcept;
};
