#include "scanner.h"
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void scan_dir(ScanContext *ctx, DirNode *node, const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (ctx->cancel) break;
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        struct stat st;
        if (lstat(fullpath, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            SDL_LockMutex(ctx->mutex);
            DirNode *child = tree_add_child(node, entry->d_name);
            SDL_UnlockMutex(ctx->mutex);

            if (child)
                scan_dir(ctx, child, fullpath);

            SDL_LockMutex(ctx->mutex);
            if (child) {
                node->size += child->size;
                node->file_count += child->file_count;
                tree_sort_children(child);
                child->complete = true;
            }
            SDL_UnlockMutex(ctx->mutex);
        } else if (S_ISREG(st.st_mode)) {
            SDL_LockMutex(ctx->mutex);
            node->size += st.st_size;
            node->file_count++;
            ctx->total_size += st.st_size;
            ctx->total_files++;
            SDL_UnlockMutex(ctx->mutex);
        }
    }

    closedir(dir);
}

static int scanner_thread_fn(void *data)
{
    ScanContext *ctx = data;
    char path[4096];
    strncpy(path, ctx->root->name, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    scan_dir(ctx, ctx->root, path);

    SDL_LockMutex(ctx->mutex);
    ctx->root->complete = true;
    ctx->done = true;
    ctx->total_size = ctx->root->size;
    SDL_UnlockMutex(ctx->mutex);

    return 0;
}

ScanContext *scanner_start(const char *path)
{
    ScanContext *ctx = calloc(1, sizeof(ScanContext));
    if (!ctx) return NULL;

    ctx->mutex = SDL_CreateMutex();
    ctx->root = tree_create(path);

    ctx->thread = SDL_CreateThread(scanner_thread_fn, "scanner", ctx);
    return ctx;
}

void scanner_cancel(ScanContext *ctx)
{
    if (!ctx) return;
    ctx->cancel = true;
    SDL_WaitThread(ctx->thread, NULL);
}

void scanner_free(ScanContext *ctx)
{
    if (!ctx) return;
    if (!ctx->done) scanner_cancel(ctx);
    else SDL_WaitThread(ctx->thread, NULL);
    tree_free(ctx->root);
    SDL_DestroyMutex(ctx->mutex);
    free(ctx);
}
