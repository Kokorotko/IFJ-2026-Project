//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef STACK_H //Nedefinuj soubor znova, pokud uz je definovan
#define STACK_H

//include
#include <stdbool.h>
#include "structure.h" //Pro přístup pro Token

//Typ položky v zásobníku - nastaveno na Token z LEX analýzy
typedef Token StackItem;

typedef struct stack_node
{
    StackItem data;
    struct stack_node *next;
}StackNode;

typedef struct
{
    StackNode *top;
}Stack;
//Incializace zasobníku (musí být zavoláno před prvním použitím)
void StackInit(Stack *s);
//Vložení prvku na vrchol zásobníku, vrací false při chybě alokace paměti
bool StackPush(Stack *s, StackItem item);
//Odstranění prvku z vrcholu zásobníku, vrací false když je zásobník prázdný. Vrací vrchní item
bool StackPopReturn(Stack *s, StackItem *item);
//Odstranění prvku z vrcholu zásobníku, vrací false když je zásobník prázdný
bool StackPop(Stack *s);
//Získání hodnoty z vrcholu zásobníku bez odstranění hodnoty
bool StackTop(Stack *s, StackItem *item);
//Kontrola prázdnoty zásobníku
bool StackIsEmpty(Stack *s);
//Uvolnění zásobníku z paměti
void StackFree(Stack *s);
//Zkontroluje jestli na vrcholu zasobniku neni operand
bool IsStackTopOperand(Stack *s);

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
    StackItem *tempItem;
    StackTop(s, tempItem);
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
    item = &temp->data;
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

bool StackTop(Stack *s, StackItem *item)
{
    if (s == NULL || s->top == NULL || item == NULL)
    {
        return false;
    }
    *item = s->top->data;
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

#endif