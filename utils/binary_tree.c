#include "binary_tree.h"

node_t* create_tree(){
    node_t* head = 0;
    return head;
}

void destroy_tree(node_t* node){
    if (node != NULL){
        free(node->name);
        destroy_tree(node->left);
        destroy_tree(node->right);
        free(node);
    }
}

node_t* tree_insert(node_t** node, char* name, unsigned addr){
    if (*node == NULL){
        *node = (node_t*)malloc(sizeof(node_t));
        memset(*node, 0, sizeof(node_t));
        (*node)->name = name;
        (*node)->addr = addr;
        return *node;
    }
    
    int cmp = strcmp(name, (*node)->name);
    if (cmp == 0)
        return NULL;
    else if (cmp < 0)
        return tree_insert(&((*node)->left), name, addr);
    else
        return tree_insert(&((*node)->right), name, addr);
}

node_t* tree_search(node_t* head, char* name){
    if (head == NULL || name == NULL){
        return NULL;
    }
    
    node_t* current = head;
    while (current != NULL){
        int cmp = strcmp(name, current->name);
        if (cmp == 0)
            return current;
        
        current = cmp < 0? current->left:current->right;
    }
    return NULL;
}

void print2DUtil(node_t *root, int space) { 
    // Base case 
    if (root == NULL) 
        return; 
  
    // Increase distance between levels 
    space += 10; 
  
    // Process right child first 
    print2DUtil(root->right, space); 
  
    // Print current node after space 
    // count 
    printf("\n"); 
    for (int i = 10; i < space; i++) 
        printf(" "); 
    printf("%s\n", root->name); 
  
    // Process left child 
    print2DUtil(root->left, space); 
} 
  