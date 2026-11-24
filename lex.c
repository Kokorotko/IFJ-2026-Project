// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

// include
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lex.h"
#include "structure.h"

typedef struct Check {
  char *check_array;
  int length;
  TokType type;
} Keyword_Check;

Token returnToken;

static const Keyword_Check Keyword[] = // Keywords list
    {{"@main", 5, KW_MAIN},
     {"def", 3, KW_DEF},
     {"do", 2, KW_DO},
     {"Double", 6, KW_DOUBLE},
     {"else", 4, KW_ELSE},
     {"if", 2, KW_IF},
     {"import", 6, KW_IMPORT},
     {"Int", 3, KW_INT},
     {"return", 6, KW_RETURN},
     {"String", 6, KW_STRING},
     {"then", 4, KW_THEN},
     {"Unit", 4, KW_UNIT},
     {"val", 3, KW_VAL},
     {"var", 3, KW_VAR},
     {"while", 5, KW_WHILE},
     {"Boolean", 7, KW_BOOL},
     {"case", 4, KW_CASE},
     {"class", 5, KW_CLASS},
     {"extension", 9, KW_EXTENSION},
     {"for", 3, KW_FOR},
     {"List", 4, KW_LIST},
     {"match", 5, KW_MATCH},
     {"type", 4, KW_TYPE}};

typedef enum // tohle je pro menší větší rovná se a kombinace
{ STATE_START,
  STATE_EQ,
  STATE_EQEQ,
  STATE_LE,
  STATE_LEEQ,
  STATE_GR,
  STATE_GREQ,
  STATE_PLUS,
  STATE_MINUS,
  STATE_MUL,
  STATE_DIV,
  STATE_NOT,
  STATE_NEQ,
  STATE_COMMA,
  STATE_COLON,
  STATE_SEMI } ScannerState; // specialní znaky

typedef enum {
  NUM_INTEGER,           // int
  NUM_DOT,               // desetinná tečka
  NUM_FRACTION,          // část za desetinnou tečkou
  NUM_EXP_START,         // račátek exponentui e/E
  NUM_EXP_SIGN,          //+/-
  NUM_EXP_DIGITS,        // číslice za e/E
  NUM_SUFFIX,            // d/D
  NUM_EXP_UNDERSCORE,    // podtrřítko pro zápui čísel
  NUM_INTEGER_UNDERCORE, // podtržitko pro int
  NUM_FRACTION_UNDERCORE // podtržítko pro desetiny

} NumberState; // čísloce
static bool lineStart = true;
static int nextChar = EOF;
static int indent = 0;
static int *indentStack = NULL; // ukazatel na pole indent
static int parenDepth = 0;      // pro to jestli se vraco
static int lineNumber = 1;
//============================================================================
//                                    FUNCTIONS
//============================================================================¨

bool IsLetter(int c) // is this lil guy a letter, god i hope so
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); // is char?
}

bool IsIdentifierStart(
    int c) // if this guy is letter, can he start an identifier?
{
  return IsLetter(c) || c == '_'; // can be start to an identifier?
}

bool IsIdentifierPart(
    int c) // i guess he could, so now he can be anything, yaay
{
  return IsIdentifierStart(c) ||
         (c >= '0' && c <= '9'); // is valid identifier char?
}

TokType KeywordOrIdentifier(
    const char *text) // rozhoduje jestli je string keyword nebo identifikator
{
  int count = sizeof Keyword / sizeof Keyword[0];
  for (int i = 0; i < count; i++) {
    if (strcmp(text, Keyword[i].check_array) == 0) {
      return Keyword[i].type;
    }
  }
  return T_ID;
}
bool IsDigit(int c) { return c >= '0' && c <= '9'; }

