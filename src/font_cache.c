#include "font_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CacheEntry {
    char        *key;
    SDL_Texture *texture;
    int          w, h;
    uint32_t     last_used;
} CacheEntry;

struct FontCache {
    CacheEntry *entries;
    int         capacity;
    int         count;
    uint32_t    tick;
};

FontCache *font_cache_create(int capacity)
{
    FontCache *c = calloc(1, sizeof(FontCache));
    if (!c) return NULL;
    c->entries = calloc(capacity, sizeof(CacheEntry));
    c->capacity = capacity;
    return c;
}

static uint32_t hash_str(const char *s)
{
    uint32_t h = 5381;
    while (*s)
        h = ((h << 5) + h) ^ (uint8_t)*s++;
    return h;
}

static int find_entry(FontCache *c, const char *key)
{
    for (int i = 0; i < c->count; i++) {
        if (strcmp(c->entries[i].key, key) == 0)
            return i;
    }
    return -1;
}

static void evict_oldest(FontCache *c)
{
    int oldest = 0;
    for (int i = 1; i < c->count; i++) {
        if (c->entries[i].last_used < c->entries[oldest].last_used)
            oldest = i;
    }
    SDL_DestroyTexture(c->entries[oldest].texture);
    free(c->entries[oldest].key);
    c->entries[oldest] = c->entries[--c->count];
}

SDL_Texture *font_cache_get(FontCache *cache, SDL_Renderer *r,
                            TTF_Font *font, const char *text,
                            SDL_Color color, int *w, int *h)
{
    if (!cache || !text || !*text) return NULL;

    cache->tick++;

    char key[384];
    snprintf(key, sizeof(key), "%s\x01%u%u%u", text, color.r, color.g, color.b);

    int idx = find_entry(cache, key);
    if (idx >= 0) {
        cache->entries[idx].last_used = cache->tick;
        if (w) *w = cache->entries[idx].w;
        if (h) *h = cache->entries[idx].h;
        return cache->entries[idx].texture;
    }

    SDL_Surface *surf = TTF_RenderText_Blended(font, text, 0, color);
    if (!surf) return NULL;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(r, surf);
    int tw = surf->w, th = surf->h;
    SDL_DestroySurface(surf);
    if (!tex) return NULL;

    if (cache->count >= cache->capacity)
        evict_oldest(cache);

    CacheEntry *e = &cache->entries[cache->count++];
    e->key = strdup(key);
    e->texture = tex;
    e->w = tw;
    e->h = th;
    e->last_used = cache->tick;

    if (w) *w = tw;
    if (h) *h = th;
    return tex;
}

void font_cache_clear(FontCache *cache)
{
    if (!cache) return;
    for (int i = 0; i < cache->count; i++) {
        SDL_DestroyTexture(cache->entries[i].texture);
        free(cache->entries[i].key);
    }
    cache->count = 0;
    cache->tick = 0;
}

void font_cache_free(FontCache *cache)
{
    if (!cache) return;
    font_cache_clear(cache);
    free(cache->entries);
    free(cache);
}
