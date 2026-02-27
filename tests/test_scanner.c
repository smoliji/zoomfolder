#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <SDL3/SDL.h>
#include "scanner.h"

static void make_test_dir(void)
{
    mkdir("/tmp/zf_test", 0755);
    mkdir("/tmp/zf_test/a", 0755);
    mkdir("/tmp/zf_test/b", 0755);
    FILE *f = fopen("/tmp/zf_test/a/file1.txt", "w");
    if (f) { fprintf(f, "%*s", 1000, ""); fclose(f); }
    f = fopen("/tmp/zf_test/b/file2.txt", "w");
    if (f) { fprintf(f, "%*s", 2000, ""); fclose(f); }
}

static void cleanup_test_dir(void)
{
    unlink("/tmp/zf_test/a/file1.txt");
    unlink("/tmp/zf_test/b/file2.txt");
    rmdir("/tmp/zf_test/a");
    rmdir("/tmp/zf_test/b");
    rmdir("/tmp/zf_test");
}

void test_scan_basic(void)
{
    make_test_dir();

    ScanContext *ctx = scanner_start("/tmp/zf_test");
    assert(ctx != NULL);

    while (!ctx->done)
        SDL_Delay(10);

    SDL_LockMutex(ctx->mutex);
    assert(ctx->root != NULL);
    assert(ctx->root->child_count == 2);
    assert(ctx->root->size >= 3000);
    assert(ctx->total_files >= 2);
    SDL_UnlockMutex(ctx->mutex);

    scanner_free(ctx);
    cleanup_test_dir();
}

void test_scan_empty(void)
{
    mkdir("/tmp/zf_test_empty", 0755);

    ScanContext *ctx = scanner_start("/tmp/zf_test_empty");
    while (!ctx->done)
        SDL_Delay(10);

    SDL_LockMutex(ctx->mutex);
    assert(ctx->root->child_count == 0);
    assert(ctx->root->size == 0);
    SDL_UnlockMutex(ctx->mutex);

    scanner_free(ctx);
    rmdir("/tmp/zf_test_empty");
}

int main(void)
{
    SDL_Init(0);
    test_scan_basic();
    test_scan_empty();
    printf("All scanner tests passed.\n");
    SDL_Quit();
    return 0;
}
