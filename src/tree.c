#include "tree.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 8

static void free_children(DirNode *node)
{
    for (uint32_t i = 0; i < node->child_count; i++)
        free_children(&node->children[i]);
    free(node->children);
    node->children = NULL;
}

DirNode *tree_create(const char *name)
{
    DirNode *node = calloc(1, sizeof(DirNode));
    if (!node) return NULL;
    strncpy(node->name, name, sizeof(node->name) - 1);
    return node;
}

DirNode *tree_add_child(DirNode *parent, const char *name)
{
    if (parent->child_count == parent->child_capacity) {
        uint32_t new_cap = parent->child_capacity == 0
            ? INITIAL_CAPACITY
            : parent->child_capacity * 2;
        DirNode *buf = realloc(parent->children, new_cap * sizeof(DirNode));
        if (!buf) return NULL;
        parent->children = buf;
        parent->child_capacity = new_cap;
    }

    DirNode *child = &parent->children[parent->child_count++];
    memset(child, 0, sizeof(DirNode));
    strncpy(child->name, name, sizeof(child->name) - 1);
    return child;
}

void tree_propagate_size(DirNode *node, uint64_t added)
{
    node->size += added;
}

static int cmp_size_desc(const void *a, const void *b)
{
    uint64_t sa = ((const DirNode *)a)->size;
    uint64_t sb = ((const DirNode *)b)->size;
    return (sb > sa) - (sb < sa);
}

void tree_sort_children(DirNode *node)
{
    if (node->child_count > 1)
        qsort(node->children, node->child_count, sizeof(DirNode), cmp_size_desc);
}

void tree_free(DirNode *node)
{
    if (!node) return;
    free_children(node);
    free(node);
}
