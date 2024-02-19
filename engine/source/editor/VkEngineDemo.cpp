#include "Engine.h"

int main() {
	Chandelier::Engine engine;
    
	engine.Initialize();
	engine.Run();
    engine.UnInit();

	return 0;
}