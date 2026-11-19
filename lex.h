// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef LEX_H
#define LEX_H

#include <stdbool.h>
#include <stdio.h>

#include "structure.h"

bool IsLetter(int c);
bool IsIdentifierStart(int c);
bool IsIdentifierPart(int c);
TokType KeywordOrIdentifier(const char *text);
void TokenInicializator(FILE *file);
Token TokenGiver();

#endif
