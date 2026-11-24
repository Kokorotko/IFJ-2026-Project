// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

// include
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "codeGen.h"
#include "lex.h"
#include "sematix.h"
#include "syntax.h"

// #include "sematix.h"
// #include "codeGen.h"

int main(/*int argc, char *argv[]*/) {
  TokenInicializator(stdin); // Standard Input
  FuncDef *program;          // AST Strom
  SyntaxChecker(&program);
  int errorCode = SemanticAnalysis(program);
  if (errorCode != 0) {
    ThrowError(errorCode); // returns
  }
  /*
  FILE *output = fopen("out.code", "w"); //debug
  if (output == NULL)
  {
    ThrowError(ERR_INTERNAL);
  }
  */
  errorCode = GenProgram(program, stdout); // stdout pro vysledek
  if (errorCode != 0) {
    ThrowError(errorCode); // returns
  }
  // fclose(output);
  return 0;
}