void SkipLineComment(FILE *mainFile) // skip komentáře
{
  int c;

  do {
    c = fgetc(mainFile);
  } while (c != '\n' && c != EOF);

  if (c == EOF) {
    if (ferror(mainFile)) {
      ThrowError(ERR_INTERNAL);
    }
    return;
  }

  // Nový řádek zpracuje běžná část lexeru.
  if (ungetc(c, mainFile) == EOF) {
    ThrowError(ERR_INTERNAL);
  }
}
static int SkipBlockComment(FILE *mainFile) {
  int depth = 1;
  int newLines = 0;
  int previous = 0;

  while (depth > 0) {
    int c = fgetc(mainFile);
    if (c == EOF) {
      if (ferror(mainFile)) {
        ThrowError(ERR_INTERNAL);
        exit(ERR_INTERNAL);
      }

      // EOF před uzavřením všech komentářů.
      ThrowError(ERR_LEX);
      exit(ERR_LEX);
    }

    if (c == '\n') {
      newLines++;
    }
    if (previous == '/' && c == '*') {
      depth++;
      previous = 0;
    } else if (previous == '*' && c == '/') {
      depth--;
      previous = 0;
    } else {
      previous = c;
    }
  }
  return newLines;
}

// static bool TryLineComment(FILE *mainFile) // oveřeni jeslti se jedna o
// komentář nebo  odělení
// {
//   int next = fgetc(mainFile);

//   if (next == '/')
//   {
//     SkipLineComment(mainFile);
//     return true;
//   }

//   if (next == EOF)
//   {
//     if (ferror(mainFile))
//     {
//       ThrowError(ERR_INTERNAL);
//       exit(ERR_INTERNAL);
//     }
//   }
//   else if (ungetc(next, mainFile) == EOF)
//   {
//     ThrowError(ERR_INTERNAL);
//     exit(ERR_INTERNAL);
//   }

