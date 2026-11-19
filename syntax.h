// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef SYNTAX // Nedefinuj soubor znova, pokud uz je definovan
#define SYNTAX

#define MAX_RHS 10 // max right hand

// include
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codeStruct.h"
#include "lex.h"
#include "linkedList.h"
#include "stack.h"
#include "structure.h"

void Next();
void Peek();
void Expect(TokType type);
char *ExpectName();
void ExpectEndOfLine();
DataType ExpectDataType();
Statement *ParseIf();
Expression *ParseCall();
Statement *ParseWhile();
Statement *ParseVariable();
Statement *ParseAssign();
Expression *ParseExpressionOrCall();
FuncDef *ParseDefFunction();
Expression *ParseExpression();
void UpdateState(TokType token);
Expression *ExpressionTransform(StackItem *item);
bool ReduceByRule(StackItem *items, int lenght, StackItem *out);
bool IsBinaryOperator(TokType token);
bool IsTerm(TokType token);
char PTableFinder(Stack *stack, TokType tokenInput);
int PTableIndexFinder_3000(TokType tokenType, bool isIndexInput);
Statement *ParseLine();
Block *ParseBlock(int parentIndendation);
Statement *ParseReturn();
int NumberOfIntendations();
void SyntaxChecker(FuncDef **program);
Expression *NewExpression(ExpressionEnum type);
Statement *NewStatement(StatementEnum type);
FuncDef *NewFunction();
Block *NewBlock();
Expression *MakeBin(TokType operation, Expression *left, Expression *right);
void BlockAppend(Block *b, Statement *s);
char *CopyName();
char *strDuplicate(const char *s);

#endif
