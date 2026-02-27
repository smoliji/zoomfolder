#include "scanner.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void scan_dir(ScanContext *ctx, DirNode *node, const char *path)
{
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (ctx->cancel) break;
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        char fullpath[MAX_PATH];
        snprintf(fullpath, sizeof(fullpath), "%s\\%s", path, fd.cFileName);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                continue;

            SDL_LockMutex(ctx->mutex);
            DirNode *child = tree_add_child(node, fd.cFileName);
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
        } else {
            uint64_t fsize = ((uint64_t)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;

            SDL_LockMutex(ctx->mutex);
            node->size += fsize;
            node->file_count++;
            ctx->total_size += fsize;
            ctx->total_files++;
            SDL_UnlockMutex(ctx->mutex);
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}

static int scanner_thread_fn(void *data)
{
    ScanContext *ctx = data;
    char path[MAX_PATH];
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