//   return false; // První lomítko je stále běžný operátor.
// }
static bool
TryComment(FILE *file,
           int *newlines) // funkce pro rozlišování více řádkových komentu
{
  *newlines = 0;

  // První '/' už máme načtené.
  int next = fgetc(file);

  if (next == '/') {
    SkipLineComment(file);
    return true;
  }

  if (next == '*') {
    *newlines = SkipBlockComment(file);

    if (*newlines > 0) {
      // Za víceřádkovým komentářem smějí být
      // do konce řádku pouze bílé znaky.
      int c;

      do {
        c = fgetc(file);
      } while (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f');

      if (c == EOF) {
        if (ferror(file)) {
          ThrowError(ERR_INTERNAL);
          exit(ERR_INTERNAL);
        }
      } else if (c == '\n') {
        // LF zpracuje hlavní část lexeru.
        if (ungetc(c, file) == EOF) {
          ThrowError(ERR_INTERNAL);
          exit(ERR_INTERNAL);
        }
      } else {
        ThrowError(ERR_LEX);
        exit(ERR_LEX);
      }
    }

    return true;
  }

  // Není to komentář: druhý znak vrátíme.
  if (next == EOF) {
    if (ferror(file)) {
      ThrowError(ERR_INTERNAL);
      exit(ERR_INTERNAL);
    }
  } else if (ungetc(next, file) == EOF) {
    ThrowError(ERR_INTERNAL);
    exit(ERR_INTERNAL);
  }

  return false;
}
// Vrátí první znak, který není bílý znak ani součást komentáře.
static int ImportNextChar(FILE *mainFile) {
  int c = fgetc(mainFile);

  while (true) {
    while (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f' ||
           c == '\n') {
      if (c == '\n') {
        lineNumber++;
      }

      c = fgetc(mainFile);
    }

    if (c == EOF && ferror(mainFile)) {
      ThrowError(ERR_INTERNAL);
      exit(ERR_INTERNAL);
    }

    int commentNewlines = 0;

    if (c == '/' && TryComment(mainFile, &commentNewlines)) {
      lineNumber += commentNewlines;
      c = fgetc(mainFile);
      continue;
    }

    return c;
  }
}

// Slovo import už načetla větev pro identifikátory.
static void CheckImport(FILE *mainFile) {
  const char *expected = "ifj26";
  int c = ImportNextChar(mainFile);

  for (int i = 0; expected[i] != '\0'; i++) {
    if (c != expected[i]) {
      ThrowError(ERR_SYNTAX);
      exit(ERR_SYNTAX);
    }

    // Uvnitř ifj26 nepřeskakujeme bílé znaky.
    if (expected[i + 1] != '\0') {
      c = fgetc(mainFile);

      if (c == EOF && ferror(mainFile)) {
        ThrowError(ERR_INTERNAL);
        exit(ERR_INTERNAL);
      }
    }
  }

  if (ImportNextChar(mainFile) != '.') {
    ThrowError(ERR_SYNTAX);
    exit(ERR_SYNTAX);
  }

  if (ImportNextChar(mainFile) != '*') {
    ThrowError(ERR_SYNTAX);
    exit(ERR_SYNTAX);
  }
}
//============================================================================
//============================================================================
FILE *mainFile; // pointer to file
#pragma region Token Init
void TokenInicializator(FILE *file) {
  free(indentStack);

  indentStack = malloc(sizeof *indentStack);
  if (indentStack == NULL) {
    ThrowError(ERR_INTERNAL);
    exit(ERR_INTERNAL);
  }

  indentStack[0] = 0; // základní úroveň: žádné mezery

  lineStart = true;
  indent = 0;
  mainFile = file;
  lineNumber = 1;
  nextChar = EOF;
  parenDepth = 0;
}
#pragma endregion
#pragma region TokenGiver()
Token TokenGiver() {
  int c = 0;                           // proměnná pro znak
  int length = 0;                      // délka 0
  char *buffer = malloc(sizeof(char)); // alokace paměti na 1 znak
  int numOfSpaces = 0;                 // počet mezer na začátku řádku¨

  if (buffer == NULL) // ověřeníé nastavení alokace paměti
  {
    free(buffer);             // uvolni alokaci
    ThrowError(ERR_INTERNAL); // vrať error internal

    returnToken.type = T_ERROR;
    returnToken.line = lineNumber;
    returnToken.val.s = NULL;
    return returnToken;
    // internal error, buffer prob. :thumbs_up:
  }
  // int state = 0;
  // static int _counter = 0;

  else {
#pragma region indent
    if (nextChar != EOF) {
      c = nextChar;
      nextChar = EOF;
    } else {
      c = fgetc(mainFile);
    }

    if (lineStart) {
      numOfSpaces = 0;

      while (true) {
        numOfSpaces = 0;
        bool hasTab = false;

        // Počítáme pouze odsazení před prvním znakem řádku.
        while (c == ' ' || c == '\t') {
          if (c == ' ') {
            numOfSpaces++;
          } else {
            hasTab = true;
          }

          c = fgetc(mainFile);
        }

        // Za sebou může být i několik komentářů.
        while (c == '/') {
          int commentNewlines = 0;

          if (!TryComment(mainFile, &commentNewlines)) {
            break; // Je to dělení, nikoli komentář.
          }

          lineNumber += commentNewlines;
          c = fgetc(mainFile);

          // Mezery za komentářem už nejsou úvodní odsazení.
          while (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f') {
            c = fgetc(mainFile);
          }
        }

        if (c == '\n') {
          // Prázdný nebo čistě komentářový řádek.
          lineNumber++;
          c = fgetc(mainFile);
          continue;
        }

        if (c == EOF) {
          break; // Zpracuje ho stávající větev pro EOF.
        }

        if (hasTab) {
          free(buffer);
          ThrowError(ERR_LEX);
          exit(ERR_LEX);
        }

        // Máme skutečný token: následuje zpracování odsazení.
        break;
      }

      // Prázdný řádek ani EOF nesmí změnit odsazení.
      if (c != '\n' && c != EOF) // TODO: syoyjd
      {
        if (numOfSpaces > indentStack[indent]) {
          // Začíná nová, hlubší úroveň.
          int *temp = realloc(indentStack, (indent + 2) * sizeof *indentStack);

          if (temp == NULL) {
            free(buffer);
            ThrowError(ERR_INTERNAL);
            exit(ERR_INTERNAL);
          }

          indentStack = temp;
          indent++;
          indentStack[indent] = numOfSpaces;
        } else if (numOfSpaces < indentStack[indent]) {
          // Můžeme se vracet přes několik úrovní najednou.
          while (indent > 0 && numOfSpaces < indentStack[indent]) {
            indent--;
          }

          // Odsazení musí odpovídat některé dřívější úrovni.
          if (numOfSpaces != indentStack[indent]) {
            free(buffer);
            ThrowError(ERR_LEX);
            exit(ERR_LEX);
          }
        }
        lineStart = false;
        // Znak uchováme pro další volání TokenGiver().
        if (indent > 0) {
          /* code */

          nextChar = c;

          returnToken.type = T_INDENDATION;
          returnToken.line = lineNumber;
          returnToken.val.i = indent;

          free(buffer);
          return returnToken;
        }
      }
    } else {
      // Mezery a komentáře mezi běžnými tokeny.
      while (true) {
        while (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f') {
          c = fgetc(mainFile);
        }

        int commentNewlines = 0;

        if (c != '/' || !TryComment(mainFile, &commentNewlines)) {
          break;
        }

        lineNumber += commentNewlines;
        c = fgetc(mainFile);

        if (commentNewlines > 0) {
          // TryComment už ověřil, že zde smí být jen LF nebo EOF.
          int endLine = lineNumber;

          if (c == '\n') {
            lineNumber++;
          }
          if (parenDepth == 0) {
            lineStart = true;

            returnToken.type = T_EOL;
            returnToken.line = endLine;
            returnToken.val.s = NULL;

            free(buffer);
            return returnToken;
          }

          // Uvnitř závorek komentář neukončuje logický řádek.
          if (c == '\n') {
            c = fgetc(mainFile);
          }
        }
      }
    }

#pragma endregion

    // Tady pokračuje tvoje stávající kontrola '\n', EOF,
    // identifikátorů, čísel atd.
    if (c == '\n') // konec řádku
    {
      if (parenDepth > 0) {
        returnToken.type = T_SKIP;     // vrať konec řádku
        returnToken.line = lineNumber; // vrať číslo
        returnToken.val.s = NULL;
        return returnToken;
      }
      returnToken.type = T_EOL;      // vrať konec řádku
      returnToken.line = lineNumber; // vrať číslo
      returnToken.val.s = NULL;      // vrať NULL pro value tokenu

      lineStart = true;
      lineNumber++;       // přičti řádek
      free(buffer);       // uvolni alokaci
      return returnToken; //  token return
    }
    if (c == EOF) // konec souboru
    {

      free(buffer); // uvolni alokaci

      if (ferror(mainFile)) // kontroluje chybu v souboru
      {
        ThrowError(ERR_INTERNAL); // vrať internal error
      }

      returnToken.type = T_EOF;
      returnToken.line = lineNumber;
      returnToken.val.s = NULL;
      return returnToken;
    }
    if (c == '@') {
      const char *expected = "main";

      // Znak @ už je načtený. Musí bezprostředně následovat main.
      for (int i = 0; expected[i] != '\0'; i++) {
        c = fgetc(mainFile);

        if (c == EOF && ferror(mainFile)) {
          free(buffer);
          ThrowError(ERR_INTERNAL);
          exit(ERR_INTERNAL);
        }

        if (c != expected[i]) {
          free(buffer);
          ThrowError(ERR_LEX);
          exit(ERR_LEX);
        }
      }

      // Ověříme, že main není začátek delšího identifikátoru.
      c = fgetc(mainFile);

      if (c == EOF && ferror(mainFile)) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
        exit(ERR_INTERNAL);
      }

      if (IsIdentifierPart(c)) {
        free(buffer);
        ThrowError(ERR_LEX);
        exit(ERR_LEX);
      }

      // Znak za anotací patří dalšímu zpracování.
      if (c != EOF && ungetc(c, mainFile) == EOF) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
        exit(ERR_INTERNAL);
      }

      returnToken.type = KW_MAIN;
      returnToken.line = lineNumber;
      returnToken.val.s = NULL;

      free(buffer);
      return returnToken;
    }

#pragma region Identifier //--------------------------------------------------------------------------------------------
    if (IsIdentifierStart(c)) // pokud znak může být začátkem identifikátoru
    {
      do {
        char *temp = realloc(
            buffer, length + 2); // realokace paměti na  znak + ukončující znak

        if (temp == NULL) {
          free(buffer);
          ThrowError(ERR_INTERNAL);
        }

        buffer = temp;            // předání paměti
        buffer[length] = (char)c; // přidání znaku na konec bufferu
        length++;                 // délka ++

        c = fgetc(mainFile); // další znak
      } while (
          IsIdentifierPart(c)); // pokud zna kmůže být součástí identifikátoru

      buffer[length] = '\0'; // na konec stringu přidej ukončující znak
      if (c == EOF && ferror(mainFile)) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }

      if (c != EOF && ungetc(c, mainFile) == EOF) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }

      if (length == 1 &&
          buffer[0] == '_') // error jsut underscore, not on my watch
      {
        free(buffer);
        ThrowError(ERR_LEX);
      }
      returnToken.type = KeywordOrIdentifier(buffer);
      returnToken.line = lineNumber;
      returnToken.val.s = NULL;
      if (returnToken.type == KW_IMPORT) {
        free(buffer);

        CheckImport(mainFile);

        return returnToken;
      }
      if (returnToken.type == T_ID) {
        returnToken.val.s = buffer;
        buffer = NULL;
      }

      free(buffer);
      return returnToken;
    } // konec if (IsIdentifierStart(c))
  }
