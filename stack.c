//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

//include
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "structure.h" //Pro přístup pro Token
#include "stack.h"

void StackInit(Stack *s)
{
    if (s != NULL)
    {
        s->top = NULL;
    }
}

bool StackPush(Stack *s, StackItem item)
{
    if (s == NULL)
    {
        return false;
    }

    StackNode *new_node = (StackNode *)malloc(sizeof(StackNode));
    if(new_node == NULL)
    {
        return false;
    }
    new_node->data = item;
    new_node->next = s->top;
    s->top = new_node;

    return true;
}

bool IsStackTopOperand(Stack *s)
{
    StackItem *tempItem = malloc(sizeof(StackItem));
    MallocChecker (tempItem);
    StackTop(s, &tempItem);
    return (tempItem->type == T_INT_LIT  ||
            tempItem->type == T_DOUBLE_LIT ||
            tempItem->type == T_STRING_LIT || 
            tempItem->type == T_ID ||
            tempItem->type == T_RPAREN);
}

bool StackPopReturn(Stack *s, StackItem *item)
{
    if (s == NULL || s->top == NULL)
    {
        return false;
    }

    StackNode *temp = s->top;
    *item = temp->data;
    s->top = s->top->next;
    free(temp);
    return true;
}
bool StackPop(Stack *s)
{
    if (s == NULL || s->top == NULL)
    {
        return false;
    }

    StackNode *temp = s->top;
    s->top = s->top->next;
    free(temp);
    return true;
}

bool StackTop(Stack *s, StackItem **item) //DOUBLE POINTER WARNING
{
    if (s == NULL || s->top == NULL)
    {
        return false;
    }
    *item = &s->top->data;
    return true;
}

bool StackIsEmpty(Stack *s)
{
    return (s == NULL || s->top == NULL);
}

void StackFree(Stack *s)
{
    if (s == NULL)
    {
        return;
    }

    //Dokud není zásobník prázdný, postupně odebírá prvky z vrcholu
    while (!StackIsEmpty(s))
    {
        StackPop(s);
    }
}