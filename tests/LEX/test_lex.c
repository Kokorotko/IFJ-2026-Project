#include <stdio.h>
#include <stdlib.h>
#include "../../structure.h"
#include "../../lex.h"

void TokenInicializator(FILE *file);
ErrorCode TokenGiver(Token *token);

int main(void)
{
    TokenInicializator(stdin);

    while (1)
    {
        Token token = {0};
        ErrorCode error = TokenGiver(&token);

        if (error != ERR_OK)
        {
            fprintf(stderr, "Chyba: %d\n", (int)error);
            return error;
        }

        printf("Typ: %d", (int)token.type);

        if (token.type == T_IDENTIFICATOR)
        {
            printf(", text: %s", token.val.s);
            free(token.val.s);
        }

        putchar('\n');

        if (token.type == T_EOF)
        {
            break;
        }
    }

    return 0;
}