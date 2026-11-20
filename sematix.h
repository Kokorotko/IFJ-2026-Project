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

#include "codeStruct.h"
#include "structure.h"

#ifndef SEMATIX // Nedefinuj soubor znova, pokud uz je definovan
#define SEMATIX

typedef struct VarSym {
  char *name;
  DataType type;
  bool isVar; // Bude true kdyz je variable bude false kdyz value nebo parametr
              // funkce
  struct VarSym *next;
} VarSym;

typedef struct Scope {
  VarSym *vars;         // Seznam promennych v bloku
  struct Scope *parent; // Ukazatel na nadrazeny blok treba z if do funkce
} Scope;

typedef struct FuncSym {
  char *name;
  Param *params;
  int paramCount;
  DataType returnType;
  bool isAnyParam; // Bude true kdyz pro print a pro toStr
  struct FuncSym *next;
} FuncSym;

typedef struct {
  FuncSym *functions;       // Vsechny funkce programu
  Scope *curScope;          // Aktualni blok
  DataType curFuncRetType;  // Vracejici typ funkce kterou aktualne kontrolujeme
  bool curFuncExplicitUnit; // Ma aktualni funkce v hlavicce napsano ": Unit"?
  bool hasReturn;           // Ma funkce v sobe return?
} SemContext;

/// @brief Vstupní bod sémantické analýzy.
/// Projde všechny funkce a zkontroluje typy a definice a @main a doplní
/// resultType ve výrazech. Při chybě volá ThrowError(ErrorCode), při úspěchu
/// vrací ERR_OK.
ErrorCode SemanticAnalysis(FuncDef *program);
static bool IsNumeric(DataType t);
static bool isTypeCompatible(DataType t, DataType d);
static void ScopePush(SemContext *ctx);
static void ScopePop(SemContext *ctx);
static VarSym *FindVarInCurScope(SemContext *ctx, const char *name);
static VarSym *FindVar(SemContext *ctx, const char *name);
static void AddVar(SemContext *ctx, char *name, DataType type, bool isVar);
static FuncSym *FindFunc(SemContext *ctx, const char *name);
static void AddFunc(SemContext *ctx, char *name, Param *params, int paramCount,
                    DataType returnType, bool isAnyParam);
static void FreeFunctions(SemContext *ctx);
static void RegisterBuiltins(SemContext *ctx);
static DataType analyzeExpression(SemContext *ctx, Expression *exp);
static DataType analyzeBlock(SemContext *ctx, Block *block,
                             bool *alwaysReturns);
static DataType analyzeStatement(SemContext *ctx, Statement *sta,
                                 bool *alwaysReturns, bool isLastInFunc);
static DataType analyzeBlock(SemContext *ctx, Block *block,
                             bool *alwaysReturns);
ErrorCode SemanticAnalysis(FuncDef *program);

#endif