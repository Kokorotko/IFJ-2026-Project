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

void InitList(Head *list)
{
    list->first = NULL;
}

bool Insert(Head *list, Token data)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL)
    {
        return false;
    }

    newNode->data = data;
    newNode->next = NULL;

    if (list->first == NULL)            /* prázdný seznam */
    {
        list->first = newNode;
        return true;
    }

    Node *current = list->first;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = newNode;
    return true;
}

void DeleteLast(Head *list)
{
    if (list->first == NULL)
    {
        return;
    }

    if (list->first->next == NULL)      /* jediný prvek */
    {
        free(list->first);
        list->first = NULL;             /* mění se přímo ve struktuře, venku to uvidí */
        return;
    }

    Node *current = list->first;
    while (current->next->next != NULL)
    {
        current = current->next;
    }

    free(current->next);
    current->next = NULL;
}

bool ReadLast(const Head *list, Token *out)
{
    if (list->first == NULL)
    {
        return false;
    }

    Node *current = list->first;
    while (current->next != NULL)
    {
        current = current->next;
    }
    *out = current->data;
    return true;
}

void FreeList(Head *list)
{
    Node *current = list->first;
    while (current != NULL)
    {
        Node *next = current->next;
        free(current);
        current = next;
    }
    list->first = NULL;
}

int ListLenght(Head *list)
{
    if (list == NULL)
    {
        return -1; //ptr is NULL
    }
    int lenght = 0;
    Node *currentNode = list->first;
    while (currentNode != NULL)
    {
        lenght++;
        currentNode = currentNode->next;
    }
    return lenght;
}

#endif