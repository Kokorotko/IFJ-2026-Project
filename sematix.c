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

#include "sematix.h"
#include "syntax.h"

bool IsNumeric(DataType t) { return (t == DT_INT || t == DT_DOUBLE); }
// Jedina konverze povolena INT -> DOUBLE
bool isTypeCompatible(DataType t, DataType d) {
  if (t == d) {
    return true;
  }
  if (t == DT_DOUBLE && d == DT_INT) {
    return true;
  }
  return false;
}

void ScopePush(SemContext *ctx) {
  Scope *newScope = (Scope *)malloc(sizeof(Scope));
  MallocChecker(newScope);
  newScope->vars = NULL;
  newScope->parent = ctx->curScope; // Napojeni na nadrazeny blok
  ctx->curScope = newScope;
}

void ScopePop(SemContext *ctx) {
  if (ctx->curScope == NULL) {
    return;
  }
  Scope *old = ctx->curScope;
  VarSym *v = old->vars;
  while (v != NULL) {
    VarSym *next = v->next;
    free(v);
    v = next;
  }
  ctx->curScope = old->parent; // Vratime se o uroven vys
  free(old);
}

VarSym *FindVarInCurScope(SemContext *ctx, const char *name) {
  if (ctx->curScope == NULL) {
    return NULL;
  }
  for (VarSym *v = ctx->curScope->vars; v != NULL; v = v->next) {
    if (strcmp(v->name, name) == 0) {
      return NULL;
    }
  }
  return NULL;
}

VarSym *FindVar(SemContext *ctx, const char *name) {
  for (Scope *s = ctx->curScope; s != NULL; s = s->parent) {
    for (VarSym *v = s->vars; v != NULL; v = v->next) {
      if (strcmp(v->name, name) == 0) {
        return v;
      }
    }
  }
  return NULL;
}

void AddVar(SemContext *ctx, char *name, DataType type, bool isVar) {
  if (FindVarInCurScope(ctx, name) != NULL) {
    ThrowError(ERR_REDEFINED);
  }
  VarSym *v = (VarSym *)malloc(sizeof(VarSym));
  MallocChecker(v);
  v->name = name;
  v->type = type;
  v->isVar = isVar;
  v->next = ctx->curScope->vars;
  ctx->curScope->vars = v;
}

FuncSym *FindFunc(SemContext *ctx, const char *name) {
  for (FuncSym *f = ctx->functions; f != NULL; f = f->next) {
    if (strcmp(f->name, name) == 0) {
      return f;
    }
  }
  return NULL;
}

void AddFunc(SemContext *ctx, char *name, Param *params, int paramCount,
             DataType returnType, bool isAnyParam) {
  if (FindFunc(ctx, name) != NULL) {
    ThrowError(ERR_REDEFINED);
  }
  FuncSym *f = (FuncSym *)malloc(sizeof(FuncSym));
  MallocChecker(f);
  f->name = name;
  f->params = params;
  f->paramCount = paramCount;
  f->returnType = returnType;
  f->isAnyParam = isAnyParam;
  f->next = ctx->functions;
  ctx->functions = f;
}

void FreeFunctions(SemContext *ctx) {
  FuncSym *f = ctx->functions;
  while (f != NULL) {
    FuncSym *next = f->next;
    free(f);
    f = next;
  }
  ctx->functions = NULL;
}

Param p_toInt[] = {{"value", DT_DOUBLE}};
Param p_strLength[] = {{"value", DT_STRING}};
Param p_substr[] = {{"value", DT_STRING}, {"start", DT_INT}, {"end", DT_INT}};
Param p_ord[] = {{"value", DT_STRING}, {"index", DT_INT}};
Param p_chr[] = {{"code", DT_INT}};

