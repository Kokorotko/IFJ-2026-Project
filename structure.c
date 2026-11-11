

#include <stdio.h>
#include <stdlib.h>
#include "structure.h"

void ErrorHandlingSyntax(ErrorCode errorCode, Token token)
{
    fprintf(stderr, "Error on line: %i", token.line);
    exit(errorCode);
}

void ThrowError(ErrorCode errorCode)
{
    exit(errorCode);
}

void MallocChecker(void *pointer)
{
    if (pointer == NULL)
    {
        free(pointer);
        ThrowError(ERR_INTERNAL);
    }
}