#include "renderer.h"
#include <string.h>
#include <stdio.h>

static const SDL_Color PALETTE[] = {
    { 0, 188, 176, 255},  // Teal
    {255,  87,  80, 255},  // Coral
    { 30, 136, 229, 255},  // Electric blue
    {255, 179,   0, 255},  // Amber
    {126,  87, 194, 255},  // Violet
    {124, 179,  66, 255},  // Lime
    {236,  64, 122, 255},  // Hot pink
    {  0, 229, 255, 255},  // Cyan
    {255, 109,   0, 255},  // Orange
    { 16, 185, 129, 255},  // Emerald
    {213,   0, 143, 255},  // Magenta
    {255, 196,   0, 255},  // Gold
    { 63,  81, 181, 255},  // Indigo
    {192, 220,  42, 255},  // Chartreuse
    {244,  67, 131, 255},  // Rose
    { 56, 189, 248, 255},  // Sky
};
#define PALETTE_SIZE (sizeof(PALETTE) / sizeof(PALETTE[0]))

#define ROW_HEIGHT 28
#define ROW_GAP 2
#define LABEL_PAD 4

static const SDL_Color COLOR_LABEL = {20, 20, 20, 255};
static const SDL_Color COLOR_TEXT  = {180, 180, 180, 255};

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

static void draw_cached_text(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                             const char *text, SDL_Color color,
                             float x, float y, float max_w)
{
    int tw, th;
    SDL_Texture *tex = font_cache_get(cache, r, font, text, color, &tw, &th);
    if (!tex) return;
    float w = (float)tw;
    if (w > max_w) w = max_w;
    SDL_FRect dst = {x, y, w, (float)th};
    SDL_RenderTexture(r, tex, NULL, &dst);
}

static inline uint8_t clamp255(int v) { return v > 255 ? 255 : (uint8_t)v; }

void camera_update(Camera *cam, float dt)
{
    float t = 12.0f * dt;
    if (t > 1.0f) t = 1.0f;
    cam->zoom += (cam->target_zoom - cam->zoom) * t;
    cam->offset_x += (cam->target_offset_x - cam->offset_x) * t;
    cam->offset_y += (cam->target_offset_y - cam->offset_y) * t;
}

static void animate_node(DirNode *node, float t)
{
    node->display_size += ((float)node->size - node->display_size) * t;
    for (uint32_t i = 0; i < node->child_count; i++)
        animate_node(&node->children[i], t);
}

void renderer_animate(DirNode *root, float dt)
{
    if (!root) return;
    float t = 8.0f * dt;
    if (t > 1.0f) t = 1.0f;
    animate_node(root, t);
}

static void draw_node_row(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                          DirNode *node, Camera *cam, DirNode *hovered,
                          float x, float y, float w, int depth,
                          int window_w, int window_h)
{
    float sx = (x + cam->offset_x) * cam->zoom;
    float sy = (y + cam->offset_y) * cam->zoom;
    float sw = w * cam->zoom;
    float sh = ROW_HEIGHT * cam->zoom;

    if (sx + sw < 0 || sx > window_w || sy > window_h)
        return;
    if (sw < 1.0f) return;

    bool visible = !(sy + sh < 0);
    if (visible) {
        SDL_Color col = PALETTE[hash_name(node->name) % PALETTE_SIZE];
        bool is_hovered = (node == hovered);

        if (is_hovered) {
            col.r = clamp255(col.r + 30);
            col.g = clamp255(col.g + 30);
            col.b = clamp255(col.b + 30);
        }

        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, col.a);
        SDL_FRect rect = {sx, sy, sw, sh};
        SDL_RenderFillRect(r, &rect);

        if (is_hovered) {
            SDL_SetRenderDrawColor(r, 200, 200, 210, 255);
        } else {
            SDL_SetRenderDrawColor(r, col.r / 2, col.g / 2, col.b / 2, 255);
        }
        SDL_RenderRect(r, &rect);

        if (sw > 40 && font && cache) {
            char label[320];
            if (sw > 120)
                snprintf(label, sizeof(label), "%s %s", node->name,
                         format_size(node->size));
            else
                snprintf(label, sizeof(label), "%s", node->name);

            draw_cached_text(r, font, cache, label, COLOR_LABEL,
                             sx + LABEL_PAD, sy + (sh - 14) / 2,
                             sw - LABEL_PAD * 2);
        }
    }

    float cx = x;
    for (uint32_t i = 0; i < node->child_count; i++) {
        DirNode *child = &node->children[i];
        float cw = (node->display_size > 0)
            ? w * (child->display_size / node->display_size)
            : 0;
        if (cw < 0.5f) continue;

        draw_node_row(r, font, cache, child, cam, hovered, cx,
                      y + ROW_HEIGHT + ROW_GAP, cw,
                      depth + 1, window_w, window_h);
        cx += cw;
    }
}

