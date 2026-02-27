#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "tree.h"

typedef struct {
    float zoom;
    float offset_x;
    float offset_y;
} Camera;

void renderer_draw(SDL_Renderer *r, TTF_Font *font, DirNode *root,
                   Camera *cam, int window_w, int window_h);
void render_welcome(SDL_Renderer *r, TTF_Font *font, int w, int h);
void render_scan_indicator(SDL_Renderer *r, TTF_Font *font,
                           uint32_t files, uint64_t size, int w, int h);
