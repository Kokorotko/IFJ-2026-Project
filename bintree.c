//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#include <stdlib.h>
#include <stdbool.h>
#include "bintree.h"
#include "linkedList.h"

Tree_Node *CreateNode(TreeItem item)
{
    Tree_Node *temp = (Tree_Node *)malloc(sizeof(Tree_Node));
    temp->data = item;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

void DestroyTree(Tree_Node *root)
{
    if(root != NULL)
    {
        DestroyTree(root->left);
        DestroyTree(root->right);
        free(root);
    }
}

void InOrder(Head *list, Tree_Node *root)
{
    bool check; // pro kontrolu internal erroru z insertu
    InOrder(list, root->left);
    check = Insert(list, root->data);
    // nějaký error-handling
    InOrder(list, root->right);
}

void Height(Tree_Node *ptr, int *max)
{
    int height_l;
    int height_r;
    if(ptr != NULL)
    {
        Height(ptr->left, &height_l);
        Height(ptr->right, &height_r);
        if(height_l > height_r)
        {
            // kvůli kořenu + 1
            *max = height_l + 1;
        }else
        {
            // kvůli kořenu + 1
            *max = height_r + 1;
        }
    }
    else
    {
        *max = 0;
    }
}

bool Height_Balanced(Tree_Node *root)
{
    if(root == NULL)
    {
        return true;
    }

    int height_l;
    int height_r;
    Height(root->left, &height_l);
    Height(root->right, &height_r);

    if(abs(height_l - height_r) > 1)
    {
        return false;
    }

    return(Height_Balanced(root->left) && Height_Balanced(root->right));
}

Tree_Node *InsertNode(Tree_Node *node, TreeItem data, char *newnode_key)
{
    if(node == NULL)
    {
        return CreateNode(data);
    }
    
    if(strcmp(newnode_key, node->key) < 0)
    {
        node->left = InsertNode(node->left, data, newnode_key);
    }
    else if(strcmp(newnode_key,node->key ) > 0)
    {
        node->right = InsertNode(node->right, data, newnode_key);
    }
    else
    {
        node->data = data;
    }

    return node;
}