#pragma endregion
#pragma region String
  if (c == '"') {
    int startLine = lineNumber;
    bool multiline = false;
    bool done = false;
    int quotes = 0;

    buffer[0] = '\0'; // Platný i pro prázdný řetězec.

    c = fgetc(mainFile);

    if (c == '"') {
      c = fgetc(mainFile);

      if (c == '"') {
        // Tři úvodní uvozovky: víceřádkový řetězec.
        multiline = true;
        c = fgetc(mainFile);
      } else {
        // Dvě uvozovky: prázdný řetězec.
        if (c == EOF && ferror(mainFile)) {
          free(buffer);
          ThrowError(ERR_INTERNAL);
          exit(ERR_INTERNAL);
        }

        // Znak za prázdným řetězcem patří dalšímu tokenu.
        if (c != EOF && ungetc(c, mainFile) == EOF) {
          free(buffer);
          ThrowError(ERR_INTERNAL);
          exit(ERR_INTERNAL);
        }

        done = true;
      }
    }

    while (!done) {
      if (c == EOF) {
        ErrorCode error = ferror(mainFile) ? ERR_INTERNAL : ERR_LEX;
        free(buffer);
        ThrowError(error);
        exit(error);
      }

      if (multiline) {
        if (c == '"') {
          quotes++;

          if (quotes == 3) {
            break; // Tři uvozovky ukončují řetězec.
          }

          c = fgetc(mainFile);
          continue;
        }

        // Jedna nebo dvě uvozovky jsou součástí obsahu.
        while (quotes > 0) {
          char *temp = realloc(buffer, length + 2);

          if (temp == NULL) {
            free(buffer);
            ThrowError(ERR_INTERNAL);
            exit(ERR_INTERNAL);
          }

          buffer = temp;
          buffer[length++] = '"';
          buffer[length] = '\0';
          quotes--;
        }

        if (c == '\n') {
          lineNumber++;
        }
      } else {
        if (c == '"') {
          break; // Konec běžného řetězce.
        }

        if (c == '\n') {
          free(buffer);
          ThrowError(ERR_LEX);
          exit(ERR_LEX);
        }

        if (c == '\\') {
          c = fgetc(mainFile);

          if (c == EOF && ferror(mainFile)) {
            free(buffer);
            ThrowError(ERR_INTERNAL);
            exit(ERR_INTERNAL);
          }

          switch (c) {
          case '\\':
            c = '\\';
            break;
          case '"':
            c = '"';
            break;
          case 'n':
            c = '\n';
            break;
          case 'r':
            c = '\r';
            break;
          case 't':
            c = '\t';
            break;
          default:
            free(buffer);
            ThrowError(ERR_LEX);
            exit(ERR_LEX);
          }
        }
      }

      // Uložení jednoho znaku obsahu.
      char *temp = realloc(buffer, length + 2);

      if (temp == NULL) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
        exit(ERR_INTERNAL);
      }

      buffer = temp;
      buffer[length++] = (char)c;
      buffer[length] = '\0';

      c = fgetc(mainFile);
    }

    returnToken.type = T_STRING_LIT;
    returnToken.line = startLine;
    returnToken.val.s = buffer;

    return returnToken;
  }
