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
#include "stack.h"

void Next();
void Peek();
void Expect(TokType type);
void ExpectDataType();
void ParseIf(bool checkIndendation, int indendationLevel);
void ParseFunction(bool checkIndendation, int indendationLevel);
void ParseWhile(bool checkIndendation, int indendationLevel);
void ParseVariable(bool checkIndendation, int indendationLevel);
void ParseExpressionOrCall(bool checkIndendation, int indendationLevel);
void ParseExpression();
void UpdateState(TokType token);
bool ReduceByRule(StackItem *items, int lenght, StackItem *out);
char PTableFinder(Stack *stack, TokType token_Input);
int PTableIndexFinder_3000(TokType tokenType, bool isIndexInput);
void ParseLine(bool checkIndendation, int indendationLevel);
void CheckIndendation(bool check, int indendationLevel);
bool SyntaxChecker();
void ParseReturn(bool checkIndendation, int indendationLevel);
bool IsBinaryOperator(TokType token);
bool IsTerm(TokType token);
void ParseCall();

#endif