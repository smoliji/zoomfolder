#include "renderer.h"
#include <string.h>
#include <stdio.h>

static const SDL_Color PALETTE[] = {
    {235, 172, 104, 255},
    {130, 204, 171, 255},
    {190, 140, 210, 255},
    {240, 200, 120, 255},
    {120, 180, 220, 255},
    {220, 150, 150, 255},
    {160, 210, 140, 255},
    {200, 170, 130, 255},
    {150, 190, 210, 255},
    {210, 180, 200, 255},
    {170, 200, 160, 255},
    {230, 190, 160, 255},
    {140, 170, 200, 255},
    {200, 210, 140, 255},
    {180, 150, 190, 255},
    {220, 200, 170, 255},
};
#define PALETTE_SIZE (sizeof(PALETTE) / sizeof(PALETTE[0]))

#define ROW_HEIGHT 28
#define ROW_GAP 2
#define LABEL_PAD 4

static uint32_t hash_name(const char *name)
{
    uint32_t h = 5381;
    while (*name)
        h = ((h << 5) + h) ^ (uint8_t)*name++;
    return h;
}

static const char *format_size(uint64_t bytes)
{
    static char buf[32];
    if (bytes >= 1ULL << 30)
        snprintf(buf, sizeof(buf), "%.1f GB", bytes / (double)(1ULL << 30));
    else if (bytes >= 1ULL << 20)
        snprintf(buf, sizeof(buf), "%.1f MB", bytes / (double)(1ULL << 20));
    else if (bytes >= 1ULL << 10)
        snprintf(buf, sizeof(buf), "%.1f KB", bytes / (double)(1ULL << 10));
    else
        snprintf(buf, sizeof(buf), "%llu B", (unsigned long long)bytes);
    return buf;
}

static void draw_node_row(SDL_Renderer *r, TTF_Font *font, DirNode *node,
                          Camera *cam, float x, float y, float w,
                          int depth, int window_w, int window_h)
{
    float sx = (x + cam->offset_x) * cam->zoom;
    float sy = (y + cam->offset_y) * cam->zoom;
    float sw = w * cam->zoom;
    float sh = ROW_HEIGHT * cam->zoom;

    if (sx + sw < 0 || sx > window_w || sy + sh < 0 || sy > window_h)
        return;
    if (sw < 1.0f) return;

    SDL_Color col = PALETTE[hash_name(node->name) % PALETTE_SIZE];
    SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
    SDL_FRect rect = {sx, sy, sw, sh};
    SDL_RenderFillRect(r, &rect);

    SDL_SetRenderDrawColor(r, 20, 20, 20, 255);
    SDL_RenderRect(r, &rect);

    if (sw > 60 && font) {
        char label[320];
        if (sw > 120)
            snprintf(label, sizeof(label), "%s %s", node->name,
                     format_size(node->size));
        else
            snprintf(label, sizeof(label), "%s", node->name);

        SDL_Surface *surf = TTF_RenderText_Blended(font, label, 0,
                            (SDL_Color){20, 20, 20, 255});
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
            float tw = (float)surf->w, th = (float)surf->h;
            if (tw > sw - LABEL_PAD * 2) tw = sw - LABEL_PAD * 2;
            SDL_FRect dst = {sx + LABEL_PAD, sy + (sh - th) / 2, tw, th};
            SDL_RenderTexture(r, tex, NULL, &dst);
            SDL_DestroyTexture(tex);
            SDL_DestroySurface(surf);
        }
    }

    float cx = x;
    for (uint32_t i = 0; i < node->child_count; i++) {
        DirNode *child = &node->children[i];
        float cw = (node->size > 0)
            ? w * ((float)child->size / (float)node->size)
            : 0;
        if (cw < 0.5f) continue;

        draw_node_row(r, font, child, cam, cx,
                      y + ROW_HEIGHT + ROW_GAP, cw,
                      depth + 1, window_w, window_h);
        cx += cw;
    }
}

void renderer_draw(SDL_Renderer *r, TTF_Font *font, DirNode *root,
                   Camera *cam, int window_w, int window_h)
{
    if (!root || root->child_count == 0) return;

    float total_w = (float)window_w;

    float x = 0;
    for (uint32_t i = 0; i < root->child_count; i++) {
        DirNode *child = &root->children[i];
        float w = (root->size > 0)
            ? total_w * ((float)child->size / (float)root->size)
            : 0;
        if (w < 0.5f) continue;

        draw_node_row(r, font, child, cam, x, 0, w,
                      0, window_w, window_h);
        x += w;
    }
}

void render_welcome(SDL_Renderer *r, TTF_Font *font, int w, int h)
{
    if (!font) return;
    const char *text = "Press O to select a folder";
    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0,
                        (SDL_Color){180, 180, 180, 255});
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = {
        (w - surf->w) / 2.0f,
        (h - surf->h) / 2.0f,
        (float)surf->w, (float)surf->h
    };
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_DestroySurface(surf);
}

void render_scan_indicator(SDL_Renderer *r, TTF_Font *font,
                           uint32_t files, uint64_t size, int w, int h)
{
    if (!font) return;
    (void)w;
    char text[128];
    snprintf(text, sizeof(text), "Scanning... %u files  %s",
             files, format_size(size));

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0,
                        (SDL_Color){180, 180, 180, 200});
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FRect dst = {8, h - surf->h - 8.0f, (float)surf->w, (float)surf->h};
    SDL_RenderTexture(r, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_DestroySurface(surf);
}
