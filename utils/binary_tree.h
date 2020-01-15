#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


typedef struct __node__{
    char* name;
    unsigned addr;
    struct __node__* left, *right;
} node_t;

node_t* create_tree();
void destroy_tree(node_t* head);
node_t* tree_insert(node_t** head, char* name, unsigned addr);
node_t* tree_search(node_t* head, char* name);
void print2DUtil(node_t *root, int space);