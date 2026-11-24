// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef STRUCTURE_H // Nedefinuj soubor znova, pokud uz je definovan
#define STRUCTURE_H

typedef enum {
  // speciální (End of File, End Of Line, )
  T_EOF,
  T_EOL,
  T_BLOCKSTART,
  T_BLOCKEND,
  T_INDENDATION,
  T_ERROR,
  T_SKIP, // Syntax preskoci tento Token
  // literály a identifikátor
  T_ID,         // identifikátor
  T_INT_LIT,    // čislo int
  T_DOUBLE_LIT, // číslo double
  T_STRING_LIT,
  // klíčová slova
  KW_MAIN,
  KW_DEF,
  KW_VAL,
  KW_VAR,
  KW_IF,
  KW_THEN,
  KW_ELSE,
  KW_WHILE,
  KW_DO,
  KW_RETURN,
  KW_IMPORT,
  KW_INT,
  KW_DOUBLE,
  KW_STRING,
  KW_UNIT,
  // operátory a interpunkce
  T_PLUS,         // +  sčítání, konkatenace řetězců, unární plus
  T_MINUS,        // -  odčítání, unární mínus
  T_MUL,          // *  násobení
  T_DIV,          // /  dělení
  T_LT,           // <
  T_LE,           // <=
  T_GT,           // >
  T_GE,           // >=
  T_EQ,           // == rovnost
  T_NEQ,          // != nerovnost
  T_ASSIGN,       // =  přiřazení (i v definici proměnné a funkce)
  T_LPAREN,       // (
  T_RPAREN,       // )
  T_COMMA,        // ,  odděluje parametry a argumenty
  T_COLON,        // :  před typem (x: Int)
  T_SEMI,         // ;  odděluje příkazy na jednom řádku
  T_DOT,          // .  v prologu
  T_AT,           // @  anotace @main
  T_ENDSYMBOL,    // $ pro SA LL Tabulku
  T_MIDDLESYMBOL, // < pro SA LL Tabulku
  T_QUOTATION,    //" pro string lit
  P_TERM,         // i/E pro SA LL Tabulku
  P_NONTERM,      // Neterminal
  P_UMINUS,       // Unarni minus

  // Extensions
  KW_BOOL,
  KW_CASE,
  KW_CLASS,
  KW_EXTENSION,
  KW_FOR,
  KW_LIST,
  KW_MATCH,
  KW_TYPE
} TokType;

typedef struct {
  TokType type;
  int line;
  union {
    int i;
    double d;
    char *s;
  } val; // Vybírá se z jednoho (long long, double or char)
} Token;

typedef enum {
  ERR_OK = 0,     // Liv gud :)
  ERR_LEX = 1,    // chyba lexikální analýzy (lexém, komentář, odsazení)
  ERR_SYNTAX = 2, // chyba syntaktické analýzy (syntaxe, prolog, odsazené bloky)
  ERR_UNDEFINED = 3, // nedefinovaná funkce/proměnná/typ, chybí @main
  ERR_REDEFINED = 4, // redefinice funkce/proměnné/typu, více @main
  ERR_ARGS = 5,      // špatný počet/typ argumentů, parametr u @main
  ERR_TYPE = 6,      // typová kompatibilita, odvození typu, návratový typ
  ERR_ASSIGN = 7,    // zápis do val nebo do parametru funkce
  ERR_OTHER_SEM =
      10, // ostatní sémantické chyby (nepovolený return, nulový krok...)
  ERR_RUNTIME = 40, // běhová chyba; překladač ji negeneruje, jen kód cílového
                    // programu (EXIT int@40)
  ERR_INTERNAL = 99 // interní chyba překladače (např. malloc)
} ErrorCode;

void ErrorHandlingSyntax(ErrorCode errorCode, Token token);
void ThrowError(ErrorCode errorCode);
void MallocChecker(void *pointer);

#endif