void RegisterBuiltins(SemContext *ctx) {
  AddFunc(ctx, "print", NULL, 1, DT_UNIT, true);
  AddFunc(ctx, "readLine", NULL, 0, DT_STRING, false);
  AddFunc(ctx, "readInt", NULL, 0, DT_INT, false);
  AddFunc(ctx, "readDouble", NULL, 0, DT_DOUBLE, false);
  AddFunc(ctx, "toStr", NULL, 1, DT_STRING, true);
  AddFunc(ctx, "toInt", p_toInt, 1, DT_INT, false);
  AddFunc(ctx, "strLength", p_strLength, 1, DT_INT, false);
  AddFunc(ctx, "substr", p_substr, 3, DT_STRING, false);
  AddFunc(ctx, "ord", p_ord, 2, DT_INT, false);
  AddFunc(ctx, "chr", p_chr, 1, DT_STRING, false);
}

DataType analyzeExpression(SemContext *ctx, Expression *exp) {
  if (exp == NULL) {
    return DT_NONE;
  }
  switch (exp->type) {
  case E_INT:
    exp->resultType = DT_INT;
    return DT_INT;
  case E_DOUBLE:
    exp->resultType = DT_DOUBLE;
    return DT_DOUBLE;
  case E_STR:
    exp->resultType = DT_STRING;
    return DT_STRING;
  case E_UNIT:
    exp->resultType = DT_UNIT;
    return DT_UNIT;
  case E_VAR: { // aby WC23 nehazel warning
    VarSym *v = FindVar(ctx, exp->data.name);
    if (v == NULL) {
      ThrowError(ERR_UNDEFINED);
    }
    exp->resultType = v->type;
    return v->type;
  }
  case E_UNARY: {
    DataType t = analyzeExpression(ctx, exp->data.unitary);
    if (exp->operation == T_MINUS || exp->operation == P_UMINUS ||
        exp->operation == T_PLUS) {
      if (IsNumeric(t) == false) {
        ThrowError(ERR_TYPE);
      }
      exp->resultType = t;
      return t;
    }
    ThrowError(ERR_TYPE);
    return DT_NONE;
  }
  case E_BIN: {
    DataType leftType = analyzeExpression(ctx, exp->data.binary.left);
    DataType rightType = analyzeExpression(ctx, exp->data.binary.right);
    switch (exp->operation) {
    case T_PLUS:
      if (leftType == DT_INT && rightType == DT_INT) {
        exp->resultType = DT_INT;
      } else if (IsNumeric(leftType) && IsNumeric(rightType)) {
        exp->resultType = DT_DOUBLE;
      } else if (leftType == DT_STRING && rightType == DT_STRING) {
        exp->resultType = DT_STRING;
      } else {
        ThrowError(ERR_TYPE);
      }
      return exp->resultType;
    case T_MINUS:
    case T_MUL:
    case T_DIV:
      if (leftType == DT_INT && rightType == DT_INT) {
        exp->resultType = DT_INT;
      } else if (IsNumeric(leftType) && IsNumeric(rightType)) {
        exp->resultType = DT_DOUBLE;
      } else {
        ThrowError(ERR_TYPE);
      }
      return exp->resultType;
    case T_LT:
    case T_LE:
    case T_GT:
    case T_GE:
      if ((IsNumeric(leftType) && IsNumeric(rightType)) ||
          (leftType == DT_STRING && rightType == DT_STRING)) {
        exp->resultType = DT_BOOL;
        return DT_BOOL;
      }
      ThrowError(ERR_TYPE);
      return DT_NONE;
    case T_EQ:
    case T_NEQ:
      if ((IsNumeric(leftType) && IsNumeric(rightType)) ||
          (leftType == rightType && leftType != DT_NONE &&
           rightType != DT_NONE)) {
        exp->resultType = DT_BOOL;
        return DT_BOOL;
      }
      ThrowError(ERR_TYPE);
      return DT_NONE;
    default:
      ThrowError(ERR_TYPE);
      return DT_NONE;
    }
  }
  case E_CALL: {
    FuncSym *f = FindFunc(ctx, exp->data.call.name);
    if (f == NULL) {
      ThrowError(ERR_UNDEFINED);
    }
    if (exp->data.call.argc != f->paramCount) {
      ThrowError(ERR_ARGS);
    }
    for (int i = 0; i < exp->data.call.argc; i++) {
      DataType argType = analyzeExpression(ctx, exp->data.call.args[i]);
      if (argType == DT_BOOL || argType == DT_NONE) {
        ThrowError(ERR_ARGS);
      }
      if (!f->isAnyParam && !isTypeCompatible(f->params[i].type, argType)) {
        ThrowError(ERR_ARGS);
      }
    }
    exp->resultType = f->returnType;
    return f->returnType;
  }
  default:
    ThrowError(ERR_INTERNAL);
    return DT_NONE;
  }
}

