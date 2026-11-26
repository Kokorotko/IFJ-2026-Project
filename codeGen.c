// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codeGen.h"
#include "codeStruct.h"

static FILE *out = NULL;                // Output
static FuncDef *program = NULL;         // AST strom
static FuncDef *currentFunction = NULL; // funkce, kterou zrovna generujeme
static int labelCounter = 0;            // cislovani pro if/while
static int varCounter = 0;              // cislovani id ve funkcich
static int genError = 0;                // errorHandling

static VarEntry *scopeTop = NULL;

// Claude, udělej mi printf ale do souboru (nakonec to je do stdout)
/// @brief printf pro jednu instrukci, přidá odsazení řádku
void Emit(const char *message, ...) {
  va_list args; // list dalších argumentů
  va_start(args, message);
  vfprintf(out, message, args); // file printf
  va_end(args);
  fputc('\n', out); // nastaví LF
}

/// @brief Přidá proměnnou do aktuálního scopu
static void PushScope(const char *name, int id, DataType type) {
  VarEntry *entry = malloc(sizeof(VarEntry));
  MallocChecker(entry);
  entry->name = name;
  entry->id = id;
  entry->type = type;
  entry->next = scopeTop;
  scopeTop = entry;
}

/// @brief Najde nejnovější proměnnou daného jména
static VarEntry *LookupScope(const char *name) {
  VarEntry *entry = scopeTop;
  while (entry != NULL) {
    if (strcmp(entry->name, name) == 0) {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

/// @brief Zahodí všechny proměnné přidané po `mark` (konec bloku)
static void RemoveScope(VarEntry *mark) {
  while (scopeTop != mark) {
    VarEntry *old = scopeTop;
    scopeTop = old->next;
    free(old);
  }
}

/// @brief Najde uživatelskou funkci podle jména (NULL = vestavěná)
static FuncDef *FindUserFunction(const char *name) {
  FuncDef *f = program;
  while (f != NULL) {
    if (strcmp(f->name, name) == 0) {
      return f;
    }
    f = f->next;
  }
  return NULL;
}

/// @brief Převede int na double (float)
static void ConvertTop(DataType from, DataType to) {
  if (from == DT_INT && to == DT_DOUBLE) {
    Emit("INT2FLOATS");
  }
}

/// @brief PUSHS řetězcové konstanty s escapováním (znaky <= 32, # a \ jako
/// \ddd) Předpokládám, že lexer už escape sekvence vyřešil (ve stringValue je
/// skutečný \n apod.)
static void PushString(const char *s) {
  fprintf(out, "PUSHS string@");
  for (; *s != '\0'; s++) {
    unsigned char c = (unsigned char)*s;
    if (c <= 32 || c == 35 || c == 92) // 32 = mezera, 35 ´= # a 92 = '\'
    {
      fprintf(out, "\\%03d", c);
    } else {
      fputc(c, out);
    }
  }
  fputc('\n', out);
}

/// @brief Vestavěné funkce, výsledek vždy zůstane na zásobníku (i Unit =
/// nil@nil)
void GenBuiltinCall(Expression *e) {
  const char *name = e->data.call.name;
  Expression **args = e->data.call.args;

  if (strcmp(name, "print") == 0) {
    GenExpr(args[0]);
    Emit("POPS GF@$tmp1");
    if (args[0]->resultType == DT_UNIT) {
      Emit("WRITE string@()"); // WRITE nil by vypsal "nil"
    } else {
      Emit("WRITE GF@$tmp1");
    }
    Emit("PUSHS nil@nil");
  } else if (strcmp(name, "toStr") == 0) {
    GenExpr(args[0]);
    switch (args[0]->resultType) {
    case DT_INT:
      Emit("INT2STRS");
      break;
    case DT_DOUBLE:
      Emit("FLOAT2STRS");
      break;
    case DT_UNIT:
      Emit("POPS GF@$tmp1");
      Emit("PUSHS string@()");
      break;
    case DT_BOOL: { // pro bool neexistuje konverzní instrukce
      int id = labelCounter++;
      Emit("PUSHS bool@true");
      Emit("JUMPIFEQS $bool_true_%d", id);
      Emit("PUSHS string@false");
      Emit("JUMP $bool_end_%d", id);
      Emit("LABEL $bool_true_%d", id);
      Emit("PUSHS string@true");
      Emit("LABEL $bool_end_%d", id);
      break;
    }
    default: // String zůstává
      break;
    }
  } else if (strcmp(name, "toInt") == 0) {
    GenExpr(args[0]);
    ConvertTop(args[0]->resultType, DT_DOUBLE);
    Emit("FLOAT2INTS");
  } else if (strcmp(name, "strLength") == 0) {
    GenExpr(args[0]);
    Emit("POPS GF@$tmp1");
    Emit("STRLEN GF@$tmp1 GF@$tmp1");
    Emit("PUSHS GF@$tmp1");
  } else if (strcmp(name, "chr") == 0) {
    GenExpr(args[0]);
    Emit("INT2CHARS");
  } else {
    // readLine, readInt, readDouble, subStr, ord -> vlastní funkce v
    // GenBuiltins()
    for (int i = 0; i < e->data.call.argc; i++) {
      GenExpr(args[i]);
    }
    Emit("CALL $builtin_%s", name);
  }
}

/// @brief Volání funkce: argumenty na zásobník (s konverzí na typ parametru) +
/// CALL
static void GenCall(Expression *e) {
  FuncDef *f = FindUserFunction(e->data.call.name);
  if (f == NULL) {
    GenBuiltinCall(e);
    return;
  }

  for (int i = 0; i < e->data.call.argc; i++) {
    Expression *arg = e->data.call.args[i];
    GenExpr(arg);
    ConvertTop(arg->resultType, f->params[i].type);
  }
  Emit("CALL $fn_%s", f->name);
}

/// @brief Instrukce pro binární operátor (operandy už jsou na zásobníku a
/// převedené na stejný typ)
void GenBinaryOp(Expression *e) {
  Expression *l = e->data.binary.left;
  Expression *r = e->data.binary.right;
  bool isDouble = (l->resultType == DT_DOUBLE || r->resultType == DT_DOUBLE);
  bool isString = (l->resultType == DT_STRING);

  switch (e->operation) {
  case T_PLUS:
    if (isString) {
      // CONCAT nemá zásobníkovou verzi
      Emit("POPS GF@$tmp2");
      Emit("POPS GF@$tmp1");
      Emit("CONCAT GF@$tmp3 GF@$tmp1 GF@$tmp2");
      Emit("PUSHS GF@$tmp3");
    } else {
      Emit("ADDS");
    }
    break;
  case T_MINUS:
    Emit("SUBS");
    break;
  case T_MUL:
    Emit("MULS");
    break;
  case T_DIV:
    Emit(isDouble ? "DIVS" : "IDIVS");
    break;
  case T_LT:
    Emit("LTS");
    break;
  case T_GT:
    Emit("GTS");
    break;
  case T_LE: // a <= b  ==  !(a > b)
    Emit("GTS");
    Emit("NOTS");
    break;
  case T_GE: // a >= b  ==  !(a < b)
    Emit("LTS");
    Emit("NOTS");
    break;
  case T_EQ:
    Emit("EQS");
    break;
  case T_NEQ:
    Emit("EQS");
    Emit("NOTS");
    break;
  default:
    genError = ERR_INTERNAL;
    break;
  }
}

/// @brief Vygeneruje výraz, výsledek skončí na vrcholu datového zásobníku
void GenExpr(Expression *e) {
  switch (e->type) {
  case E_INT:
    Emit("PUSHS int@%d", e->data.intValue);
    break;

  case E_DOUBLE:
    Emit("PUSHS float@%a", e->data.doubleValue);
    break;

  case E_STR:
    PushString(e->data.stringValue);
    break;

  case E_UNIT:
    Emit("PUSHS nil@nil");
    break;

  case E_VAR: {
    VarEntry *v = LookupScope(e->data.name);
    MallocChecker(v);
    Emit("PUSHS LF@%s$%d", v->name, v->id);
    break;
  }

  case E_UNARY:
    GenExpr(e->data.unitary);
    if (e->operation == T_MINUS) // unární + není potřeba
    {
      // * -1 místo 0 - x, aby fungovalo i -0.0
      Emit(e->resultType == DT_DOUBLE ? "PUSHS float@-0x1p+0" : "PUSHS int@-1");
      Emit("MULS");
    } else if (e->operation == T_NOT) {
      Emit("NOTS");
    }
    break;

  case E_BIN: {
    Expression *l = e->data.binary.left;
    Expression *r = e->data.binary.right;
    if (e->operation == T_AND || e->operation == T_OR) {
      // zkratové vyhodnocení (ANDS/ORS by vyhodnotily obě strany):
      // && skončí hned při levém false, || při levém true
      const char *shortVal = (e->operation == T_AND) ? "false" : "true";
      int id = labelCounter++;
      GenExpr(l);
      Emit("PUSHS bool@%s", shortVal);
      Emit("JUMPIFEQS $sc_short_%d", id);
      GenExpr(r);
      Emit("JUMP $sc_end_%d", id);
      Emit("LABEL $sc_short_%d", id);
      Emit("PUSHS bool@%s", shortVal);
      Emit("LABEL $sc_end_%d", id);
      break;
    }
    GenExpr(l);
    ConvertTop(l->resultType,
               r->resultType); // levý Int, pravý Double -> převeď levý
    GenExpr(r);
    ConvertTop(r->resultType,
               l->resultType); // pravý Int, levý Double -> převeď pravý
    GenBinaryOp(e);
    break;
  }

  case E_CALL:
    GenCall(e);
    break;

  case E_BOOL:
  {
    if (e->data.boolValue)
    {
      Emit("PUSHS bool@%s", "true");
    }
    else 
    {
      Emit("PUSHS bool@%s", "false");
    }
    break;
  }
}
}

/// @brief Návrat z funkce: hodnota (nebo nil) na zásobník + epilog
/// @param value NULL = bare return / tělo bez koncového výrazu (Unit)
void GenReturn(Expression *value) {
  if (value != NULL) {
    GenExpr(value);
    if (currentFunction->returnType == DT_UNIT) {
      Emit("POPS GF@$tmp1"); // u Unit funkce se hodnota zahazuje
      Emit("PUSHS nil@nil");
    } else {
      ConvertTop(value->resultType, currentFunction->returnType);
    }
  } else {
    Emit("PUSHS nil@nil");
  }
  Emit("POPFRAME");
  Emit("RETURN");
}

/// @brief Pre-pass: DEFVAR všech lokálních proměnných funkce (včetně vnořených
/// bloků)  !musí procházet ve STEJNÉM pořadí jako GenStatement (then -> else,
/// body),
void VarPreparation(Block *b) {
  MallocChecker(b);
  Statement *s = b->first;
  while (s != NULL) {
    switch (s->type) {
    case ST_VARDEF:
      Emit("DEFVAR LF@%s$%d", s->data.ST_vardef.name, varCounter++);
      break;
    case ST_IF:
      VarPreparation(s->data.ST_if.then);
      if (s->data.ST_if.els != NULL) { // if bez else BOOLTHEN
        VarPreparation(s->data.ST_if.els);
      }
      break;
    case ST_WHILE:
      VarPreparation(s->data.ST_while.body);
      break;
    default:
      break;
    }
    s = s->next;
  }
}

/// @brief Generuje kod podle statementu
/// @param s Pointer na statement

void GenStatement(Statement *s) {
  switch (s->type) {
  case ST_VARDEF: {
    Expression *init = s->data.ST_vardef.init;
    DataType type = s->data.ST_vardef.type;
    int id = varCounter++; // musí sedět s VarPreparation

    GenExpr(init);
    ConvertTop(init->resultType, type);
    // do scopu až PO init, aby `val x = x + 1` viselo na vnější x
    PushScope(s->data.ST_vardef.name, id, type);
    Emit("POPS LF@%s$%d", s->data.ST_vardef.name, id);
    break;
  }

  case ST_ASSIGN: {
    VarEntry *v = LookupScope(s->data.ST_assign.name);
    MallocChecker(v); // reused :)
    GenExpr(s->data.ST_assign.value);
    ConvertTop(s->data.ST_assign.value->resultType, v->type);
    Emit("POPS LF@%s$%d", v->name, v->id);
    break;
  }

  case ST_IF: {
    int id = labelCounter++;
    GenExpr(s->data.ST_if.condition); // bool na zásobníku
    Emit("PUSHS bool@false");
    Emit("JUMPIFEQS $if_else_%d", id);
    GenBlock(s->data.ST_if.then);
    Emit("JUMP $if_end_%d", id);
    Emit("LABEL $if_else_%d", id);
    if (s->data.ST_if.els != NULL) { // if bez else (BOOLTHEN)
      GenBlock(s->data.ST_if.els);
    }
    Emit("LABEL $if_end_%d", id);
    break;
  }

  case ST_WHILE: {
    int id = labelCounter++;
    Emit("LABEL $while_start_%d", id);
    GenExpr(s->data.ST_while.condition);
    Emit("PUSHS bool@false");
    Emit("JUMPIFEQS $while_end_%d", id);
    GenBlock(s->data.ST_while.body);
    Emit("JUMP $while_start_%d", id);
    Emit("LABEL $while_end_%d", id);
    break;
  }

  case ST_RETURN:
    GenReturn(s->data.retur);
    break;

  case ST_EXPR:
    GenExpr(s->data.expression);
    Emit("POPS GF@$tmp1"); // hodnota samostatného výrazu se zahodí
    break;
  }
}

/// @brief Blok = nový scope; příkazy jedou přes while

void GenBlock(Block *b) {
  MallocChecker(b);

  VarEntry *mark = scopeTop;
  Statement *s = b->first;
  while (s != NULL) {
    GenStatement(s);
    s = s->next;
  }
  RemoveScope(mark);
}

/// @brief Generuje kod podle funkce
/// @param f pointer na funkci

void GenFunction(FuncDef *f) {
  currentFunction = f;
  varCounter = 0;

  Emit("LABEL $fn_%s", f->name);
  Emit("CREATEFRAME");
  Emit("PUSHFRAME");

  // argumenty leží na zásobníku zleva doprava -> pop od konce
  for (int i = 0; i < f->paramCount; i++) {
    Emit("DEFVAR LF@%s$%d", f->params[i].name, i);
  }
  for (int i = f->paramCount - 1; i >= 0; i--) {
    Emit("POPS LF@%s$%d", f->params[i].name, i);
  }
  for (int i = 0; i < f->paramCount; i++) {
    PushScope(f->params[i].name, i, f->params[i].type);
  }

  // DEFVAR všech lokálních proměnných, pak se počítadlo vrátí, aby GenStatement
  // dostal stejná id
  varCounter = f->paramCount;
  VarPreparation(f->body);
  varCounter = f->paramCount;

  // tělo: poslední příkaz zapsaný přímo v bloku, pokud je to výraz, je
  // návratová hodnota
  Statement *lastExpr = NULL;
  Statement *s = f->body->first;
  while (s != NULL) {
    if (s->next == NULL && s->type == ST_EXPR) {
      lastExpr = s;
    } else {
      GenStatement(s);
    }
    s = s->next;
  }

  GenReturn(lastExpr != NULL ? lastExpr->data.expression : NULL);

  RemoveScope(NULL); // úklid scopu
}

/// @brief Vygeneruje builtin pro čtení: nil (EOF / chybný formát) -> výchozí
/// hodnota
/// @param name Jméno funkce (readLine, readInt, readDouble)
/// @param type Typ pro instrukci READ (string, int, float)
/// @param fallback Konstanta, která se vrátí místo nil

void GenReadBuiltin(const char *name, const char *type, const char *fallback) {
  Emit("LABEL $builtin_%s", name);
  Emit("CREATEFRAME");
  Emit("PUSHFRAME");
  Emit("DEFVAR LF@res");
  Emit("READ LF@res %s", type);
  Emit("JUMPIFNEQ $builtin_%s_ok LF@res nil@nil", name);
  Emit("MOVE LF@res %s", fallback);
  Emit("LABEL $builtin_%s_ok", name);
  Emit("PUSHS LF@res");
  Emit("POPFRAME");
  Emit("RETURN");
}

/// @brief Vestavěné funkce, které potřebují smyčku / ošetření chyb -> vlastní
/// funkce v IFJcode26 Stejná konvence jako u uživatelských
void GenBuiltins(void) {
  GenReadBuiltin("readLine", "string", "string@");
  GenReadBuiltin("readInt", "int", "int@0");
  GenReadBuiltin("readDouble", "float", "float@0x0p+0");
  GenReadBuiltin("readBoolean", "bool", "bool@false");

  Emit("LABEL $builtin_ord");
  Emit("CREATEFRAME");
  Emit("PUSHFRAME");
  Emit("DEFVAR LF@value");
  Emit("DEFVAR LF@index");
  Emit("DEFVAR LF@len");
  Emit("DEFVAR LF@cond");
  Emit("DEFVAR LF@res");
  Emit("POPS LF@index");
  Emit("POPS LF@value");
  Emit("STRLEN LF@len LF@value");
  Emit("LT LF@cond LF@index int@0");
  Emit("JUMPIFEQ $builtin_ord_bad LF@cond bool@true");
  Emit("LT LF@cond LF@index LF@len");
  Emit("JUMPIFEQ $builtin_ord_bad LF@cond bool@false");
  Emit("STRI2INT LF@res LF@value LF@index");
  Emit("JUMP $builtin_ord_end");
  Emit("LABEL $builtin_ord_bad");
  Emit("MOVE LF@res int@-1");
  Emit("LABEL $builtin_ord_end");
  Emit("PUSHS LF@res");
  Emit("POPFRAME");
  Emit("RETURN");

  Emit("LABEL $builtin_subStr");
  Emit("CREATEFRAME");
  Emit("PUSHFRAME");
  Emit("DEFVAR LF@value");
  Emit("DEFVAR LF@start");
  Emit("DEFVAR LF@end");
  Emit("DEFVAR LF@len");
  Emit("DEFVAR LF@cond");
  Emit("DEFVAR LF@ch");
  Emit("DEFVAR LF@res");
  Emit("POPS LF@end");
  Emit("POPS LF@start");
  Emit("POPS LF@value");
  Emit("STRLEN LF@len LF@value");
  Emit("LT LF@cond LF@start int@0");
  Emit("JUMPIFEQ $builtin_subStr_err LF@cond bool@true");
  Emit("LT LF@cond LF@end int@0");
  Emit("JUMPIFEQ $builtin_subStr_err LF@cond bool@true");
  Emit("GT LF@cond LF@start LF@end");
  Emit("JUMPIFEQ $builtin_subStr_err LF@cond bool@true");
  Emit("LT LF@cond LF@start LF@len");
  Emit("JUMPIFEQ $builtin_subStr_err LF@cond bool@false");
  Emit("GT LF@cond LF@end LF@len");
  Emit("JUMPIFEQ $builtin_subStr_err LF@cond bool@true");
  Emit("MOVE LF@res string@");
  Emit("LABEL $builtin_subStr_loop");
  Emit("JUMPIFEQ $builtin_subStr_done LF@start LF@end");
  Emit("GETCHAR LF@ch LF@value LF@start");
  Emit("CONCAT LF@res LF@res LF@ch");
  Emit("ADD LF@start LF@start int@1");
  Emit("JUMP $builtin_subStr_loop");
  Emit("LABEL $builtin_subStr_done");
  Emit("PUSHS LF@res");
  Emit("POPFRAME");
  Emit("RETURN");
  Emit("LABEL $builtin_subStr_err");
  Emit("EXIT int@40");
}

/// @brief Vygeneruje celý program v IFJcode26
/// @param program AST po sémantické analýze
/// @param output Kam se kód zapisuje (stdout)
/// @return Error kod
int GenProgram(FuncDef *innerProgram, FILE *output) {
  out = output;
  program = innerProgram;
  currentFunction = NULL;
  labelCounter = 0;
  genError = 0;
  scopeTop = NULL;

  Emit(".IFJcode26");

  Emit("DEFVAR GF@$tmp1"); // pro UNIT
  Emit("DEFVAR GF@$tmp2"); // pro CONCAT
  Emit("DEFVAR GF@$tmp3"); // pro PRINT

  //zavolej @main a skonči
  FuncDef *f = innerProgram;
  while (f != NULL && !f->isMain) {
    f = f->next;
  }
  MallocChecker(f);
  Emit("CALL $fn_%s", f->name);
  Emit("EXIT int@0");

  // všechny funkce
  f = innerProgram;
  while (f != NULL) {
    GenFunction(f);
    f = f->next;
  }
  GenBuiltins();
  return genError;
}
