//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "structure.h"
#include "lex.h"
#include "syntax.h"

//#include "sematix.h"
//#include "codeGen.h"

int main(int argc, char *argv[]) 
{
  if (argc <= 1)
  {
    return false; //Something idk
  }
  FILE *file = fopen(argv[2], "r");
  if (file == NULL)
  {
    return ERR_REDEFINED; //reading file error
  }
  TokenInicializator(file);
  FuncDef *program;
  int errorCode = SyntaxChecker(&program);
  if (errorCode != 0)
  {
     ThrowError(errorCode);
  }
  //Sematix check
  //Semathics();
  //GenerateCode();
  return 0; 
}