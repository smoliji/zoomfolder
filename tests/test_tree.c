#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "tree.h"

void test_create(void)
{
    DirNode *root = tree_create("root");
    assert(root != NULL);
    assert(strcmp(root->name, "root") == 0);
    assert(root->size == 0);
    assert(root->child_count == 0);
    assert(root->complete == false);
    tree_free(root);
}

void test_add_children(void)
{
    DirNode *root = tree_create("root");
    DirNode *a = tree_add_child(root, "a");
    DirNode *b = tree_add_child(root, "b");
    assert(root->child_count == 2);
    assert(strcmp(a->name, "a") == 0);
    assert(strcmp(b->name, "b") == 0);
    tree_free(root);
}

void test_propagate_size(void)
{
    DirNode *root = tree_create("root");
    DirNode *a = tree_add_child(root, "a");
    tree_propagate_size(a, 1000);
    assert(a->size == 1000);
    tree_free(root);
}

void test_sort_children(void)
{
    DirNode *root = tree_create("root");
    DirNode *small = tree_add_child(root, "small");
    DirNode *big = tree_add_child(root, "big");
    small->size = 100;
    big->size = 9000;
    tree_sort_children(root);
    assert(strcmp(root->children[0].name, "big") == 0);
    assert(strcmp(root->children[1].name, "small") == 0);
    tree_free(root);
}

void test_dynamic_growth(void)
{
    DirNode *root = tree_create("root");
    for (int i = 0; i < 100; i++) {
        char name[32];
        snprintf(name, sizeof(name), "child_%d", i);
        tree_add_child(root, name);
    }
    assert(root->child_count == 100);
    tree_free(root);
}

int main(void)
{
    test_create();
    test_add_children();
    test_propagate_size();
    test_sort_children();
    test_dynamic_growth();
    printf("All tree tests passed.\n");
    return 0;
}
