#pragma once
#include "3rdParty\SDL2\include\SDL_ttf.h"
#include <stdio.h>
#include <string>
#include <vector>

std::vector< Uint16 > toUTF8(const char*);
struct text_container{
	std::string text;
	std::vector<Uint16> utf8_text;
	SDL_Color color;
	SDL_Rect pos;
	SDL_Texture* texture;
	text_container(std::string text, SDL_Color color = { 0, 0, 0 }, SDL_Rect pos = { 0, 0, 0, 0 }) :text(text), color(color), pos(pos), texture(NULL){
		utf8_text = toUTF8(text.c_str());
	}
	text_container(std::string text, SDL_Color color, int x, int y) :text(text), color(color), texture(NULL){
		utf8_text = toUTF8(text.c_str());
		pos.x = x;
		pos.y = y;
		pos.w = 0;
		pos.h = 0;
	}
};
struct glyph_info{
	SDL_Surface* glyph_surface;
	int minx, miny, maxx, maxy;
	int advance;
	int atlasx;
}; //array of UTF8 chars

class UI
{
	SDL_Rect mouseRect; //прямоулольник под курсором 1х1
	SDL_Rect* mainWindowRect;
	TTF_Font* font;
	SDL_Texture* text_texture;
	SDL_Color text_color;
	SDL_Renderer* renderer;
	std::vector<text_container> texts;
	glyph_info gy[0x7F + 0xFF];
	SDL_Texture* atlas;
	SDL_Rect atlasRect;
	Uint32 rmask, gmask, bmask, amask;
	 //пока что только для одно- и двухбайтных символов юникода
public:
	UI(SDL_Rect* mwr, SDL_Renderer* rnd);
	UI();
	~UI();
	void mouseX(int x){ mouseRect.x = x; }
	void mouseY(int y){ mouseRect.y = y; }
	SDL_Rect* getMouseRect();
	bool createTextTexture(text_container& tc);
	bool loadFont(std::string path);
	SDL_Surface* fillAtlas(int outline, SDL_Color color);
	void render();
	void addText(text_container t);
	void cleanup();
	void text_clear(){ texts.clear(); };
};