void renderer_draw(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                   DirNode *root, Camera *cam, DirNode *hovered,
                   int window_w, int window_h)
{
    if (!root || root->child_count == 0) return;

    float total_w = (float)window_w;

    float x = 0;
    for (uint32_t i = 0; i < root->child_count; i++) {
        DirNode *child = &root->children[i];
        float w = (root->display_size > 0)
            ? total_w * (child->display_size / root->display_size)
            : 0;
        if (w < 0.5f) continue;

        draw_node_row(r, font, cache, child, cam, hovered, x, 0, w,
                      0, window_w, window_h);
        x += w;
    }
}

void render_background(SDL_Renderer *r, int w, int h)
{
    for (int y = 0; y < h; y++) {
        float t = (float)y / (float)(h > 1 ? h - 1 : 1);
        uint8_t cr = (uint8_t)(42 + (28 - 42) * t);
        uint8_t cg = (uint8_t)(42 + (28 - 42) * t);
        uint8_t cb = (uint8_t)(66 + (38 - 66) * t);
        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
        SDL_RenderLine(r, 0, (float)y, (float)w, (float)y);
    }

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

    int depth = 100;
    for (int i = 0; i < depth; i++) {
        float t = 1.0f - (float)i / (float)depth;
        uint8_t a = (uint8_t)(35.0f * t * t);
        if (a == 0) break;
        SDL_SetRenderDrawColor(r, 0, 0, 0, a);
        SDL_FRect top = {0, (float)i, (float)w, 1};
        SDL_FRect bot = {0, (float)(h - 1 - i), (float)w, 1};
        SDL_FRect lft = {(float)i, 0, 1, (float)h};
        SDL_FRect rgt = {(float)(w - 1 - i), 0, 1, (float)h};
        SDL_RenderFillRect(r, &top);
        SDL_RenderFillRect(r, &bot);
        SDL_RenderFillRect(r, &lft);
        SDL_RenderFillRect(r, &rgt);
    }

    int spacing = 24;
    SDL_SetRenderDrawColor(r, 255, 255, 255, 28);
    for (int gy = spacing; gy < h; gy += spacing)
        for (int gx = spacing; gx < w; gx += spacing)
            SDL_RenderPoint(r, (float)gx, (float)gy);

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
}

void render_welcome(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                    int w, int h)
{
    if (!font || !cache) return;
    int tw, th;
    SDL_Texture *tex = font_cache_get(cache, r, font,
                                      "Press O to select a folder",
                                      COLOR_TEXT, &tw, &th);
    if (!tex) return;
    SDL_FRect dst = {
        (w - tw) / 2.0f, (h - th) / 2.0f,
        (float)tw, (float)th
    };
    SDL_RenderTexture(r, tex, NULL, &dst);
}

void render_scan_indicator(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                           uint32_t files, uint64_t size, int w, int h)
{
    if (!font || !cache) return;
    (void)w;

    static const char *dots[] = {"   ", ".  ", ".. ", "..."};
    int phase = (int)(SDL_GetTicks() / 400) % 4;

    char text[128];
    snprintf(text, sizeof(text), "Scanning%s %u files  %s",
             dots[phase], files, format_size(size));

    int tw, th;
    SDL_Texture *tex = font_cache_get(cache, r, font, text, COLOR_TEXT,
                                      &tw, &th);
    if (!tex) return;
    SDL_FRect dst = {8, h - th - 8.0f, (float)tw, (float)th};
    SDL_RenderTexture(r, tex, NULL, &dst);
}

