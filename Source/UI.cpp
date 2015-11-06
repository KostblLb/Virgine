#include "UI.h"

UI::UI()
{
	mouseRect.x = 0;
	mouseRect.y = 0;
	mouseRect.w = 1;
	mouseRect.h = 1;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
}

UI::UI(SDL_Rect* mwr, SDL_Renderer* rnd) :mainWindowRect(mwr), renderer(rnd), font(NULL), text_texture(NULL){
	mouseRect.x = 0;
	mouseRect.y = 0;
	mouseRect.w = 1;
	mouseRect.h = 1;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	loadFont("resources/liberationmono-regular.ttf");
}

UI::~UI()
{
}

void UI::cleanup(){
	SDL_DestroyTexture(text_texture);
	TTF_CloseFont(font);
	for (text_container t : texts){
		if (t.texture)
			SDL_DestroyTexture(t.texture);
	}
}

SDL_Rect* UI::getMouseRect(){
	return &mouseRect;
}

bool UI::loadFont(std::string path){
	font = TTF_OpenFont(path.c_str(), mainWindowRect->h / 20);
	if (!font)
		return false;
	SDL_Surface* sur1 = fillAtlas(0, { 255, 255, 255 });
	//SDL_Surface* sur2 = fillAtlas(1, { 255, 255, 255 });
	//SDL_BlitSurface(sur2, &sur1->clip_rect, sur1, &sur2->clip_rect);
	atlas = SDL_CreateTextureFromSurface(renderer, sur1);
	SDL_FreeSurface(sur1);
	//SDL_FreeSurface(sur2);
	return true;
}

SDL_Surface* UI::fillAtlas(int o, SDL_Color color){ //убрать дубль кода
	TTF_SetFontOutline(font, o);
	int outline = TTF_GetFontOutline(font);
	SDL_Surface* atlasSurface = NULL;
	int w = 0;
	int h = 0;
	//get a surface and a placement in the global texture for every char
	for (Uint16 ch = 0x20; ch < 0x7F; ch++){ //ASCII chars from 0x20
		int offset = ch - 0x20;
		if (ch >= 0x400) offset += 0x7f - 0x400;
		gy[ch - 0x20].glyph_surface = TTF_RenderGlyph_Shaded(font, ch, color, { 0, 0, 0 });
		if (!gy[ch - 0x20].glyph_surface)
			fprintf(stderr, "%d, %s\n", ch, TTF_GetError());
		TTF_GlyphMetrics(font, ch, &gy[ch - 0x20].minx, &gy[ch - 0x20].maxx, &gy[ch - 0x20].miny, &gy[ch - 0x20].maxy, &gy[ch - 0x20].advance);
		gy[ch-0x20].advance += outline;
		gy[ch - 0x20].atlasx = w;
		w += gy[ch - 0x20].advance;
		h = h>gy[ch - 0x20].maxy - gy[ch - 0x20].miny ? h : gy[ch - 0x20].maxy - gy[ch - 0x20].miny;
	}
	for (Uint16 ch = 0x400; ch < 0x4FF; ch++){ //Cyrillic chars
		int offset = 0x7F - 0x20 + (ch - 0x400); //offset in glyphs array
		gy[offset].glyph_surface = TTF_RenderGlyph_Shaded(font, ch, color, { 0, 0, 0 });
		if (!gy[offset].glyph_surface)
			fprintf(stderr, "%s\n", TTF_GetError());
		TTF_GlyphMetrics(font, ch, &gy[offset].minx, &gy[offset].maxx, &gy[offset].miny, &gy[offset].maxy, &gy[offset].advance);
		gy[offset].advance += outline;
		gy[offset].atlasx = w;
		w += gy[offset].advance+outline;
		h = h>gy[offset].maxy - gy[offset].miny ? h : gy[offset].maxy - gy[offset].miny+2*outline;
	}


	atlasSurface = SDL_CreateRGBSurface(0, w, h, 32,
		rmask, gmask, bmask, amask);
	if (!atlasSurface){
		fprintf(stderr, "UI::fillAtlas: %s\n", SDL_GetError());
	}
	for (int i = 0; i < 0x7f - 0x20 + 0xff; i++){
		SDL_Rect r = gy[i].glyph_surface->clip_rect;
		r.x = gy[i].atlasx-outline;
		//r.w += outline;
		SDL_BlitSurface(gy[i].glyph_surface, NULL, atlasSurface, &r);
	}
	/*atlas = SDL_CreateTextureFromSurface(renderer, atlasSurface);
	if (!atlas)
		fprintf(stderr, "UI:fillAtlas: %s\n", SDL_GetError());
	
	SDL_FreeSurface(atlasSurface);*/
	atlasRect = atlasSurface->clip_rect;
	return atlasSurface;
}

