#include <Core/Game.h>
#include <System/Log.h>

void Game::EngineStarted()
{
	Columbus::Log::Message("ENGINE STARTED");
}

void Game::Update()
{
	static bool once = true;
	if (once)
		Columbus::Log::Message("UPDATE");
	once = false;
}

void Game::Shutdown()
{
	Columbus::Log::Message("SHUTDOWN");
}
