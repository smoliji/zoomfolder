#pragma once
#include "tree.h"
#include <SDL3/SDL_mutex.h>

typedef struct {
    DirNode      *root;
    SDL_Mutex    *mutex;
    SDL_Thread   *thread;
    bool          cancel;
    bool          done;
    uint64_t      total_size;
    uint32_t      total_files;
} ScanContext;

ScanContext *scanner_start(const char *path);
void         scanner_cancel(ScanContext *ctx);
void         scanner_free(ScanContext *ctx);
