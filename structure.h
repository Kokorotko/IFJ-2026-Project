//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef STRUCTURE_H //Nedefinuj soubor znova, pokud uz je definovan
#define STRUCTURE_H

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

/*Kod generovany Cloud AI: "Vygenerej mi enum pro LEX tokeny potřebné z dokumentace" */
typedef enum 
{
    // speciální (End of File, End Of Line, )
    T_EOF, T_BLOCKSTART, T_BLOCKEND,
    // literály a identifikátor
    T_IDENTIFICATOR, T_INT_LIT, T_DOUBLE_LIT, T_STR_LIT,
    // klíčová slova
    KW_DEF, KW_VAL, KW_VAR, KW_IF, KW_THEN, KW_ELSE, KW_WHILE, KW_DO, KW_RETURN, KW_IMPORT, KW_INT, KW_DOUBLE, KW_STRING, KW_UNIT, KW_BOOL, KW_FOR, KW_TYPE, KW_CASE, KW_CLASS, KW_EXTENSION, KW_LIST, KW_MATCH,
    // operátory a interpunkce
    T_PLUS, T_MINUS, T_MUL, T_DIV, T_LT, T_LE, T_GT, T_GE, T_EQ, T_NEQ, T_ASSIGN, T_LPAREN, T_RPAREN, T_COMMA, T_COLON, T_SEMI, T_DOT, T_AT, T_STAR_IMPORT
} TokType;

typedef struct 
{
    TokType type;
    int line;
    union { long long i; double d; char *s; } val; //Vybírá se z jednoho (long long, double or char)
} Token;

typedef enum
{
    ERR_OK          = 0,   // Liv gud :)
    ERR_LEX         = 1,   // chyba lexikální analýzy (lexém, komentář, odsazení)
    ERR_SYNTAX      = 2,   // chyba syntaktické analýzy (syntaxe, prolog, odsazené bloky)
    ERR_UNDEFINED   = 3,   // nedefinovaná funkce/proměnná/typ, chybí @main
    ERR_REDEFINED   = 4,   // redefinice funkce/proměnné/typu, více @main
    ERR_ARGS        = 5,   // špatný počet/typ argumentů, parametr u @main
    ERR_TYPE        = 6,   // typová kompatibilita, odvození typu, návratový typ
    ERR_ASSIGN      = 7,   // zápis do val nebo do parametru funkce
    ERR_OTHER_SEM   = 10,  // ostatní sémantické chyby (nepovolený return, nulový krok...)
    ERR_RUNTIME     = 40,  // běhová chyba; překladač ji negeneruje, jen kód cílového programu (EXIT int@40)
    ERR_INTERNAL    = 99   // interní chyba překladače (např. malloc)
} ErrorCode;

#endif
