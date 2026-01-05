#pragma once

class Engine {
private:
	LARGE_INTEGER _frequency;
	LARGE_INTEGER _time_init; 

	bool _shutdown = 0; 

public:
	static Engine& GetInstance() noexcept {
		static Engine instance;
		return instance;
	}

	Engine(); 
	~Engine();
	bool Initialize() noexcept;
	void Shutdown() noexcept;
	void Run() noexcept;

};