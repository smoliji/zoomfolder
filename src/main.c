#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <nfd.h>
#include <stdio.h>

#include "tree.h"
#include "scanner.h"
#include "renderer.h"
#include "input.h"
#include "font_cache.h"

typedef enum { STATE_WELCOME, STATE_SCANNING, STATE_VIEWING } AppState;

static void open_folder(ScanContext **scan, Camera *cam, AppState *state,
                        FontCache *cache)
{
    nfdchar_t *path = NULL;
    if (NFD_PickFolder(&path, NULL) == NFD_OKAY) {
        if (*scan) scanner_free(*scan);
        *scan = scanner_start(path);
        *cam = (Camera){.zoom = 1.0f, .target_zoom = 1.0f};
        *state = STATE_SCANNING;
        font_cache_clear(cache);
        NFD_FreePath(path);
    }
}

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (!TTF_Init())
        fprintf(stderr, "TTF_Init: %s\n", SDL_GetError());
    NFD_Init();

    SDL_Window *window = SDL_CreateWindow("Zoomfolder", 1280, 720,
                                          SDL_WINDOW_RESIZABLE);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderVSync(renderer, 1);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        return 1;
    }

    char font_path[4096];
    const char *base = SDL_GetBasePath();
    snprintf(font_path, sizeof(font_path), "%sfonts/Inter-Regular.ttf", base);
    TTF_Font *font = TTF_OpenFont(font_path, 14);
    if (!font)
        fprintf(stderr, "TTF_OpenFont: %s (path: %s)\n",
                SDL_GetError(), font_path);

    FontCache *cache = font_cache_create(512);

    AppState state = STATE_WELCOME;
    ScanContext *scan = NULL;
    Camera cam = {.zoom = 1.0f, .target_zoom = 1.0f};
    uint64_t last_tick = SDL_GetTicksNS();

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }

            int w, h;
            SDL_GetWindowSize(window, &w, &h);

            if (event.type == SDL_EVENT_KEY_DOWN &&
                event.key.key == SDLK_O) {
                open_folder(&scan, &cam, &state, cache);
            }

            if (state != STATE_WELCOME)
                input_handle(&event, &cam, w, h);
        }

        uint64_t now = SDL_GetTicksNS();
        float dt = (float)(now - last_tick) / 1e9f;
        last_tick = now;
        if (dt > 0.05f) dt = 0.05f;

        camera_update(&cam, dt);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        SDL_SetRenderDrawColor(renderer, 28, 28, 38, 255);
        SDL_RenderClear(renderer);
        render_background(renderer, w, h);

        if (state == STATE_WELCOME) {
            render_welcome(renderer, font, cache, w, h);
        } else {
            float mx = 0, my = 0;
            SDL_GetMouseState(&mx, &my);

            SDL_LockMutex(scan->mutex);
            renderer_animate(scan->root, dt);
            DirNode *hovered = renderer_hit_test(scan->root, &cam, w, mx, my);
            renderer_draw(renderer, font, cache, scan->root, &cam,
                          hovered, w, h);
            bool done = scan->done;
            uint64_t total = scan->total_size;
            uint32_t files = scan->total_files;
            SDL_UnlockMutex(scan->mutex);

            if (!done) {
                render_scan_indicator(renderer, font, cache,
                                     files, total, w, h);
            } else if (state == STATE_SCANNING) {
                state = STATE_VIEWING;
            }

            if (hovered)
                render_tooltip(renderer, font, cache, hovered,
                               mx, my, w, h);
        }

        SDL_RenderPresent(renderer);
    }

    if (scan) scanner_free(scan);
    font_cache_free(cache);
    if (font) TTF_CloseFont(font);
    NFD_Quit();
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
