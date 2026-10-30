//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef BINTREE_H
#define BINTREE_H

#include "structure.h"
#include "linkedlist.h"

typedef Token TreeItem;

typedef struct tree_node
{
    TreeItem data;
    char* key;  // how do we get this? don't know
    struct tree_node *left;
    struct tree_node *right;
} Tree_Node;

// Vytvoření nové položky s NULL potomky
Tree_Node *CreateNode(TreeItem item);
// Rušení stromu od uzlu root (free)
void DestroyTree(Tree_Node *root);
// Průchod in-order stromem, zapsán do list
void InOrder(Head *list, Tree_Node *root);
// Zjištění výšky stromu od kořene *ptr, výška vložena do *max
void Height(Tree_Node *ptr, int *max);
// Kontrola výškové vyváženosti celého stromu od daného kořene *root
bool Height_Balanced(Tree_Node *root);
// Průchod stromem podle klíče, při nalezení prázdného uzlu (==NULL), vytváří nový
Tree_Node *InsertNode(Tree_Node *node, TreeItem data, char *newnode_key);

#endif