#include "stdafx.h"
#include "Engine.h"

int main()
{
	Engine& engine = Engine::GetInstance(); 
	if (!engine.Initialize()) {
		std::cerr << "Engine initialization failed." << std::endl;
		return -1;
	}
	engine.Run();
	engine.Shutdown();
    
	return 0; 
}
