//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef LEX //Nedefinuj soubor znova, pokud uz je definovan
#define LEX

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "structure.h"

typedef struct Check
{
    char *check_array;
    int length;
    TokType type;
} Keyword_Check;

static const Keyword_Check Keyword[] =
{
    { "def", 3, KW_DEF},
    { "do", 2, KW_DO },
    { "Double", 6, KW_DOUBLE },
    { "else", 4, KW_ELSE },
    { "if", 2, KW_IF },
    { "import", 6, KW_IMPORT },
    { "Int", 3, KW_INT },
    { "return", 6, KW_RETURN },
    { "String", 6, KW_STRING },
    { "then", 4, KW_THEN },
    { "Unit", 4, KW_UNIT },
    { "val", 3, KW_VAL },
    { "var", 3, KW_VAR },
    { "while", 5, KW_WHILE },
    { "Boolean", 7, KW_BOOL },
    { "case", 4, KW_CASE},
    { "class", 5, KW_CLASS},
    { "extension", 9, KW_EXTENSION},
    { "for", 3, KW_FOR },
    { "List", 4, KW_LIST},
    { "match", 5, KW_MATCH},
    { "type", 4, KW_TYPE }
};

static FILE *mainFile;
static int lineNumber = 0;

void TokenInicializator(FILE *file)
{
  mainFile = file;
}

void TokenGiver(Token *returnToken)
{
  int c;
  int length = 0;
  char *buffer = malloc(sizeof(char));
  if(buffer == NULL){
    // no nic, unlucky
    // ukonci me trapeni
  }
  //int state = 0;
  //static int _counter = 0;
  else{
    c = fgetc(mainFile);
    while (c != EOF)
    {
      // c je aktuální znak
      //automat

      /* mensi schizo na zacatek, omluvte me */
      /*switch(state){
      case (0): //wow, zacatek
        if(c == '_'){
          _counter++;
          state = 1;
        }
        else if(c == 'I') state = 3; // mozna Int
        else if(c == 'D') state = 8; // mozna Double
        else if(c == 'S') state = 15; // mozna String
        break;
      case(1):
        if(_counter > 1){
          state = 0;
        }
        else if(c == ' '){
          // nejaka mrdka z rozsireni, neresim  jeste, fuck you
          // nebo nazev :)
          _counter = 0;
        }else if(c == '_'){
          _counter++;
          state = 0;
        }else{
          // mozna nazev
          _counter = 0;
        }
        break;
      case(2):
        // samotné '_', něco v rozšíření, fuck do I know
        break;
      case(3):  // 'I'
        if(c == 'n') state = 4;
        else state = 7;
        break;
      case(4):  // 'In'
        if(c == 't') state = 5;
        else state = 7;
        break;
      case(5):  // 'Int'
        if(c == ' ') state = 6;
        else state = 7;
        break;
      case(6):  // wow, on to je actually Int
        returnToken.type = T_INT;
        break;
      case(7):  // fuck...
        returnToken.type = T_IDENTIFICATOR;
        break;
      case(8):  // 'D'
        if(c == 'o') state = 9;
        else state = 7;
        break;
      case(9):  // 'Do'
        if(c == 'u') state = 10;
        else state = 7;
        break;
      case(10): // 'Dou'
        if(c == 'b') state = 11;
        else state = 7;
        break;
      case(11): // 'Doub'
        if(c == 'l') state = 12;
        else state = 7;
        break;
      case(12): // 'Doubl'
        if(c == 'e') state = 13;
        else state = 7;
        break;
      case(13): // 'Double'
        if(c == ' ') state = 14;
        else state = 7;
        break;
      case(14): // wow, on to je actually Double
        returnToken.type = T_DOUBLE_LIT
        break;
      case(15): // 'S'
        if(c == 't') state = 16;
        else state = 7;
        break;
      case(16): // 'St'
        if(c == 'r') state = 17;
        else state = 7;
        break;
      case(17): // 'Str'
        if(c == 'i') state = 18;
        else state = 7;
        break;
      case(18): // 'Stri'
        if(c == 'n') state = 19;
        else state = 7;
        break;
      case(19): // */

      if((c >= 65 && c <= 90) || (c >= 97 && c <= 122)){
        length++;
        char *temp = realloc(buffer, sizeof(char) * length);
        if(temp == NULL){
          // no nic, unlucky^2
          // ukonci me trapeni^2
          free(buffer);
          break;
        }
        buffer = temp;
        buffer[length-1] = c;
        c = fgetc(mainFile);
      }else{
        for(int i = 0; i < 23; i++){
          if(length == Keyword[i].length){
            int check_value = strcmp(buffer, Keyword[i].check_array);
            if(check_value == 0){
              returnToken->type = Keyword[i].type;
            }else{
              returnToken->type = T_IDENTIFICATOR;
            }
          }
        }
      }
      
      
    }
  }

  free(buffer);
}

#endif