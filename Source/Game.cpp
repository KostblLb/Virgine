#define SHINY_PROFILER FALSE
//#define _CRT_SECURE_NO_WARNINGS
#include "Game.h"
//#include "Shiny.h"
#include <cstdio>
#include <exception>
#include <iostream>
#include <iomanip>
#include <queue>
//int i, j;
char filename[256];
SDL_Event e;
static Game* GameInstancePointer = 0;

float clip(float n, float lower, float upper) {
	return std::max(lower, std::min(n, upper));
}

Game* Game::Instance(){
	if (!GameInstancePointer)
		GameInstancePointer = new Game;
	return GameInstancePointer;
}

Game::Game(){}
Game::~Game(){
	for (auto x : _GameObjects)
		delete x;
	_GameObjects.clear();
}
int Game::Init(){
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "info", "initializing...", NULL);
	L = luaL_newstate();
	luaL_openlibs(L);
	int error = luaL_dofile(L, "scripts/setup.lua");
	if (error){
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game init error", "Error running setup script!", 0);
		lua_pop(L, 1);
	}
	lua_getglobal(L, "init_scene");
	initScene = lua_tostring(L, -1);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "info", lua_tostring(L,-1), NULL);
	lua_pop(L, 1);
	lua_getglobal(L, "wnd_options");
	lua_getfield(L, -1, "w");
	winW = lua_tointeger(L, -1);
	lua_getfield(L, -2, "h");
	winH = lua_tointeger(L, -1);
	lua_getfield(L, -3, "label");
	const char* label = lua_tostring(L, -1);
	lua_getfield(L, -4, "fullscreen");
	bool fullscreen = lua_toboolean(L, -1);
	lua_pop(L, 5);
	if (!initScene || !winW || !winH || !label) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game init error", "incorrect init params!", NULL);
		return -1;
	}
	lua_register(L, "c_GetVar", c_GetVar);
	lua_register(L, "c_SetVar", c_SetVar);
	lua_register(L, "c_DisplayText", c_DisplayText);
	lua_register(L, "c_LevelFrames", c_LevelFrames);
	lua_register(L, "c_LevelTime", c_LevelTime);
	lua_register(L, "c_DestroyObject", c_DestroyObject);
	lua_register(L, "c_InstanceName", c_InstanceName);

	int winFlags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) || !IMG_Init(IMG_INIT_PNG)){
		fprintf(stderr, "\n%s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game init error", SDL_GetError(), 0);
		return -1;
	}
	if (TTF_Init() == -1){
		fprintf(stderr, "\n%s\n", TTF_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game init error", TTF_GetError(), 0);
		return -1;
	}
	MainWindow = 0;
	MainWindow = SDL_CreateWindow(label, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, winW, winH, winFlags);
	if (!MainWindow){
		fprintf(stderr, "\n%s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game init error", SDL_GetError(), 0);
		return -1;
	}
	MainRenderer = SDL_CreateRenderer(MainWindow, -1, SDL_RENDERER_ACCELERATED);
	if (MainRenderer == NULL)
	{
		fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game init error", SDL_GetError(), 0);
		return -1;
	}
	SDL_SetRenderDrawColor(MainRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	MainSurface = SDL_GetWindowSurface(MainWindow);
	ui = new UI(getWindowRect(), MainRenderer);
	if (!ui) {
		fprintf(stderr, "Couldn't create UI!");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Game init error", "Couldn't create UI!", 0);
		return -1;
	}

	running = true;
	return 0;
}


void Game::Cleanup(){
	ui->cleanup();
	SDL_FreeSurface(MainSurface);
	SDL_DestroyWindow(MainWindow);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

bool Game::isRunning(){
	//PROFILE_FUNC();
	return running;
}


void Game::HandleEvents(){
	//PROFILE_FUNC();
	while (SDL_PollEvent(&e)){

		switch (e.type)
		{
		case SDL_QUIT:{
			running = false;
			break;
		}
		case SDL_MOUSEBUTTONDOWN:{
			ui->text_clear();
			int i = 0;
			for (i = _GameObjects.size() - 1; i > 0; i--){
				if (SDL_HasIntersection(ui->getMouseRect(), &_GameObjects[i]->getRect()))
					break;
			}
			//qDebug << "hello";
			switch (e.button.button){
			case SDL_BUTTON_LEFT:{
				if (i <= 0)
					break;
				Action1(_GameObjects[i]);
				break;
			}
			case SDL_BUTTON_RIGHT:{
				if (i <= 0)
					break;
				Action2(_GameObjects[i]);
				break;
			}
			default: break;
			}
			break;
		}
		case SDL_MOUSEMOTION:{
			ui->mouseX(e.motion.x);
			ui->mouseY(e.button.y);
			break;
		}
		default:
			break;
		}
	}
}

void Game::ClearScene(){
	
}
void Game::LoadScene(std::string scname){
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "info", "loading scene...", NULL);
	_GameObjects.clear();

	int error = luaL_dofile(L, std::string("scripts/scenes/" + scname).c_str());
	if (error){
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Scene loading error", lua_tostring(L, -1), 0);
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	int s;
	lua_getglobal(L, "scene"); {
		lua_getfield(L, -1, "background");
		_GameObjects.push_back(new GameObject(lua_tostring(L, -1), MainRenderer, "background"));		lua_pop(L, 1);//pop background field
		lua_getfield(L, -1, "objects");
		int t = lua_gettop(L);
		lua_pushnil(L);  /* first key */ //top=3
		while (lua_next(L, t) != 0) {
			lua_getfield(L, -1, "name");
			std::string objname = lua_tostring(L, -1);
			lua_getfield(L, -2, "path");
			lua_getfield(L, -3, "pos_x");
			lua_getfield(L, -4, "pos_y");
			lua_getfield(L, -5, "parent_class");
			std::string objparent = lua_tostring(L, -1);
			lua_getfield(L, -6, "anim_frames");
			int frames = lua_tonumber(L, -1);
			_GameObjects.push_back(new GameObject(
				lua_tostring(L, -5), //path to tex
				MainRenderer,
				lua_tostring(L, -6), //name tag
				lua_tostring(L, -2), //parent class
				Vector2f(lua_tonumber(L, -4)*MainSurface->w, lua_tonumber(L, -3)*MainSurface->h),//x and y are relative to window(0..1)
				lua_tonumber(L, -1)
				));
			lua_pop(L, 7);//remove obj(1) and its attributes(5)

			lua_newtable(L); //create an object table (on top)
			int newtableindex = lua_gettop(L);
			int err = luaL_dofile(L, std::string("scripts/objects/" + objparent).c_str()); //read a class table for the object
			if (err){
				fprintf(stderr, "%s\n", lua_tostring(L, -1));
				lua_pop(L, 1);
			}
			lua_getglobal(L, objparent.c_str()); //get prepared class table (class table on top)
			lua_setfield(L, newtableindex, objparent.c_str()); //put the class table into the object table (obj table on top)
			lua_setglobal(L, objname.c_str()); //set object table as global with name 'objname' (nothing on top)

			//lua_pop(L,1); //pop obj table

		}
		lua_pop(L, 1);
		s = lua_gettop(L);
	}
	lua_pop(L, 1);//pop scene table

	LevelFrames = 0;
	LevelInitTime = SDL_GetTicks();
	LevelTime = 0;
	return;

}


void Game::Render()
{
	//PROFILE_FUNC();
	SDL_RenderClear(MainRenderer);
	if (_GameObjects.size() == 0){
		fprintf(stderr, "\nNothing to render! Level is empty!\n");
		return;
	}
	for (int i = 0; i < _GameObjects.size(); i++){
		ActiveObject = _GameObjects[i];
		_GameObjects[i]->Render(MainRenderer);
		if (_GameObjects[i]->parent != ""){
			lua_getglobal(L, _GameObjects[i]->tag.c_str());
			lua_getfield(L, -1, _GameObjects[i]->parent.c_str());
			lua_getfield(L, -1, "OnFrameUpdate");
			if (!lua_isnil(L, -1)){
				lua_call(L, 0, 0); //call it
				lua_pop(L, 2);
			}
			else
				lua_pop(L, 3);
		}
	}
	ui->render();
	SDL_RenderPresent(MainRenderer);
	LevelFrames++;
}

SDL_Rect* Game::getWindowRect(){
	return &MainSurface->clip_rect;
}

void Game::Update(){
	//PROFILE_FUNC();
	LevelTime = SDL_GetTicks() - LevelInitTime;
	for (GameObject* obj : _GameObjects){
		ActiveObject = obj;
		obj->Update();
		if (obj->parent != ""){
			lua_getglobal(L, obj->tag.c_str());
			lua_getfield(L, -1, obj->parent.c_str());
			lua_getfield(L, -1, "OnUpdate");
			if (!lua_isnil(L, -1)){
				//fprintf(stderr, "w: no update func for %s\n", obj->tag.c_str());
				//else
				lua_call(L, 0, 0);
				lua_pop(L, 2);
			}
			else
				lua_pop(L, 3);
		}
	}
}


void Game::Action1(GameObject* obj){
	ActiveObject = obj;
	if (obj->parent != ""){
		lua_getglobal(L, obj->tag.c_str()); //get the object's table
		lua_getfield(L, -1, obj->parent.c_str()); //get its class table
		lua_getfield(L, -1, "OnLClick"); //get the class method 'onlclick'
		const char* type = lua_typename(L, lua_type(L, -1));
		if (!lua_isnil(L, -1)){
			lua_call(L, 0, 0); //call it
			lua_pop(L, 2);
		}
		else
			lua_pop(L, 3);
		int t = lua_gettop(L);
	}
}
void Game::Action2(GameObject* obj){
	ActiveObject = obj;
	if (obj->parent != ""){
		lua_getglobal(L, obj->tag.c_str()); //get the object's table
		lua_getfield(L, -1, obj->parent.c_str()); //get its class table
		lua_getfield(L, -1, "OnRClick"); //get the class method 'onlclick'
		if (!lua_isnil(L, -1)){
			lua_call(L, 0, 0); //call it
			lua_pop(L, 2);
		}
		else
			lua_pop(L, 3);
		int t = lua_gettop(L);
	}
}

void Game::DestroyObject(std::string objname){
	for (std::vector<GameObject*>::iterator it = _GameObjects.begin(); it != _GameObjects.end(); it++){
		if ((*it)->tag == objname){
			_GameObjects.erase(it);
			break;
		}
	}
}


/*lua: c_GetVar(v)
получает значение переменной v на стеке*/
int c_GetVar(lua_State* L){

	std::queue<std::string> vfields;
	if (!lua_isstring(L, 1)){
		lua_pushstring(L, "c_GetVar: incorrect argument v");
		lua_error(L);
	}
	std::string vname = lua_tostring(L, 1);
	if (vname.find('.', 0) == std::string::npos){
		lua_getglobal(L, vname.c_str());
	}
	else{
		size_t start = 0, len = vname.find('.', 0);
		while (len != std::string::npos){
			len = vname.find('.', start);
			std::string t = vname.substr(start, len);
			vfields.push(t);
			start += len + 1;
		}

		lua_getglobal(L, vfields.front().c_str());
		vfields.pop();
		//int pops = 1;
		while (vfields.size()){
			if (vfields.size() >= 2 && !(lua_type(L, -1) == LUA_TTABLE)){
				lua_pushstring(L, "c_SetVar: v is not a table!");
				lua_error(L);
			}
			lua_getfield(L, -1, vfields.front().c_str());
			vfields.pop();
			//pops++;
		}
	}
	return 1;
}

/*lua: c_SetVar(v,n)
устанавливает значение переменной v на стеке равным n*/
int c_SetVar(lua_State* L){
	std::queue<std::string> vfields;
	if (!lua_isstring(L, 1)){
		lua_pushstring(L, "c_SetVar: incorrect argument v");
		lua_error(L);
	}
	std::string vname = lua_tostring(L, 1);
	if (vname.find('.', 0) == std::string::npos){
		lua_setglobal(L, vname.c_str());
	}
	else{
		size_t start = 0, len = vname.find('.', 0);
		while (len != std::string::npos){
			len = vname.find('.', start);
			std::string t = vname.substr(start, len);
			vfields.push(t);
			start += len + 1;
		}

		lua_getglobal(L, vfields.front().c_str());
		vfields.pop();
		int pops = 1;
		while (vfields.size() > 1){
			if (vfields.size() >= 2 && !(lua_type(L, -1) == LUA_TTABLE)){
				lua_pushstring(L, "c_SetVar: v is not a table!");
				lua_error(L);
			}
			lua_getfield(L, -1, vfields.front().c_str());
			vfields.pop();
			pops++;
		}

		switch (lua_type(L, 2)){ //n==?
		case LUA_TNIL:
			lua_pushnil(L);
			break;
		case LUA_TNUMBER:
			lua_pushnumber(L, lua_tonumber(L, 2));
			break;
		case LUA_TBOOLEAN:
			lua_pushboolean(L, lua_toboolean(L, 2));
			break;
		case LUA_TSTRING:
			lua_pushstring(L, lua_tostring(L, 2));
			break;
		default:
			break;
		}
		lua_setfield(L, -2, vfields.front().c_str());
		lua_pop(L, pops);
	}
	return 0;
}

int c_DisplayText(lua_State* L){
	const char* str = lua_tostring(L, 1);
	int t = lua_gettop(L);
	GameInstancePointer->AddTextToUI(lua_tostring(L, 1));
	return 0;
}

int c_LevelTime(lua_State* L){
	lua_pushnumber(L, GameInstancePointer->getLevelTime());
	return 1;
}

int c_LevelFrames(lua_State* L){
	lua_pushnumber(L, GameInstancePointer->getLevelFrames());
	return 1;
}

int c_DestroyObject(lua_State* L){
	std::string objname = lua_tostring(L, -1);
	GameInstancePointer->DestroyObject(objname);
	return 0;
}

int c_InstanceName(lua_State* L){
	lua_pushstring(L, GameInstancePointer->getActiveObject()->tag.c_str());
	return 1;
}