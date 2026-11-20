// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#include "bintree.h"
#include "linkedList.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Tree_Node *CreateNode(TreeItem item, char *key) {
  Tree_Node *temp = (Tree_Node *)malloc(sizeof(Tree_Node));
  if (temp == NULL) {
    return NULL;
  }
  temp->data = item;
  temp->key = key;
  temp->left = NULL;
  temp->right = NULL;
  temp->height = 1;
  return temp;
}

void DestroyTree(Tree_Node *root) {
  if (root != NULL) {
    DestroyTree(root->left);
    DestroyTree(root->right);
    free(root);
  }
}

bool InOrder(Head *list, Tree_Node *root) {
  // bool check; // pro kontrolu internal erroru z insertu
  if (root == NULL) {
    return true;
  }
  if (InOrder(list, root->left) == false) {
    return false;
  }
  if (Insert(list, root->data) == false) {
    return false;
  }
  return InOrder(list, root->right);
}

int NodeHeight(Tree_Node *node) {
  if (node == NULL) {
    return 0;
  } else {
    return node->height;
  }
}

void HeightUpdate(Tree_Node *node) {
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

int HeightBalance_Weight(Tree_Node *node) {
  if (node == NULL) {
    return 0;
  } else {
    // pokud bude těžší levý potomek, výsledek bude záporný (-> -1/1 jak v
    // prezentacích IAL)
    return NodeHeight(node->right) - NodeHeight(node->left);
  }
}

Tree_Node *Rotate_Right(Tree_Node *node_y) {
  Tree_Node *node_x = node_y->left;
  Tree_Node *tree_t2 = node_x->right;

  node_x->right = node_y;
  node_y->left = tree_t2;

  HeightUpdate(node_y);
  HeightUpdate(node_x);

  return node_x;
}

Tree_Node *Rotate_Left(Tree_Node *node_x) {
  Tree_Node *node_y = node_x->right;
  Tree_Node *tree_t2 = node_y->left;

  node_y->left = node_x;
  node_x->right = tree_t2;

  HeightUpdate(node_x);
  HeightUpdate(node_y);

  return node_y;
}

Tree_Node *InsertNode(Tree_Node *node, TreeItem data, char *newnode_key) {
  if (node == NULL) {
    return CreateNode(data, newnode_key);
  }

  if (strcmp(newnode_key, node->key) < 0) {
    node->left = InsertNode(node->left, data, newnode_key);
  } else if (strcmp(newnode_key, node->key) > 0) {
    node->right = InsertNode(node->right, data, newnode_key);
  } else {
    node->data = data;
    return node;
  }

  HeightUpdate(node);
  int balance = HeightBalance_Weight(node);

  // LL-case
  if (balance < -1 && (strcmp(newnode_key, node->left->key) < 0)) {
    return Rotate_Right(node);
  }

  // RR-case
  if (balance > 1 && (strcmp(newnode_key, node->right->key) > 0)) {
    return Rotate_Left(node);
  }

  // LR-case
  if (balance < -1 && (strcmp(newnode_key, node->left->key) > 0)) {
    node->left = Rotate_Left(node->left);
    return Rotate_Right(node);
  }

  // RL-case
  if (balance > 1 && (strcmp(newnode_key, node->right->key) < 0)) {
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