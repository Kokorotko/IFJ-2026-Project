//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "structure.h"
#include "codeStruct.h"

#ifndef SEMATIX //Nedefinuj soubor znova, pokud uz je definovan
#define SEMATIX

/// @brief Vstupní bod sémantické analýzy.
/// Projde všechny funkce a zkontroluje typy a definice a @main a doplní resultType ve výrazech.
/// Při chybě volá ThrowError(ErrorCode), při úspěchu vrací ERR_OK.
ErrorCode SemanticAnalysis(FuncDef *program);

#endif