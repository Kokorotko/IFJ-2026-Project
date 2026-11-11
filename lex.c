// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "structure.h"
#include "lex.h"

typedef enum // tohle je pro menší větší rovná se a kombinace
{
  STATE_START,
  STATE_EQ,
  STATE_EQEQ,
  STATE_LE,
  STATE_LEEQ,
  STATE_GR,
  STATE_GREQ,
  STATE_PLUS,
  STATE_MINUS,
  STATE_MUL,
  STATE_NOT,
  STATE_NEQ
} ScannerState;

typedef struct Check
{
  char *check_array;
  int length;
  TokType type;
} Keyword_Check;

Token returnToken;

const Keyword_Check Keyword[] = // Keywords list
    {
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
        {"type", 4, KW_TYPE}
    };


bool IsLetter(int c) // is this lil guy a letter, god i hope so
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); // is char?
}

bool IsIdentifierStart(int c) // if this guy is letter, can he start an identifier?
{
  return IsLetter(c) || c == '_'; // can be start to an identifier?
}

bool IsIdentifierPart(int c) // i guess he could, so now he can be anything, yaay
{
  return IsIdentifierStart(c) || (c >= '0' && c <= '9'); // is valid identifier char?
}

TokType KeywordOrIdentifier(const char *text) // vrací řídící strukturu
{
  int count = sizeof Keyword / sizeof Keyword[0];
  for (int i = 0; i < count; i++)
  {
    if (strcmp(text, Keyword[i].check_array) == 0)
    {
      return Keyword[i].type;
    }
  }
  return T_ID;
}
//============================================================================
//============================================================================
FILE *mainFile;     // pointer to file
int lineNumber = 1; // line in code start at 1

void TokenInicializator(FILE *file)
{
  mainFile = file;
}

Token TokenGiver()
{
  int c = 0;
  int length = 0;
  char *buffer = malloc(sizeof(char));
  if (buffer == NULL)
  {
    free(buffer);
    ThrowError(ERR_INTERNAL);
    // internal error, buffer prob. :thumbs_up:
  }
  // int state = 0;
  // static int _counter = 0;
  else
  {    
    c = fgetc(mainFile);

    if (c == EOF)
    {

      free(buffer);

      if (ferror(mainFile))
      {
        ThrowError(ERR_INTERNAL);
      }      
      returnToken.type = T_EOF;
      returnToken.line = lineNumber;
      returnToken.val.s = NULL;
      return returnToken;
    }
    if (IsIdentifierStart(c))
    {
      do
      {
        char *temp = realloc(buffer, length + 2);

        if (temp == NULL)
        {
          free(buffer);
          ThrowError(ERR_INTERNAL);
        }

        buffer = temp;
        buffer[length] = (char)c;
        length++;

        c = fgetc(mainFile);
      } while (IsIdentifierPart(c)); // end of do while

      buffer[length] = '\0'; // last char -> add end char
      if (c == EOF && ferror(mainFile))
      {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }

      if (c != EOF && ungetc(c, mainFile) == EOF)
      {
        free(buffer);
        ThrowError(ERR_INTERNAL);
      }

      if (length == 1 && buffer[0] == '_') // error jsut underscore, not on my watch
      {
        free(buffer);
        ThrowError(ERR_LEX);
      }
      returnToken.type = KeywordOrIdentifier(buffer);
      returnToken.line = lineNumber;
      returnToken.val.s = NULL;

      if (returnToken.type == T_ID)
      {
        returnToken.val.s = buffer;
        buffer = NULL;
      }

      free(buffer);
      return returnToken;
    } // konec if (IsIdentifierStart(c))
  }
  /*i nomine patris et filii et spiritus sancti */
  if (c == '=' || c == '<' || c == '>' || c == '+' || c == '-' || c == '*' || c == '!')
  {
    ScannerState state = STATE_START;

    while (true)
    {
      switch (state)
      {
      case STATE_START:
        if (c == '=')
          state = STATE_EQ;
        else if (c == '<')
          state = STATE_LE;
        else if (c == '>')
          state = STATE_GR;
        else if (c == '+')
          state = STATE_PLUS;
        else if (c == '-')
          state = STATE_MINUS;
        else if (c == '*')
          state = STATE_MUL;
        else if (c == '!')
          state = STATE_NOT;
        break;
      case STATE_NOT:
        c = fgetc(mainFile);
        if (c == EOF && ferror(mainFile))
        {
          free(buffer);
          ThrowError(ERR_INTERNAL);
        }

        if (c == '=')
        {
          state = STATE_NEQ;
          break;
        }
        free(buffer);
        ThrowError(ERR_LEX);
        break;
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

      case STATE_EQ:
      case STATE_LE:
      case STATE_GR:
        c = fgetc(mainFile);

        if (c == EOF && ferror(mainFile))
        {
          free(buffer);
          ThrowError(ERR_INTERNAL);
        }

        if (c == '=')
        {
          if (state == STATE_EQ)
            state = STATE_EQEQ;
          else if (state == STATE_LE)
            state = STATE_LEEQ;
          else
            state = STATE_GREQ;

          break;
        }

        // Znak už nepatří do operátoru, vrátíme ho.
        if (c != EOF && ungetc(c, mainFile) == EOF)
        {
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
  fprintf(stderr, "Nerozpoznany znak: ASCII %d\n", c); // kontrola
  free(buffer);
  return returnToken;
  /*i nomine patris et filii et spiritus sancti */
}

// Testovacio soubor lex, spustit napsat výraz, zkontrolovat s TokType v structure.h
// zatim z meho jemneho testzovaní funguje

#pragma region Comments by authors
// c je aktuální znak
// automat

/* mensi schizo na zacatek, omluvte me */
/*sorry martine, tady tvoje schizo konci a zacina moje - Ondrej :)*/

/*takze, potrebuju cist znaky ze vstupu, a podle toho rozhodovat co to bude jestli: indentifikator, int double ridicio struktura a podobne*/
/*dej se vule bozi.*/
/*i nomine patris et filii et spiritus sancti */

// idk jestli to bude fungovat tak jak ma, ale smysl to davá, a realně se mi to líbí

// update, tohle je jeden z kodů
/*i nomine patris et filii et spiritus sancti */
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