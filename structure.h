// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef STRUCTURE_H // Nedefinuj soubor znova, pokud uz je definovan
#define STRUCTURE_H

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

/*Kod generovany Cloud AI: "Vygenerej mi enum pro LEX tokeny potřebné z dokumentace" 
 * Moc efektivni to nebylo :(
 */
typedef enum 
{
    T_EOF,           // End of File
    T_BLOCKSTART,    // začátek blok
    T_BLOCKEND,      // konec bloku
    T_EOL,           // End of Line
    T_INDENDATION,   // Indendation

    // literály a identifikátor
    T_ID, // jméno proměnné nebo funkce: x, factorial, _tmp
    T_INT_LIT,       // celé číslo: 42, 1_000_000
    T_DOUBLE_LIT,    // desetinné číslo: 3.14, 1e5, .5
    T_STRING_LIT,    // řetězec: "ahoj" (i víceřádkový """...""")

    // klíčová slova
    KW_DEF,          // def: definice funkce
    KW_VAL,          // val: nemodifikovatelná proměnná
    KW_VAR,          // var: modifikovatelná proměnná
    KW_IF,           // if
    KW_THEN,         // then
    KW_ELSE,         // else
    KW_WHILE,        // while
    KW_DO,           // do
    KW_RETURN,       // return
    KW_IMPORT,       // import: prolog "import ifj26.*"
    KW_INT,          // typ Int
    KW_DOUBLE,       // typ Double
    KW_STRING,       // typ String
    KW_UNIT,         // typ Unit

    // operátory a interpunkce
    T_PLUS,          // +  sčítání, konkatenace řetězců, unární plus
    T_MINUS,         // -  odčítání, unární mínus
    T_MUL,           // *  násobení
    T_DIV,           // /  dělení
    T_LT,            // <
    T_LE,            // <=
    T_GT,            // >
    T_GE,            // >=
    T_EQ,            // == rovnost
    T_NEQ,           // != nerovnost
    T_ASSIGN,        // =  přiřazení (i v definici proměnné a funkce)
    T_LPAREN,        // (
    T_RPAREN,        // )
    T_COMMA,         // ,  odděluje parametry a argumenty
    T_COLON,         // :  před typem (x: Int)
    T_SEMI,          // ;  odděluje příkazy na jednom řádku
    T_DOT,           // .  v prologu
    T_AT,            // @  anotace @main
    T_ENDSYMBOL,     // $ pro SA LL Tabulku
    T_MIDDLESYMBOL,  // < pro SA LL Tabulku
    P_TERM,          // i/E pro SA LL Tabulku
    P_NONTERM,       // Neterminal
    P_UMINUS         // Unarni minus
} TokType;

typedef struct
{
    TokType type;
    int line;
    union
    {
        long long i;
        double d;
        char *s;
    } val; // Vybírá se z jednoho (long long, double or char)
} Token;

typedef enum
{
    ERR_OK = 0,         // Liv gud :)
    ERR_LEX = 1,        // chyba lexikální analýzy (lexém, komentář, odsazení)
    ERR_SYNTAX = 2,     // chyba syntaktické analýzy (syntaxe, prolog, odsazené bloky)
    ERR_UNDEFINED = 3,  // nedefinovaná funkce/proměnná/typ, chybí @main
    ERR_REDEFINED = 4,  // redefinice funkce/proměnné/typu, více @main
    ERR_ARGS = 5,       // špatný počet/typ argumentů, parametr u @main
    ERR_TYPE = 6,       // typová kompatibilita, odvození typu, návratový typ
    ERR_ASSIGN = 7,     // zápis do val nebo do parametru funkce
    ERR_OTHER_SEM = 10, // ostatní sémantické chyby (nepovolený return, nulový krok...)
    ERR_RUNTIME = 40,   // běhová chyba; překladač ji negeneruje, jen kód cílového programu (EXIT int@40)
    ERR_INTERNAL = 99   // interní chyba překladače (např. malloc)
} ErrorCode;

void Error_Handling(ErrorCode errorCode)
{
    exit(errorCode);
}

void MallocChecker(void *pointer)
{
    if (pointer == NULL)
    {
        Error_Handling(ERR_INTERNAL);
    }
}

#endif
