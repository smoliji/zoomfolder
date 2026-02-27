#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "tree.h"
#include "font_cache.h"

typedef struct {
    float zoom, target_zoom;
    float offset_x, target_offset_x;
    float offset_y, target_offset_y;
} Camera;

void camera_update(Camera *cam, float dt);
void renderer_animate(DirNode *root, float dt);
void renderer_draw(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                   DirNode *root, Camera *cam, DirNode *hovered,
                   int window_w, int window_h);
void render_background(SDL_Renderer *r, int w, int h);
void render_welcome(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                    int w, int h);
void render_scan_indicator(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                           uint32_t files, uint64_t size, int w, int h);

DirNode *renderer_hit_test(DirNode *root, Camera *cam,
                           int window_w, float mx, float my);
void render_tooltip(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                    DirNode *node, float mx, float my, int window_w, int window_h);
