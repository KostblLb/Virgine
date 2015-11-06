#pragma once
#include "3rdParty\SDL2\include\SDL.h"
#include "3rdParty\lua\include\lua.hpp"
#include "3rdParty\lua\include\lualib.h"
#include "3rdParty\lua\include\lauxlib.h"
#include "Vector2f.h"

#pragma comment(lib, "lua52")
#pragma comment(lib, "sdl2main")
#pragma comment(lib, "sdl2")
#pragma comment(lib, "sdl2_image")
#pragma comment(lib, "sdl2_ttf")

#ifdef _DEBUG
#pragma comment(lib, "ShinyDebug")
#else
#pragma comment(lib,"Shiny")
#endif