std::vector<Uint16> toUTF8(const char* ch){
	int c;
	std::vector<Uint16> res;
	while (*ch){ //example: in unicode cyrillic 'ј' is concatenation of 2 8-bit uints: 208 and 144
		Uint16 u16 = 0;
		if ((*ch & 0xc000) == 0xc000){ //208 == 1101 0000, ok; first 110 is unicode reserved and should be masked out
			u16 = (0x1F & *ch); //0x1F == 0001 1111 - mask; u16 == 0000 0000 0001 0000
			u16 = u16 << 6;  //u16 == 0000 0100 0000 0000
			ch++; //144 == 1001 0000; first 10 is unicode reserved
			u16 = u16 | (*ch & 0x3F); //0x3f == 0011 1111
			res.push_back(u16);
		}
		else res.push_back(*ch);
		ch++;
	}
	res.push_back(0);
	return res;
}
bool UI::createTextTexture(text_container& tc){
	/*if (text_texture){
		SDL_DestroyTexture(text_texture);
		text_texture = 0;
	}*/
	//SDL_Surface* stringSurface;
	/*Uint8 ch[100];
	memcpy(ch, t.text.c_str(), t.text.size() * 2 * sizeof(Uint8));
	for (int i = 0; i < 100; i++){
	fprintf(stdout, "%d ", (ch[i] & 0xc0) == 0xc0);
	}*/
	//
	//std::vector<Uint16> utf8_text = toUTF8(text.c_str());
	float brdx = 0.05*mainWindowRect->w;
	float brdy = 0.05*mainWindowRect->h;
	/*if (tc.pos.x< (int)brdx)
		tc.pos.x = (int)brdx;
	
	if (tc.pos.y < (int)brdy)
		tc.pos.y = (int)brdy;
*/
	//int wtext = tc.pos.x < mainWindowRect->w - tc.pos.x ? 4 * (tc.pos.x ) : 4 * (mainWindowRect->w - tc.pos.x );
	tc.pos.w = mainWindowRect->w-2*brdx;
	tc.pos.x = brdx;
	int wtext = tc.pos.w;
	//if (tc.pos.x+wtext >mainWindowRect->w - (int)brdx)
	//	tc.pos.x = mainWindowRect->w - (int)brdx-wtext;
	int htext;
	struct line_info{
		int len;
		int width;
	};
	std::vector<line_info> lines;
	int offset, t_len = 0, t_width = 0, i = 0;
	lines.push_back({ 0, 0 });
	
	//сделать контроль  ќ„≈Ќ№ длинных слов
	while (tc.utf8_text[i] != 0){ //возможно, стоит прикрутить поиск по регул€рным выражени€м
		while (tc.utf8_text[i] != ' ' && tc.utf8_text[i] != 0){
			offset = tc.utf8_text[i] - 0x20;
			if (tc.utf8_text[i] >= 0x400)
				offset += 0x7f - 0x400;
			t_len++; t_width += gy[offset].advance;
			i++;
		}
		if (tc.utf8_text[i] == 0){ //EOF
			lines.back().len = t_len+1;
			lines.back().width = t_width+gy[offset].advance;
			continue;
		}
		else if (t_width + gy[' '].advance <= wtext){ //если строка вмещает слово и пробел после него
			lines.back().len = ++t_len;
			lines.back().width = t_width += gy[' '].advance;
		}
		else if (t_width <= wtext){ //если вмещает только слово, добавл€ем его в строку и переходим на новую
			
			lines.back().len = t_len;
			lines.back().width = t_width;
			lines.push_back({ 0, 0 });
			t_len = 0; t_width = 0;
		}
		else{ //иначе - переходим на новую строку
			i -= t_len - lines.back().len+1 ;
			lines.push_back({ 0, 0 });
			t_len = 0; t_width = 0;
		};
		i++;
	}
	htext = lines.size()*atlasRect.h;
	tc.pos.h = htext;
	tc.pos.y = mainWindowRect->h - htext - brdy;
	//if (tc.pos.y + htext>mainWindowRect->h - (int)brdy)
	//	tc.pos.y = mainWindowRect->h - (int)brdy - htext;
	tc.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, wtext, htext); //на неЄ рендерим символы из атласа
	if (!tc.texture){
		fprintf(stderr, "UI::rendertext: %s\n", SDL_GetError());
		return false;
	}
	SDL_RenderPresent(renderer);//ок, если не пр€мо посреди рендера
	SDL_SetTextureBlendMode(tc.texture, SDL_BLENDMODE_BLEND);
	SDL_SetRenderTarget(renderer, tc.texture);
	for (int i = 0, x = 0, y = 0, j = 0; i < lines.size(); j += lines[i].len, i++, y += atlasRect.h){
		x = (int)(wtext - lines[i].width) / 2;
		for (int k = 0; k < lines[i].len; k++){
			int offset = tc.utf8_text[j + k] - 0x20;
			if (tc.utf8_text[j+k] >= 0x400)
				offset += 0x7f - 0x400;
			SDL_Rect rsrc, rdst;
			rsrc.x = gy[offset].atlasx;
			rsrc.y = 0;
			rsrc.h = atlasRect.h;
			rsrc.w = gy[offset].advance;
			rdst.w = rsrc.w;
			rdst.h = rsrc.h;
			rdst.x = x;
			rdst.y = y;
			SDL_RenderCopy(renderer, atlas, &rsrc, &rdst);
			x += rsrc.w;
		}
	}
	SDL_RenderPresent(renderer);
	SDL_SetRenderTarget(renderer, NULL);

	//SDL_RenderCopy(renderer, tc.texture, NULL, &tc.pos);
	//SDL_RenderPresent(renderer);
	return tc.texture != NULL;

	/*SDL_Surface* sur = TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, mainWindowRect->w - tc.pos.x);
	if (!sur){
		fprintf(stderr, "ui::rendertext error: %s\n", TTF_GetError());
		return false;
	}
	else{
		pos->h = sur->h;
		pos->w = sur->w;
		text_texture = SDL_CreateTextureFromSurface(renderer, sur);
		if (!text_texture){
			fprintf(stderr, "ui::rendertext error: %s\n", SDL_GetError());
		}
		SDL_FreeSurface(sur);
	}
	return text_texture != NULL;*/
}

void UI::render(){
	/*for (int i = 0; i < texts.size(); i++){
		text_container text_to_render = texts[i];
		if (renderText(text_to_render.text, &text_to_render.pos, text_to_render.color, text_to_render)){
			int e = SDL_RenderCopy(renderer, text_texture, NULL, &text_to_render.pos);
			if (e) fprintf(stderr, "%s\n", SDL_GetError());
		}
	}*/
	for (int i = 0; i < texts.size(); i++){
		if (!texts[i].texture)
			if (!createTextTexture(texts[i])) fprintf(stderr, "error creating text texture\n");
		int e = SDL_RenderCopy(renderer, texts[i].texture, NULL, &texts[i].pos);
		if (e) fprintf(stderr, "%s\n", SDL_GetError());
	}
}
void UI::addText(text_container t){
	texts.push_back(t);
}