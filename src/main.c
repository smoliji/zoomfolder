#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

#include "tree.h"
#include "renderer.h"
#include "input.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }
    if (!TTF_Init()) {
        fprintf(stderr, "TTF_Init: %s\n", SDL_GetError());
    }

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
        fprintf(stderr, "TTF_OpenFont failed: %s (path: %s)\n",
                SDL_GetError(), font_path);

    DirNode *root = tree_create("/test");
    DirNode *docs = tree_add_child(root, "Documents");
    docs->size = 500000000;
    DirNode *pics = tree_add_child(root, "Pictures");
    pics->size = 300000000;
    DirNode *code = tree_add_child(root, "Code");
    code->size = 200000000;
    root->size = 1000000000;

    DirNode *work = tree_add_child(docs, "Work");
    work->size = 350000000;
    DirNode *personal = tree_add_child(docs, "Personal");
    personal->size = 150000000;

    DirNode *photos = tree_add_child(pics, "Photos");
    photos->size = 200000000;
    DirNode *screenshots = tree_add_child(pics, "Screenshots");
    screenshots->size = 100000000;

    tree_sort_children(root);
    tree_sort_children(docs);
    tree_sort_children(pics);

    Camera cam = {.zoom = 1.0f, .offset_x = 0, .offset_y = 0};

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
            input_handle(&event, &cam, w, h);
        }

        int w, h;
        SDL_GetWindowSize(window, &w, &h);

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);
        renderer_draw(renderer, font, root, &cam, w, h);
        SDL_RenderPresent(renderer);
    }

    tree_free(root);
    if (font) TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