DataType analyzeStatement(SemContext *ctx, Statement *sta, bool *alwaysReturns,
                          bool isLastInFunc) {
  *alwaysReturns = false;
  if (sta == NULL) {
    return DT_UNIT;
  }
  switch (sta->type) {
  case ST_VARDEF: {
    DataType declaredType = sta->data.ST_vardef.type;
    DataType initType = DT_NONE;
    if (sta->data.ST_vardef.init != NULL) {
      initType = analyzeExpression(ctx, sta->data.ST_vardef.init);
    }
    if (declaredType == DT_BOOL || initType == DT_BOOL) {
      ThrowError(ERR_TYPE);
    }
    if (declaredType == DT_NONE) {
      if (initType == DT_NONE) {
        ThrowError(ERR_TYPE);
      }
      declaredType = initType;
      sta->data.ST_vardef.type = declaredType;
    } else if (initType != DT_NONE &&
               !isTypeCompatible(declaredType, initType)) {
      ThrowError(ERR_TYPE);
    }
    AddVar(ctx, sta->data.ST_vardef.name, declaredType,
           sta->data.ST_vardef.isVar);
    return DT_UNIT;
  }
  case ST_ASSIGN: {
    VarSym *v = FindVar(ctx, sta->data.ST_assign.name);
    if (v == NULL) {
      ThrowError(ERR_UNDEFINED);
    }
    if (v->isVar == false) {
      ThrowError(ERR_ASSIGN);
    }
    DataType valType = analyzeExpression(ctx, sta->data.ST_assign.value);
    if (valType == DT_BOOL || !isTypeCompatible(v->type, valType)) {
      ThrowError(ERR_TYPE);
    }
    return DT_UNIT;
  }
  case ST_IF: {
    DataType conType = analyzeExpression(ctx, sta->data.ST_if.condition);
    if (conType != DT_BOOL) {
      ThrowError(ERR_TYPE);
    }
    bool thenRet = false;
    bool elseRet = false;
    ScopePush(ctx);
    DataType thenType = analyzeBlock(ctx, sta->data.ST_if.then, &thenRet);
    ScopePop(ctx);
    DataType elseType = DT_UNIT;
    if (sta->data.ST_if.els != NULL) {
      ScopePush(ctx);
      elseType = analyzeBlock(ctx, sta->data.ST_if.els, &elseRet);
      ScopePop(ctx);
    }
    if (thenRet && elseRet) {
      *alwaysReturns = true;
      return DT_NONE;
    }
    if (isLastInFunc == true) {
      if (ctx->curFuncExplicitUnit) {
        return DT_UNIT;
      }
      if (ctx->curFuncRetType == DT_NONE && thenType == DT_UNIT &&
          elseType == DT_UNIT) {
        return DT_UNIT;
      }
      ThrowError(ERR_TYPE);
    }
    return DT_UNIT;
  }
  case ST_WHILE: {
    DataType conType = analyzeExpression(ctx, sta->data.ST_if.condition);
    if (conType != DT_BOOL) {
      ThrowError(ERR_TYPE);
    }
    bool bodyRet = false;
    ScopePush(ctx);
    analyzeBlock(ctx, sta->data.ST_while.body, &bodyRet);
    ScopePop(ctx);
    return DT_UNIT;
  }
  case ST_RETURN: {
    ctx->hasReturn = true;
    *alwaysReturns = true;
    if (sta->data.retur == NULL) {
      if (ctx->curFuncExplicitUnit == false) {
        ThrowError(ERR_TYPE);
      }
    } else {
      DataType retType = analyzeExpression(ctx, sta->data.retur);
      if (retType == DT_BOOL) {
        ThrowError(ERR_TYPE);
      }
      if (!ctx->curFuncExplicitUnit &&
          !isTypeCompatible(ctx->curFuncRetType, retType)) {
        ThrowError(ERR_TYPE);
      }
    }
    return DT_NONE;
  }
  case ST_EXPR: {
    DataType t = analyzeExpression(ctx, sta->data.expression);
    return t;
  }
  default:
    ThrowError(ERR_INTERNAL);
    return DT_NONE;
  }
}

