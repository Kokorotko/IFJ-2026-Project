// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef BINTREE_H
#define BINTREE_H

#include "linkedList.h"
#include "structure.h"

typedef Token TreeItem;

typedef struct tree_node {
  TreeItem data;
  char *key; // how do we get this? don't know
  struct tree_node *left;
  struct tree_node *right;
  int height;
} Tree_Node;

// Vytvoření nové položky s NULL potomky
Tree_Node *CreateNode(TreeItem item, char *key);
// Rušení stromu od uzlu root (free)
void DestroyTree(Tree_Node *root);
// Průchod in-order stromem, zapsán do list, vrací false pokud došlo k chybě
// (např. NULL z malloc), jinak true
bool InOrder(Head *list, Tree_Node *root);
// Vrácení výšky daného uzlu
int NodeHeight(Tree_Node *node);
// Přepočítání výšky daného uzlu (po rotacích, insertu, delete, atd.)
void HeightUpdate(Tree_Node *node);
// Výpočet balanční váhy (balančního faktoru) daného uzlu (0, -1 a 1 jsou
// výškově vyvážené)
int HeightBalance_Weight(Tree_Node *node);
// Pravá rotace
Tree_Node *Rotate_Right(Tree_Node *node_y);
// Levá rotace
Tree_Node *Rotate_Left(Tree_Node *node_x);
// Vložení nového uzlu do stromu s daným klíčem, obsahuje kontrolu pro případy
// LL, RR, LR, RL a jejich opravu
Tree_Node *InsertNode(Tree_Node *node, TreeItem data, char *newnode_key);
// Vrací ukazatel na hledaný uzel dle klíče, vrátí NULL pokud nebude nalezen
Tree_Node *Search(Tree_Node *node, char *targetnode_key);

#endif