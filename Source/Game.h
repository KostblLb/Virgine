#pragma once
#include "VgineStd.h"
#include "UI.h"
#include "GameObject.h"
#include <vector>
#include <map>

int c_GetVar(lua_State* L);
int c_SetVar(lua_State* L);
int c_DisplayText(lua_State* L);
int c_LevelFrames(lua_State* L);
int c_LevelTime(lua_State* L);
int c_DestroyObject(lua_State* L);
int c_InstanceName(lua_State* L);

class Game{
	Game();
	~Game();
	bool running;
	SDL_Window* MainWindow;
	SDL_Surface* MainSurface;
	SDL_Renderer* MainRenderer;
	int winH;
	int winW;
	std::vector<GameObject*> _GameObjects;
	GameObject* ActiveObject;
	unsigned long LevelTime;
	unsigned long LevelFrames;
	unsigned long LevelInitTime;
	//std::map<std::string, GameObject*> GameObjects;
	//Level* level;
	UI* ui;
	lua_State* L;

	//std::map<const char*, int> Stats;
public:
	const char* initScene;
	static Game* Instance();
	SDL_Rect* getWindowRect();
	void LoadScene(std::string);
	void ClearScene();
	int Init();
	void HandleEvents();
	void Update();
	void Render(); 
	void Cleanup();
	bool isRunning();
	void CheckCollisions();
	void Action1(GameObject* obj);
	void Action2(GameObject* obj);
	void AddTextToUI(std::string t){ ui->addText(t); }
	unsigned long getLevelFrames(){ return LevelFrames; }
	unsigned long getLevelTime(){ return LevelTime; }
	GameObject* getActiveObject(){ return ActiveObject; }
	void DestroyObject(std::string);
};
