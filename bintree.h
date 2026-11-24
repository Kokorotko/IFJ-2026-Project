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
  char *key;
  void *data;
  struct tree_node *left;
  struct tree_node *right;
  int height;
} Tree_Node;

// Ukazatel na funkci pro uvolnění alokované paměti pro data uzlu (funkce pro
// uvolnění paměti dat VarSym a FuncSym se budou lišit)
typedef void (*FreeDataFnc)(void *data);

typedef enum {
  Tree_OK,
  Tree_DUPLICATE, // Pokus o vložení klíče, který je již ve stromě, mělo by vést
                  // k ERR_REDEFINED
  Tree_MEM_ERR    // Chyba alokace paměti, mělo by vést k ERR_INTERNAL
} TreeStatus;

// Rekurzivně (přes Post-Order) uvolňuje alokovanou paměť stromu, uvolní paměť
// pro key a Tree_Node, ale pro data potřebuje ukazatel na funkci, která uvolní
// samotné data vložená do uzlu
void DestroyTree(Tree_Node *root, FreeDataFnc free_data);
// Vložení nového uzlu do stromu s daným klíčem, pokud dojde ke shodě, přepíše
// se stav TreeStatus na Tree_DUPLICATE a data se nepřepisují
Tree_Node *InsertNode(Tree_Node *root, void *data, const char *newnode_key,
                      TreeStatus *status);
// Vrací ukazatel na hledaný uzel dle klíče, vrátí NULL pokud nebude nalezen
Tree_Node *Search(Tree_Node *node, char *targetnode_key);

#endif