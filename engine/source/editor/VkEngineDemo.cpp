#include "Engine.h"

int main(int argc, char** argv)
{
	Chandelier::Engine engine;

	Chandelier::EngineInitInfo engine_init_info = {};
    engine_init_info.executable_path            = argv[0];

	engine.Initialize(engine_init_info);
	engine.Run();
    engine.UnInit();

	return 0;
}