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
//Odstranění prvku z vrcholu zásobníku, vrací false když je zásobník prázdný
bool StackPop(Stack *s);
//Získání hodnoty z vrcholu zásobníku bez odstranění hodnoty
bool StackTop(Stack *s, StackItem *item);
//Kontrola prázdnoty zásobníku
bool StackIsEmpty(Stack *s);
//Uvolnění zásobníku z paměti
void StackFree(Stack *s);


#endif