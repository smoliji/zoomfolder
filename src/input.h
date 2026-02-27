#pragma once
#include <SDL3/SDL.h>
#include "renderer.h"

void input_handle(SDL_Event *event, Camera *cam, int window_w, int window_h);
