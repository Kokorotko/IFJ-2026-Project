#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "structure.h"

#ifndef LINKEDLIST //Nedefinuj soubor znova, pokud uz je definovan
#define LINKEDLIST

typedef struct Node
{
    Token data;
    struct Node *next;
} Node;

typedef struct Head
{
    Node *first;
} Head;

void InitList(Head *list);
bool Insert(Head *list, Token data);
void DeleteLast(Head *list);
bool ReadLast(const Head *list, Token *out);
void FreeList(Head *list);
int ListLenght(Head *list);

#endif