#include "Game.h"
#include "GameObject.h"
int main(int argc, char* argv[]){
	//int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	//tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;

	if (Game::Instance()->Init())
		return (-1);
	Game::Instance()->LoadScene(Game::Instance()->initScene);
	Uint32 ticks = SDL_GetTicks();
	int fps = 5;
	while (Game::Instance()->isRunning()){
		Game::Instance()->HandleEvents();
		//Game::Instance()->CheckCollisions();
		Game::Instance()->Update();
		if (SDL_GetTicks()-ticks>1000/fps)
		{
			Game::Instance()->Render();
			ticks = SDL_GetTicks();
		}
		//PROFILER_UPDATE();
	}
	//PROFILER_OUTPUT();
	Game::Instance()->Cleanup();

	//_CrtDumpMemoryLeaks();
	return 0;
}