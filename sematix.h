// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

// include
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bst.h"
#include "codeStruct.h"
#include "structure.h"

#ifndef SEMATIX // Nedefinuj soubor znova, pokud uz je definovan
#define SEMATIX

typedef struct VarSym {
  char *name;
  int id; // číslování pro codeGen
  DataType type;
  bool isVar; // Bude true kdyz je variable bude false kdyz value nebo parametr
} VarSym;

typedef struct Scope {
  Tree_Node *vars;      // Seznam promennych v bloku
  struct Scope *parent; // Ukazatel na nadrazeny blok treba z if do funkce
} Scope;

typedef struct FuncSym {
  char *name;
  Param *params;
  int paramCount;
  DataType returnType;
  bool isAnyParam; // Bude true kdyz pro print a pro toStr
} FuncSym;

typedef struct {
  Tree_Node *functions;    // Vsechny funkce programu
  Scope *curScope;         // Aktualni blok
  DataType curFuncRetType; // Vracejici typ funkce kterou aktualne kontrolujeme
  int nextVarId;
  bool curFuncExplicitUnit; // Ma aktualni funkce v hlavicce napsano ": Unit"?
  bool hasReturn;           // Ma funkce v sobe return?
} SemContext;

/// @brief Vstupní bod sémantické analýzy.
/// Projde všechny funkce a zkontroluje typy a definice a @main a doplní
/// resultType ve výrazech. Při chybě volá ThrowError(ErrorCode), při úspěchu
/// vrací ERR_OK.
ErrorCode SemanticAnalysis(FuncDef *program);
bool IsNumeric(DataType t);
bool isTypeCompatible(DataType t, DataType d);
void PushScope(SemContext *ctx);
void ScopePop(SemContext *ctx);
VarSym *FindVarInCurScope(SemContext *ctx, const char *name);
VarSym *FindVar(SemContext *ctx, const char *name);
int AddVar(SemContext *ctx, char *name, DataType type, bool isVar);
FuncSym *FindUserFunction(SemContext *ctx, const char *name);
void AddFunc(SemContext *ctx, char *name, Param *params, int paramCount,
             DataType returnType, bool isAnyParam);
void FreeFunctions(SemContext *ctx);
void RegisterBuiltins(SemContext *ctx);
DataType analyzeExpression(SemContext *ctx, Expression *exp);
DataType analyzeBlock(SemContext *ctx, Block *block, bool *alwaysReturns);
DataType analyzeStatement(SemContext *ctx, Statement *sta, bool *alwaysReturns,
                          bool isLastInFunc);
DataType analyzeBlock(SemContext *ctx, Block *block, bool *alwaysReturns);
ErrorCode SemanticAnalysis(FuncDef *program);

#endif