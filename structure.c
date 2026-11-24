

#include "structure.h"
#include <stdio.h>
#include <stdlib.h>

void ErrorHandlingSyntax(ErrorCode errorCode, Token token) {
  fprintf(stderr, "Error on line: %i\nToken Type: %i\n", token.line,
          token.type);
  exit(errorCode);
}

void ThrowError(ErrorCode errorCode) { exit(errorCode); }

void MallocChecker(void *pointer) {
  if (pointer == NULL) {
    free(pointer);
    ThrowError(ERR_INTERNAL);
  }
}