static DirNode *hit_test_row(DirNode *node, Camera *cam,
                             float x, float y, float w,
                             float mx, float my, int window_w)
{
    float sx = (x + cam->offset_x) * cam->zoom;
    float sy = (y + cam->offset_y) * cam->zoom;
    float sw = w * cam->zoom;
    float sh = ROW_HEIGHT * cam->zoom;

    if (mx < sx || mx > sx + sw || my < sy || my > sy + sh + 10000)
        return NULL;

    if (my >= sy && my < sy + sh)
        return node;

    float cx = x;
    for (uint32_t i = 0; i < node->child_count; i++) {
        DirNode *child = &node->children[i];
        float cw = (node->display_size > 0)
            ? w * (child->display_size / node->display_size)
            : 0;
        if (cw < 0.5f) continue;

        DirNode *hit = hit_test_row(child, cam, cx,
                                    y + ROW_HEIGHT + ROW_GAP, cw,
                                    mx, my, window_w);
        if (hit) return hit;
        cx += cw;
    }

    return NULL;
}

DirNode *renderer_hit_test(DirNode *root, Camera *cam,
                           int window_w, float mx, float my)
{
    if (!root || root->child_count == 0) return NULL;

    float total_w = (float)window_w;
    float x = 0;
    for (uint32_t i = 0; i < root->child_count; i++) {
        DirNode *child = &root->children[i];
        float w = (root->display_size > 0)
            ? total_w * (child->display_size / root->display_size)
            : 0;
        if (w < 0.5f) continue;

        DirNode *hit = hit_test_row(child, cam, x, 0, w,
                                    mx, my, window_w);
        if (hit) return hit;
        x += w;
    }
    return NULL;
}

void render_tooltip(SDL_Renderer *r, TTF_Font *font, FontCache *cache,
                    DirNode *node, float mx, float my,
                    int window_w, int window_h)
{
    if (!node || !font || !cache) return;

    char line1[320], line2[128];
    snprintf(line1, sizeof(line1), "%s", node->name);
    snprintf(line2, sizeof(line2), "%s  %u files",
             format_size(node->size), node->file_count);

    int tw1, th1, tw2, th2;
    SDL_Texture *tex1 = font_cache_get(cache, r, font, line1, COLOR_TEXT,
                                       &tw1, &th1);
    SDL_Texture *tex2 = font_cache_get(cache, r, font, line2, COLOR_TEXT,
                                       &tw2, &th2);

    int pad = 8;
    int box_w = (tw1 > tw2 ? tw1 : tw2) + pad * 2;
    int box_h = th1 + th2 + pad * 3;

    float tx = mx + 16;
    float ty = my + 16;
    if (tx + box_w > window_w) tx = mx - box_w - 4;
    if (ty + box_h > window_h) ty = my - box_h - 4;

    SDL_FRect outer = {tx - 1, ty - 1, (float)box_w + 2, (float)box_h + 2};
    SDL_SetRenderDrawColor(r, 60, 60, 70, 255);
    SDL_RenderFillRect(r, &outer);

    SDL_FRect bg = {tx, ty, (float)box_w, (float)box_h};
    SDL_SetRenderDrawColor(r, 32, 32, 38, 240);
    SDL_RenderFillRect(r, &bg);

    SDL_FRect inner = {tx + 1, ty + 1, (float)box_w - 2, (float)box_h - 2};
    SDL_SetRenderDrawColor(r, 44, 44, 52, 240);
    SDL_RenderFillRect(r, &inner);

    if (tex1) {
        SDL_FRect d1 = {tx + pad, ty + pad, (float)tw1, (float)th1};
        SDL_RenderTexture(r, tex1, NULL, &d1);
    }
    if (tex2) {
        SDL_FRect d2 = {tx + pad, ty + pad + th1 + pad, (float)tw2, (float)th2};
        SDL_RenderTexture(r, tex2, NULL, &d2);
    }
}