#pragma endregion

#pragma region Paren ()
  if (c == '(' || c == ')' || c == '[' || c == ']') {
    switch (c) {
    case '(':
      parenDepth++;
      returnToken.type = T_LPAREN;
      break;
    case ')':
      if (parenDepth > 0) {
        parenDepth--;
      }
      returnToken.type = T_RPAREN;
      break;

    case '[':
      ThrowError(ERR_LEX);
      free(buffer);
      break;

    case ']':
      ThrowError(ERR_LEX);
      free(buffer);
      break;
    }

    returnToken.line = lineNumber;
    returnToken.val.s = NULL;

    free(buffer);
    return returnToken;
  }

#pragma endregion
#pragma region Num
  if (IsDigit(c) || c == '.') {
    NumberState numState;
    bool done = false;

    // Počáteční stav nastavíme jen jednou.
    if (c == '.') {
      int next = fgetc(mainFile);

      if (next == EOF && ferror(mainFile)) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }

      // Podívali jsme se dopředu, znak zase vrátíme.
      if (next != EOF && ungetc(next, mainFile) == EOF) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }

      if (!IsDigit(next)) {
        returnToken.type = T_DOT;
        returnToken.line = lineNumber;
        returnToken.val.s = NULL;

        free(buffer);
        return returnToken;
      }

      numState = NUM_DOT;
    } else {
      numState = NUM_INTEGER;
    }

    do {
      // Uložíme aktuální znak do bufferu.
      char *temp = realloc(buffer, length + 2);

      if (temp == NULL) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }

      buffer = temp;
      buffer[length] = (char)c;
      length++;
      buffer[length] = '\0';

      // Načteme další znak a rozhodneme, co s ním.
      c = fgetc(mainFile);

      if (c == EOF && ferror(mainFile)) {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }
