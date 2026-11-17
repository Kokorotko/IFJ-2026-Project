#include <stdio.h>
#include <stdlib.h>
#include "../../structure.h"
#include "../../lex.h"

int main(void)
{
    TokenInicializator(stdin);

    while (1)
    {
        Token token = TokenGiver();

        printf("Typ: %d", (int)token.type);

        if (token.type == T_ID || token.type == T_STRING_LIT)
        {
            printf(", text: %s", token.val.s);
            free(token.val.s);
        }
        else if (token.type == T_INT_LIT)
        {
            printf(", hodnota: %lld", token.val.i);
        }
        else if (token.type == T_DOUBLE_LIT)
        {
            printf(", hodnota: %.17g", token.val.d);
        }
        else if (token.type == T_INDENDATION)
        {
            printf(", indent: %lld", token.val.i);
        }

        putchar('\n');

        if (token.type == T_ERROR)
        {
            return EXIT_FAILURE;
        }

        if (token.type == T_EOF)
        {
            break;
        }
    }

    return EXIT_SUCCESS;
}