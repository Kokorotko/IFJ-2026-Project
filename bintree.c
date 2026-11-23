// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#include "bintree.h"
#include <stdlib.h>
#include <string.h>

static int NodeHeight(Tree_Node *node) {
  if (node == NULL) {
    return 0;
  } else {
    return node->height;
  }
}

static void HeightUpdate(Tree_Node *node) {
  int height_l = NodeHeight(node->left);
  int height_r = NodeHeight(node->right);
  if (height_l > height_r) {
    // + 1 kvůli kořenu
    node->height = height_l + 1;
  } else {
    // + 1 kvůli kořenu
    node->height = height_r + 1;
  }
}

static int HeightBalance_Weight(Tree_Node *node) {
  if (node == NULL) {
    return 0;
  } else {
    // pokud bude těžší levý potomek, výsledek bude záporný (-> -1/1 jak v
    // prezentacích IAL)
    return NodeHeight(node->right) - NodeHeight(node->left);
  }
}

static Tree_Node *Rotate_Right(Tree_Node *node_y) {
  Tree_Node *node_x = node_y->left;
  Tree_Node *tree_t2 = node_x->right;

  node_x->right = node_y;
  node_y->left = tree_t2;

  HeightUpdate(node_y);
  HeightUpdate(node_x);

  return node_x;
}

static Tree_Node *Rotate_Left(Tree_Node *node_x) {
  Tree_Node *node_y = node_x->right;
  Tree_Node *tree_t2 = node_y->left;

  node_y->left = node_x;
  node_x->right = tree_t2;

  HeightUpdate(node_x);
  HeightUpdate(node_y);

  return node_y;
}

static char *DuplicateKey(const char *key) {
  size_t length = strlen(key) + 1;
  char *copy = malloc(length);
  if (copy == NULL) {
    return NULL;
  }
  memcpy(copy, key, length);
  return copy;
}

static Tree_Node *CreateNode(const char *key, void *data) {
  Tree_Node *node = malloc(sizeof(Tree_Node));
  if (node == NULL) {
    return NULL;
  }

  node->key = DuplicateKey(key);
  if (node->key == NULL) {
    free(node);
    return NULL;
  }

  node->data = data;
  node->left = NULL;
  node->right = NULL;
  node->height = 1;
  return node;
}

void DestroyTree(Tree_Node *root, FreeDataFnc free_data) {
  if (root == NULL) {
    return;
  }
  DestroyTree(root->left, free_data);
  DestroyTree(root->right, free_data);

  if (free_data != NULL && root->data != NULL) {
    free_data(root->data);
  }
  free(root->key);
  free(root);
}

Tree_Node *InsertNode(Tree_Node *node, void *data, const char *key,
                      TreeStatus *status) {
  if (node == NULL) {
    Tree_Node *temp_node = CreateNode(key, data);
    if (temp_node == NULL) {
      *status = Tree_MEM_ERR;
    } else {
      *status = Tree_OK;
    }
    return temp_node;
  }

  int key_compare = strcmp(key, node->key);
  if (key_compare < 0) {
    node->left = InsertNode(node->left, data, key, status);
  } else if (key_compare > 0) {
    node->right = InsertNode(node->right, data, key, status);
  }
  // key_compare == 0
  else {
    *status = Tree_DUPLICATE;
    return node;
  }

  if (*status != Tree_OK) {
    return node;
  }

  HeightUpdate(node);
  int balance = HeightBalance_Weight(node);

  // LL-case
  if (balance < -1 && HeightBalance_Weight(node->left) < 0) {
    return Rotate_Right(node);
  }

  // RR-case
  if (balance > 1 && HeightBalance_Weight(node->right) > 0) {
    return Rotate_Left(node);
  }

  // LR-case
  if (balance < -1 && HeightBalance_Weight(node->left) > 0) {
    node->left = Rotate_Left(node->left);
    return Rotate_Right(node);
  }

  // RL-case
  if (balance > 1 && HeightBalance_Weight(node->right) < 0) {
    node->right = Rotate_Right(node->right);
    return Rotate_Left(node);
  }

  return node;
}

Tree_Node *Search(Tree_Node *node, char *targetnode_key) {
  if (node == NULL) {
    return NULL;
  }

  if (strcmp(targetnode_key, node->key) == 0) {
    return node;
  } else if (strcmp(targetnode_key, node->key) < 0) {
    return (Search(node->left, targetnode_key));
  }

  // jinak jdeme doprava, i když vpravo od daného uzlu je NULL (nemá pravého
  // potomka), dostaneme NULL a tedy rekurze skončí a dostaneme výsledek že uzel
  // s klíčem neexistuje
  return (Search(node->right, targetnode_key));
}