#pragma region NumState Switch
      switch (numState) {
#pragma region Int
      case NUM_INTEGER:
        if (IsDigit(c)) {
          numState = NUM_INTEGER;
        } else if (c == '.') {
          numState = NUM_DOT;
        } else if (c == 'e' || c == 'E') {
          numState = NUM_EXP_START;
        } else if (c == '_') {
          numState = NUM_INTEGER_UNDERCORE;
        } else {
          done = true;
        }
        break;
#pragma endregion
#pragma region Num Dot
      case NUM_DOT:
        if (IsDigit(c)) {
          numState = NUM_FRACTION;
        } else {
          free(buffer);
          ThrowError(ERR_LEX);
        }
        break;
#pragma endregion
#pragma region NUM FRACTION
      case NUM_FRACTION:
        if (IsDigit(c)) {
          numState = NUM_FRACTION;
        } else if (c == 'e' || c == 'E') {
          numState = NUM_EXP_START;
        } else if (c == 'd' || c == 'D') {
          numState = NUM_SUFFIX;
        } else if (c == '_') {
          numState = NUM_FRACTION_UNDERCORE;
        } else {
          done = true;
        }
        break;
#pragma endregion
#pragma region Exponent
      case NUM_EXP_START:
        if (c == '+' || c == '-') {
          numState = NUM_EXP_SIGN;
        } else if (IsDigit(c)) {
          numState = NUM_EXP_DIGITS;
        } else {
          free(buffer);
          ThrowError(ERR_LEX);
        }
        break;
#pragma endregion
#pragma region EXP SIGN
      case NUM_EXP_SIGN:
        if (IsDigit(c)) {
          numState = NUM_EXP_DIGITS;
        } else {
          free(buffer);
          ThrowError(ERR_LEX);
        }
        break;
#pragma endregion
#pragma region EXP DIGITS
      case NUM_EXP_DIGITS:
        if (IsDigit(c)) {
          numState = NUM_EXP_DIGITS;
        } else if (c == 'd' || c == 'D') {
          numState = NUM_SUFFIX;
        } else if (c == '_') {
          numState = NUM_EXP_UNDERSCORE;
        } else {
          done = true;
        }
        break;
      case NUM_INTEGER_UNDERCORE:
      case NUM_FRACTION_UNDERCORE:
      case NUM_EXP_UNDERSCORE: {
        if (IsDigit(c)) {
          if (numState == NUM_INTEGER_UNDERCORE) {
            numState = NUM_INTEGER;
          } else if (numState == NUM_FRACTION_UNDERCORE) {
            numState = NUM_FRACTION;
          } else {
            numState = NUM_EXP_DIGITS;
          }
        } else {
          free(buffer);
          ThrowError(ERR_LEX);
        }
        break;
      }
      case NUM_SUFFIX:

        done = true;
        break;
      }
