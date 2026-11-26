// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef CODEGEN_H // Nedefinuj soubor znova, pokud uz je definovan
#define CODEGEN_H

#include <stdio.h>

#include "codeStruct.h"
#include "structure.h"

/// @brief Záznam proměnné ve scopu CodeGenu (zásobník, nejnovější nahoře)
typedef struct VarEntry {
  const char *name; // jméno ze zdroje (nekopíruje se, patří AST)
  int id;           // jedinečné číslo v rámci funkce -> LF@name$id
  DataType type;    // typ proměnné (pro konverzi INT -> DOUBLE při přiřazení)
  struct VarEntry *next; // další (starší) proměnná
} VarEntry;

int GenProgram(FuncDef *internalProgram, FILE *output);
void Emit(const char *message, ...);
void GenExpr(Expression *e);
void GenBuiltinCall(Expression *e);
void GenBinaryOp(Expression *e);
void GenStatement(Statement *s);
void GenBlock(Block *b);
void GenReturn(Expression *value);
void VarPreparation(Block *b);
void GenFunction(FuncDef *f);
void GenReadBuiltin(const char *name, const char *type, const char *fallback);
void GenBuiltins(void);

#endif
