//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef SYNTAX //Nedefinuj soubor znova, pokud uz je definovan
#define SYNTAX

#define MAX_RHS 10 //max right hand 

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "lex.h"
#include "structure.h"
#include "linkedList.h"

typedef struct rule
{
    int leftSide;
    int length;
    int rightSide[MAX_RHS];
} SyntaxRule;

static const SyntaxRule rules[] =
{
    { T_IDENTIFICATOR , 3, { T_IDENTIFICATOR, T_PLUS, T_IDENTIFICATOR } },   // expr2 -> + term expr2
};

//static SyntaxRules rules;

bool NumberOfPossibleOptions(Head *tokenHead, int *possibleOptions)
{
    if (tokenHead == NULL || possibleOptions == NULL)
    {
        return false;
    }
    return true; //for now
    //ListSize(&tokenHead);
    //while ()
}

int SyntaxChecker(void)
{
    Head list; //One could do this by array, but this is more fun :)
    InitList(&list);

    Token token;
    do
    {
        token = TokenGiver();
        if (!Insert(&list, token))
        {
            FreeList(&list);
            return ERR_INTERNAL; //Malloc Fail
        }

        int *possibleOptions = NULL;
        if (!NumberOfPossibleOptions(&list, possibleOptions))
        {

        }
        if (*possibleOptions == 0)
        {
            //Found syntax error
            FreeList(&list);
            return ERR_SYNTAX;
        }
        if (*possibleOptions == 1)
        {
            //Found the used rule and we can start from the start
            FreeList(&list);
        }
    } while (token.type != T_EOF); //End of File

    return 0;
}

#endif