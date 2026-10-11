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

static FILE *mainFile;
static int lineNumber = 0;

void TokenInicializator(FILE *file)
{
  mainFile = file;
}

Token TokenGiver()
{
  Token returnToken;
  char *buffer;
  int c;
  while ((c = fgetc(mainFile)) != EOF)
  {
      // c je aktuální znak
      //automat
      
      //if ()¨
      {
        returnToken.line = 0;
        returnToken.type = T_EOF;
        returnToken.val.i = 1000;
      }
  
  }
  return returnToken;
}

#endif