#pragma once
#include "VgineStd.h"
#include "3rdParty\SDL2\include\SDL_image.h"
#include <string>

/*todo: хэширование по тегу*/
class GameObject{
protected:
	SDL_Surface* surface;
	SDL_Texture* texture;
	bool flip;
	float rotation; /*rotation in rads*/
	Vector2f velocity;
	Vector2f pos;
	int AnimationFrame;
	int AnimationFramesTotal; //actual frames -1
public:
	std::string tag;
	std::string parent;
	SDL_Rect getRect();
	Vector2f getVelocity(){ return velocity; }
	std::string getTag(){ return tag; }
	void translate(float x, float y);
	void relocate(float x, float y);
	void addVelocity(float x, float y);
	void setVelocity(float x, float y);
	void rotate(float rad);
	void getFrameRect();
	GameObject();
	~GameObject();
	GameObject(const char* surfacePath, SDL_Renderer* pren, std::string t = "", std::string parent="", Vector2f pos = Vector2f(), int frames=0);
	SDL_Surface* LoadSurface(const char* file);
	SDL_Surface* getSurface(){ return surface; }
	SDL_Texture* getTexture(){ return texture; }
	void Update();
	void Render(SDL_Renderer* ren);
};