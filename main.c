// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

// include
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "syntax.h"

// #include "sematix.h"
// #include "codeGen.h"

int main(/*int argc, char *argv[]*/) {
  TokenInicializator(stdin); // Standard Input
  FuncDef *program;          // AST Strom
  SyntaxChecker(&program);
  // Sematix check
  // Semathics();
  // GenerateCode();
  return 0;
}