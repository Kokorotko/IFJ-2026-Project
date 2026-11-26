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

#include "structure.h"

#ifndef CODE_STRUCT // Nedefinuj soubor znova, pokud uz je definovan
#define CODE_STRUCT

/// @brief Vybírá mezi datovými typy a absolutně ničím
typedef enum {
  DT_NONE,
  DT_INT,
  DT_DOUBLE,
  DT_STRING,
  DT_UNIT,
  DT_BOOL
} DataType;

/// @brief Vybírá jaký typ je expression
typedef enum {
  // DataType
  E_INT,
  E_DOUBLE,
  E_STR,
  E_UNIT,
  E_VAR, // name
  E_UNARY,
  E_BIN,
  E_CALL,
  E_BOOL          // call
} ExpressionEnum; // TokType pro strukturu

/// @brief Vybírá jaký typ je statement
typedef enum {
  ST_VARDEF,     // vardef
  ST_ASSIGN,     // assign
  ST_IF,         // if
  ST_WHILE,      // while
  ST_RETURN,     // return
  ST_EXPR        // expression/call
} StatementEnum; // TokType pro strukturu2

/// @brief Struktura pro definici expression
typedef struct Expression {
  ExpressionEnum type;
  TokType operation;   // for unary/binary
  DataType resultType; // Data (Naplnuje Sematika)
  int line;
  int varId;
  union // Union == Choose one of these
  {
    int intValue;
    double doubleValue;
    char *stringValue;
    char *name;
    bool boolValue;

    struct // Strom
    {
      struct Expression *right;
      struct Expression *left;
    } binary;

    struct Expression *unitary;

    struct // Funkce
    {
      char *name;
      struct Expression **args; // Pointer that points my beloved
      int argc;
    } call;
  } data;
} Expression;

typedef struct Statement
    Statement; // Aby Statement věděl o Bloku a block věděl o statementu

/// @brief Stromový block
typedef struct {
  Statement *first;
  Statement *last;
} Block;

/// @brief
struct Statement {
  StatementEnum type;
  int line;
  Statement *next;
  union {
    struct // vardef
    {
      char *name;
      bool isVar;
      int id;
      DataType type; // INT, DOUBLE, STRING
      Expression *init;
    } ST_vardef;

    struct // assign
    {
      char *name;
      int id;
      DataType type; // INT, DOUBLE, STRING
      Expression *value;
    } ST_assign;

    struct // if
    {
      Expression *condition;
      Block *then;
      Block *els; // nemuze se jmenovat else :(
    } ST_if;

    struct // While
    {
      Expression *condition;
      Block *body;
    } ST_while;

    Expression *retur; // NULL = bare return
    Expression *expression;
  } data;
};

/// @brief Struktura pro parametr
typedef struct {
  char *name;
  DataType type;
} Param;

/// @brief Strukruta por definici funkce
typedef struct FuncDef {
  char *name;
  Param *params; // Params se chovají jak VAL!
  int paramCount;
  DataType returnType;
  bool isMain; // Main může být pouze jeden
  Block *body;
  struct FuncDef *next; // další funkce
} FuncDef;

#endif