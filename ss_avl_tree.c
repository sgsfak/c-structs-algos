#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "ss_avl_tree.h"

void ss_avl_init(ss_avl_tree* avl, int (*compar)(ss_avl_node* a, ss_avl_node* b))
{
    assert(avl != NULL);
    avl->root = NULL;
    avl->compar = compar;
    avl->n = 0;
}

ss_avl_node* ss_avl_find(ss_avl_tree* avl, ss_avl_node* val)
{
    assert(avl != NULL);
    assert(val != NULL);
    ss_avl_node* h = avl->root;
    while (h != NULL) {
        int c = avl->compar(val, h);
        if (c == 0)
            return h;
        h = c < 0 ? h->left : h->right;
    }
    return NULL;
}

#define NODE_HEIGHT(node) ((node) != NULL ? (node)->height : 0)
#define NODE_LEFT_HEIGHT(node) NODE_HEIGHT((node)->left)
#define NODE_RIGHT_HEIGHT(node) NODE_HEIGHT((node)->right)
#define BALANCE_FACTOR(node) (NODE_LEFT_HEIGHT(node) - NODE_RIGHT_HEIGHT(node))

#define MAX_CHILD_HEIGHT(node) (NODE_LEFT_HEIGHT(node) > NODE_RIGHT_HEIGHT(node) ? NODE_LEFT_HEIGHT(node) : NODE_RIGHT_HEIGHT(node))
#define UPDATE_NODE_HEIGHT(node) ((node)->height = 1 + MAX_CHILD_HEIGHT(node))

#define IMBALANCE_LEFT(node) (BALANCE_FACTOR(node) > 1)
#define IMBALANCE_RIGHT(node) (BALANCE_FACTOR(node) < -1)

static
ss_avl_node* balance(ss_avl_node* node)
{
    int b = BALANCE_FACTOR(node);
    if (b > 1) {
        /* Imbalance left */
        ss_avl_node* left = node->left;
        if (NODE_LEFT_HEIGHT(left) > NODE_RIGHT_HEIGHT(left)) {
            /* Right rotation */
            node->left = left->right;
            left->right = node;
            UPDATE_NODE_HEIGHT(node);
            UPDATE_NODE_HEIGHT(left);
            return left;
        }   
        else {
            ss_avl_node* r = left->right;
            /* Left-Right rotation */
            left->right = r->left;
            UPDATE_NODE_HEIGHT(left);
            node->left = r->right;
            UPDATE_NODE_HEIGHT(node);
            r->left = left;
            r->right = node;
            UPDATE_NODE_HEIGHT(r);
            return r;
        }
    }
    else if (b < -1) {
        /* Imbalance right */
        ss_avl_node* right = node->right;
        if (NODE_RIGHT_HEIGHT(right) > NODE_LEFT_HEIGHT(right)) {
            // Left rotation
            node->right = right->left;
            right->left = node;
            UPDATE_NODE_HEIGHT(node);
            UPDATE_NODE_HEIGHT(right);
            return right;
        }
        else {
            // Right-Left rotation
            ss_avl_node* r = right->left;

            right->left = r->right;
            UPDATE_NODE_HEIGHT(right);
            node->right = r->left;
            UPDATE_NODE_HEIGHT(node);
            r->left = node;
            r->right = right;
            UPDATE_NODE_HEIGHT(r);
            return r;
        }
    }
    UPDATE_NODE_HEIGHT(node); 
    return node;
}

static
ss_avl_node* ss_avl_insert_rec(ss_avl_tree* avl, ss_avl_node* node, ss_avl_node* val)
{
    if (node == NULL) {
        node = val;
        node->height = 1;
        node->left = node->right = NULL;
        avl->n++;
        return node;
    }
    int c = avl->compar(val, node);
    if (c == 0)
        return node;
    if (c < 0) {
        /* Insert into the left sub-tree */
        node->left = ss_avl_insert_rec(avl, node->left, val);
    }
    else {
        /* Insert into the right sub-tree */
        node->right = ss_avl_insert_rec(avl, node->right, val);
    }

    node = balance(node);
    return node;
}

void ss_avl_insert(ss_avl_tree* avl, ss_avl_node* val)
{
    assert(avl != NULL);
    avl->root = ss_avl_insert_rec(avl, avl->root, val);
}


static
ss_avl_node* extract_leftmost(ss_avl_node* root, ss_avl_node**node)
{
    if (root->left == NULL) {
        *node = root;
        return root->right;
    }
    root->left = extract_leftmost(root->left, node);
    root = balance(root);
    return root;
}


static
ss_avl_node* ss_avl_join(ss_avl_node* root, ss_avl_node* left, ss_avl_node* right)
{
    int b = NODE_HEIGHT(left) - NODE_HEIGHT(right);

    if (-1 <= b && b <= 1) {
        root->left = left;
        root->right = right;
        UPDATE_NODE_HEIGHT(root);
        return root;
    }
    if (b > 1) {
        left->right = ss_avl_join(root, left->right, right);
        root = left;
    }
    else {
        right->left = ss_avl_join(root, left, right->left);
        root = right;
    }
    root = balance(root);
    return root;

}

static 
ss_avl_node* ss_avl_delete_node(ss_avl_tree* avl, ss_avl_node* node, ss_avl_node* val, ss_avl_node** found)
{
    if (node == NULL)
        return NULL;
    int c = avl->compar(val, node);
    if (c < 0)
        node->left = ss_avl_delete_node(avl, node->left, val, found);
    else if (c > 0)
        node->right = ss_avl_delete_node(avl, node->right, val, found);
    else {
        *found = node;
        if (node->right == NULL) {
            node = node->left;
        }
        else if (node->left == NULL) {
            node = node->right;
        }
        else {
            /* Find the in-order successor, i.e. the leftmost of the 
            right subtree, extract it, and then merge the left and
            right subtrees putting the successor as root */
            ss_avl_node* succ;
            node->right = extract_leftmost(node->right, &succ);
            node = ss_avl_join(succ, node->left, node->right);
        }
    }
    return balance(node);
}

ss_avl_node* ss_avl_delete(ss_avl_tree* avl, ss_avl_node* val)
{
    assert(avl);
    ss_avl_node* node = NULL;
    avl->root = ss_avl_delete_node(avl, avl->root, val, &node);
    if (node != NULL)
        avl->n--;
    return node;

}

static
void ss_avl_node_to_dot(ss_avl_node* p, char* (*tostr)(const ss_avl_node*))
{

    char label[200];
    snprintf(label, 200, "<left> | <name> %u (%s) | <right>", p->height, tostr ? tostr(p) : "");
    printf("n%p [label=\"%s\"];\n", p, label);

    if (p->left) {
        printf("\"n%p\":left -> \"n%p\":name;\n", p, p->left);
        ss_avl_node_to_dot(p->left, tostr);
    }
    if (p->right) {
        printf("\"n%p\":right -> \"n%p\":name;\n", p, p->right);
        ss_avl_node_to_dot(p->right, tostr);
    }

}

void ss_avl_to_dot(ss_avl_tree* t, char* (*tostr)(const ss_avl_node*))
{

    printf("digraph graphname {\n"
            "node [shape = record];\n");
    if (t->root)
        ss_avl_node_to_dot(t->root, tostr);

    printf("}\n");
}
