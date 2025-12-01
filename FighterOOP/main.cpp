#include "stdafx.h"
#include "Net.h"
#include "Game.h" 

class Engine {
private:
	LARGE_INTEGER _frequency;
	LARGE_INTEGER _time_init;
	volatile bool _running = false;
	volatile int _frameCount = 0;
	volatile int _FPS_curr = 0;
	volatile int _FPS_prev = 0;
	inline long long GetFrequency() const noexcept {
		return _frequency.QuadPart;
	}
	inline long long GetTick() const noexcept {
		LARGE_INTEGER currTime;
		QueryPerformanceCounter(&currTime);
		return currTime.QuadPart;
	}

public:
	Engine();
	~Engine();
	bool Initialize() noexcept;
	void Shutdown() noexcept;
	void Run() noexcept;

public:
	static Engine& GetInstance() noexcept {
		static Engine instance;
		return instance;
	}
};

Engine::Engine()
{
	QueryPerformanceFrequency(&_frequency);
	QueryPerformanceCounter(&_time_init);
}

Engine::~Engine() = default;

bool Engine::Initialize() noexcept {
	if (Net::Manager::GetInstance().Initialize() == false) return false; 
	fprintf_s(stdout, "Network Initialization Bind Listen Complete\n"); 
	return true;
}

void Engine::Run() noexcept {
	_running = true; 
	const long long ticks_per_second = GetFrequency();
	const long long tick_per_frame = ticks_per_second / 60; // FRAME_PER_SECOND
	long long ticks_curr_second_start = GetTick();
	long long ticks_next_second_plan = ticks_curr_second_start + ticks_per_second;
	_frameCount = 0;
	timeBeginPeriod(1); 
	Net::Manager& network = Net::Manager::GetInstance(); 
	Game::Manager& game = Game::Manager::GetInstance();
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
			ticks_curr_second_start + ((long long)_frameCount * ticks_per_second) / 60; // FRAME_PER_SECOND
		// TODO 
		
		network.BuildFDSets();
		network.Poll();
		game.UpdateAttackDamage(); 
		game.UpdateMovement(); 
		game.UpdateDeath(); 
		network.FlushAll(); 

		_FPS_curr++;
		ticks_curr_frame = GetTick();
		if (ticks_curr_frame < ticks_curr_frame_limit) {
			long long ticks_to_sleep = ticks_curr_frame_limit - ticks_curr_frame;
			DWORD sleep_ms = static_cast<DWORD>((ticks_to_sleep / (ticks_per_second / 1000)));
			if (sleep_ms > 1) Sleep(sleep_ms);
		}
	}
}

void Engine::Shutdown() noexcept {
	Net::Manager::GetInstance().Shutdown();
}

int main() {
	Engine& engine = Engine::GetInstance();
	if (engine.Initialize() == false) {
		fprintf_s(stderr, "Engine Initialization Failed\n");
		return -1;
	}
	engine.Run();
	engine.Shutdown();

	return 0;
}