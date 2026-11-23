// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>

#include "codeStruct.h"

/// @brief Vygeneruje IFJcode26 pro celý program
/// @param funcs Seznam funkcí po sémantice
/// @param output Kam se kód vypíše (typicky stdout)
/// @return 0 = OK, 99 = interní chyba překladače
int GenProgram(FuncDef *program, FILE *output);

#endif
