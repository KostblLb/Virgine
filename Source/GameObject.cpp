#include "GameObject.h"
#include <cstdio>
#include <iostream>
GameObject::GameObject() :surface(NULL), rotation(0), flip(false), texture(NULL)
{
	velocity = Vector2f();
	tag = "";
}

GameObject::GameObject(const char* surfacePath, SDL_Renderer* pren, std::string t, std::string parent, Vector2f pos, int frames) :
rotation(0),
flip(false),
pos(pos),
AnimationFrame(0),
AnimationFramesTotal(frames),
parent(parent),
tag(t)
{
	velocity = Vector2f();
	surface = LoadSurface(surfacePath);
	texture = SDL_CreateTextureFromSurface(pren, surface);
}


GameObject::~GameObject()
{
	if (surface)
		SDL_FreeSurface(surface);
	if (texture)
		SDL_DestroyTexture(texture);
}

SDL_Surface* GameObject::LoadSurface(const char* file){
	SDL_Surface* t = IMG_Load(file);
	fprintf(stderr, "\n%s\n", SDL_GetError());
	return t;
}

void GameObject::translate(float x, float y){
	pos += Vector2f(x, y);
}

void GameObject::relocate(float x, float y){
	pos = Vector2f(x, y);
}

void GameObject::addVelocity(float x, float y){
	velocity += Vector2f(x, y);
}

void GameObject::setVelocity(float x, float y){
	velocity = Vector2f(x, y);
}

void GameObject::rotate(float angle){
	rotation += angle;
}

SDL_Rect GameObject::getRect(){
	SDL_Rect r = surface->clip_rect;
	r.x = pos.x();
	r.y = pos.y();
	if (AnimationFramesTotal)
		r.w /= AnimationFramesTotal;
	return r;
}

void GameObject::Update(){
	translate(velocity.x(), velocity.y());
}

void GameObject::Render(SDL_Renderer* ren){
	if (!AnimationFramesTotal)
		SDL_RenderCopy(ren, texture, NULL, &this->getRect());
	else{
		SDL_Rect AnimFrameRect = getRect();
		SDL_Rect dst = getRect();
		//AnimFrameRect.w /= AnimationFramesTotal;
		AnimFrameRect.y = 0;
		AnimFrameRect.x = AnimFrameRect.w * AnimationFrame;
		SDL_RenderCopy(ren, texture, &AnimFrameRect, &dst);
		AnimationFrame = (AnimationFrame + 1) % AnimationFramesTotal;
	}
}