DataType analyzeBlock(SemContext *ctx, Block *block, bool *alwaysReturns) {
  *alwaysReturns = false;
  if (block == NULL || block->first == NULL) {
    return DT_UNIT;
  }
  DataType lastType = DT_UNIT;
  for (Statement *s = block->first; s != NULL; s = s->next) {
    bool staReturns = false;
    bool isLast = (s->next == NULL);
    lastType = analyzeStatement(ctx, s, &staReturns, isLast);
    if (staReturns) {
      *alwaysReturns = true;
    }
  }
  return lastType;
}

ErrorCode SemanticAnalysis(FuncDef *program) {
  SemContext ctx = {.functions = NULL,
                    .curScope = NULL,
                    .curFuncRetType = DT_NONE,
                    .curFuncExplicitUnit = false,
                    .hasReturn = false};
  RegisterBuiltins(&ctx);
  int mainCount = 0;
  for (FuncDef *f = program; f != NULL; f = f->next) {
    if (f->isMain) {
      mainCount++;
      if (mainCount > 1) {
        ThrowError(ERR_REDEFINED);
      }
      if (f->paramCount != 0) {
        ThrowError(ERR_ARGS);
      }
      if (f->returnType != DT_UNIT && f->returnType != DT_NONE) {
        ThrowError(ERR_TYPE);
      }
    }
    AddFunc(&ctx, f->name, f->params, f->paramCount, f->returnType, false);
  }
  if (mainCount == 0) {
    ThrowError(ERR_UNDEFINED);
  }
  // Kontrola tel funkci
  for (FuncDef *f = program; f != NULL; f = f->next) {
    ctx.curFuncRetType = f->returnType;
    ctx.curFuncExplicitUnit = (f->returnType == DT_UNIT);
    ctx.hasReturn = false;
    ScopePush(&ctx);

    for (int i = 0; i < f->paramCount; i++) {
      if (f->params[i].type == DT_BOOL) {
        ThrowError(ERR_TYPE);
      }
      AddVar(&ctx, f->params[i].name, f->params[i].type, false);
    }
    bool alwaysReturns = false;
    DataType bodyType = analyzeBlock(&ctx, f->body, &alwaysReturns);
    // Kontrola navratoveho typu funkce
    if (f->returnType == DT_NONE) {
      if (ctx.hasReturn) {
        ThrowError(ERR_TYPE); // Pri return musi byt navratovy typ explicitni
      }
      if (bodyType == DT_BOOL) {
        ThrowError(ERR_TYPE);
      }
      f->returnType = bodyType;
      FuncSym *fs = FindFunc(&ctx, f->name);
      if (fs != NULL) {
        fs->returnType = bodyType;
      } else if (ctx.curFuncExplicitUnit == false) {
        if (!alwaysReturns && !isTypeCompatible(f->returnType, bodyType)) {
          ThrowError(ERR_TYPE);
        }
      }
      ScopePop(&ctx);
    }
  }
  FreeFunctions(&ctx);
  return ERR_OK;
}