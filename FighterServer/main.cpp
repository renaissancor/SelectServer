#include "stdafx.h"
#include "Engine.h"

int main()
{
	Engine& engine = Engine::GetInstance(); 
	if (!engine.Initialize()) {
		fprintf_s(stderr, "Engine initialization failed.\n"); 
		return -1;
	}
	engine.Run();
	engine.Shutdown();
    
	return 0; 
}
