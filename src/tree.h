#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct DirNode {
    char            name[256];
    uint64_t        size;
    uint32_t        file_count;
    struct DirNode *children;
    uint32_t        child_count;
    uint32_t        child_capacity;
    bool            complete;
} DirNode;

DirNode *tree_create(const char *name);
DirNode *tree_add_child(DirNode *parent, const char *name);
void     tree_propagate_size(DirNode *node, uint64_t added);
void     tree_sort_children(DirNode *node);
void     tree_free(DirNode *node);
