//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef STACK_H
#define STACK_H

#include "codeStruct.h"

typedef struct {
    //TokType type;
    int     line;
    Token   token;
    Expression *e; //pro stavění a uchování stromečku
} StackItem;

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
bool StackTop(Stack *s, StackItem **item);
//Kontrola prázdnoty zásobníku
bool StackIsEmpty(Stack *s);
//Uvolnění zásobníku z paměti
void StackFree(Stack *s);
//Zkontroluje jestli na vrcholu zasobniku neni operand
bool IsStackTopOperand(Stack *s);

#endif