#pragma endregion

    } while (!done);
#pragma endregion
    // Znak, který už nepatří do čísla, vrátíme.
    if (c != EOF && ungetc(c, mainFile) == EOF) {
      free(buffer);
      ThrowError(ERR_INTERNAL);
    }
    // Zpracování čísla
    // -------------------------------------------------------------------------------------
    int wrtieIndex = 0;
    for (int readIndex = 0; readIndex < length; readIndex++) {
      if (buffer[readIndex] != '_') {
        buffer[wrtieIndex] = buffer[readIndex];
        wrtieIndex++;
      }
    }

    length = wrtieIndex;
    buffer[length] = '\0';

    returnToken.line = lineNumber;

    if (numState == NUM_INTEGER) {
      returnToken.type = T_INT_LIT;
      if (length > 1 && buffer[0] == '0') {
        free(buffer);
        ThrowError(ERR_LEX);
      }
      returnToken.val.i = strtoll(buffer, NULL, 10);
    } else {
      if (length > 0 &&
          (buffer[length - 1] == 'd' || buffer[length - 1] == 'D')) {
        length--;
        buffer[length] = '\0';
      }
      returnToken.type = T_DOUBLE_LIT;
      returnToken.val.d = strtod(buffer, NULL);
    }
    free(buffer);
    return returnToken;
  }
