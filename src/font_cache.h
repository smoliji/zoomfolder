#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct FontCache FontCache;

FontCache   *font_cache_create(int capacity);
SDL_Texture *font_cache_get(FontCache *cache, SDL_Renderer *r,
                            TTF_Font *font, const char *text,
                            SDL_Color color, int *w, int *h);
void         font_cache_clear(FontCache *cache);
void         font_cache_free(FontCache *cache);