#pragma endregion

  /*i nomine patris et filii et spiritus sancti */
  if (c == '=' || c == '<' || c == '>' || c == '+' || c == '-' || c == '*' ||
      c == '/' || c == '!' || c == ',' || c == ':' ||
      c == ';') // podmínka pro specialní znaky, od nich se odvíjí stavy pro
                // tokeny
  {
    ScannerState state = STATE_START;

    while (true) {
#pragma region Switch start_state
      switch (state) {
      case STATE_START:
        switch (c) {
        case '=':
          state = STATE_EQ;
          break;
        case '<':
          state = STATE_LE;
          break;
        case '>':
          state = STATE_GR;
          break;
        case '+':
          state = STATE_PLUS;
          break;
        case '-':
          state = STATE_MINUS;
          break;
        case '*':
          state = STATE_MUL;
          break;
        case '/':
          state = STATE_DIV;
          break;
        case '!':
          state = STATE_NOT;
          break;
        case ',':
          state = STATE_COMMA;
          break;
        case ':':
          state = STATE_COLON;
          break;
        case ';':
          state = STATE_SEMI;
          break;
        default:
          break;
        }
        break;
#pragma endregion

#pragma region Not_State
      case STATE_NOT:
        c = fgetc(mainFile);
        if (c == EOF && ferror(mainFile)) {
          free(buffer);
          ThrowError(ERR_INTERNAL);
        }

        if (c == '=') {
          state = STATE_NEQ;
          break;
        }
        free(buffer);
        ThrowError(ERR_LEX);
        break;
#pragma endregion
#pragma region Rest of it
      case STATE_PLUS:
        returnToken.type = T_PLUS;
        goto operator_done;

      case STATE_MINUS:
        returnToken.type = T_MINUS;
        goto operator_done;

      case STATE_MUL:
        returnToken.type = T_MUL;
        goto operator_done;

      case STATE_NEQ:
        returnToken.type = T_NEQ;
        goto operator_done;
      case STATE_COMMA:
        returnToken.type = T_COMMA;
        goto operator_done;
      case STATE_COLON:
        returnToken.type = T_COLON;
        goto operator_done;
      case STATE_DIV:
        returnToken.type = T_DIV;
        goto operator_done;
      case STATE_SEMI:
        returnToken.type = T_SEMI;
        goto operator_done;
      case STATE_EQ:
      case STATE_LE:
      case STATE_GR:
        c = fgetc(mainFile);

        if (c == EOF && ferror(mainFile)) {
          free(buffer);
          ThrowError(ERR_INTERNAL);
        }

        if (c == '=') {
          if (state == STATE_EQ)
            state = STATE_EQEQ;
          else if (state == STATE_LE)
            state = STATE_LEEQ;
          else
            state = STATE_GREQ;

          break;
        }
        // Znak už nepatří do operátoru, vrátíme ho.
        if (c != EOF && ungetc(c, mainFile) == EOF) {
          free(buffer);
          ThrowError(ERR_INTERNAL);
        }

        if (state == STATE_EQ)
          returnToken.type = T_ASSIGN;
        else if (state == STATE_LE)
          returnToken.type = T_LT;
        else
          returnToken.type = T_GT;

        goto operator_done;

      case STATE_EQEQ:
        returnToken.type = T_EQ;
        goto operator_done;

      case STATE_LEEQ:
        returnToken.type = T_LE;
        goto operator_done;

      case STATE_GREQ:
        returnToken.type = T_GE;
        goto operator_done;
      }
    }

  operator_done:
    returnToken.line = lineNumber;
    returnToken.val.s = NULL;

    free(buffer);
    return returnToken;
  }
  fprintf(stderr, "Nerozpoznany znak: ASCII %d\nLine: %d\nParenDepth: %d\n", c,
          lineNumber, parenDepth); // kontrola
  free(buffer);
  ThrowError(ERR_LEX);

  returnToken.type = T_ERROR;
  returnToken.line = lineNumber;
  returnToken.val.s = NULL;
  return returnToken;
/*i nomine patris et filii et spiritus sancti */
#pragma endregion
#pragma endregion TokenGiver()
}

// Testovacio soubor lex, spustit napsat výraz, zkontrolovat s TokType v
// structure.h zatim z meho jemneho testzovaní funguje
#pragma region Comments by authors
// c je aktuální znak
// automat

/* mensi schizo na zacatek, omluvte me */
/*sorry martine, tady tvoje schizo konci a zacina moje - Ondrej :)*/

/*takze, potrebuju cist znaky ze vstupu, a podle toho rozhodovat co to bude
 * jestli: indentifikator, int double ridicio struktura a podobne*/
/*dej se vule bozi.*/
/*i nomine patris et filii et spiritus sancti */

// idk jestli to bude fungovat tak jak ma, ale smysl to davá, a realně se mi to
// líbí

// update, tohle je jeden z kodů
/*i nomine patris et filii et spiritus sancti */

/* me rupne to, and lets just say, that by to I mean my peanits*/
// og code VV
//  while (c != EOF)
//     {
//       if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122))
//       {
//         length++;
//         char *temp = realloc(buffer, sizeof(char) * length);
//         if (temp == NULL)
//         {
//           // no nic, unlucky^2
//           // ukonci me trapeni^2
//           free(buffer);
//           break;
//         }
//         buffer = temp;
//         buffer[length - 1] = c;
//         c = fgetc(mainFile);
//       }
//       else
//       {
//         for (int i = 0; i < 23; i++)
//         {
//           if (length == Keyword[i].length)
//           {
//             int check_value = strcmp(buffer, Keyword[i].check_array);
//             if (check_value == 0)
//             {
//               returnToken->type = Keyword[i].type;
//             }
//             else
//             {
//               returnToken->type = T_IDENTIFICATOR;
//             }
//           }
//         }
//       }
//     }
#pragma endregion