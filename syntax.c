//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef SYNTAX_C //Nedefinuj soubor znova, pokud uz je definovan
#define SYNTAX_C

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "structure.h"
#include "syntax.h"
#include "codeStruct.h"


static bool previousWasOperand = false;
static bool foundError = false;
static int parenDepth = 0;

//PROCEDURE TABLE
#pragma region

// akce
#define S 's'   // '<'  shift (vlož marker + token)
#define R 'r'   // '>'  reduce
#define E 'e'   // '='  shift bez markeru
#define X 'x'   // '·'  chyba
#define F 'f'   // hotovo ($ vs $)
#define $ '$'   // End symbol
#define P_TABLE_SIZE 9
#define TABLE_MAX_RULE_SIZE 3

/// @brief LL Tabulka
//AI: Vygeneruj mi 2D tabulku v C podle téhle SA tabulky (*SA tabulka z dokumentace)
static const char p_Table[P_TABLE_SIZE][P_TABLE_SIZE] = {
//            -u  */  +-  rel eq  (   )   i   $
/* -u   */  { S,  R,  R,  R,  R,  S,  R,  S,  R },
/* * /  */  { S,  R,  R,  R,  R,  S,  R,  S,  R },
/* +-   */  { S,  S,  R,  R,  R,  S,  R,  S,  R },
/* rel  */  { S,  S,  S,  X,  R,  S,  R,  S,  R },
/* ==   */  { S,  S,  S,  S,  X,  S,  R,  S,  R },
/* (    */  { S,  S,  S,  S,  S,  S,  E,  S,  X },
/* )    */  { X,  R,  R,  R,  R,  X,  R,  X,  R },
/* i    */  { X,  R,  R,  R,  R,  X,  R,  X,  R },
/* $    */  { S,  S,  S,  S,  S,  S,  X,  S,  F },
//S == Shift, R == Reduce, E == Equals, X == Unreachable syntax, F == Finito
};

#pragma endregion

static Token token;
static Token nextToken;
bool tokenPeaked = false;

void Next()
{
    if (tokenPeaked)
    {
        token = nextToken;
        tokenPeaked = false;
    }
    else
    {
        token = TokenGiver();
    }
}

void Peek()
{
    if (!tokenPeaked)
    {
        tokenPeaked = true;
        nextToken = TokenGiver();
    }
}

/// @brief Rozhodne, jestli další token je požadovaný token. Může hodit syntax_error
/// @param type Předpokládaný token
void Expect(TokType type)
{
    if (token.type != type)
    {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
    }
    Next();
}

/// @brief Rozhodne, jestli další token je požadovaný token typu data (INT, DOUBLE, STRING, UNIT).
void ExpectDataType()
{
    switch(token.type)
    {
        case KW_INT:
        {
            Expect(KW_INT);
             return;
        }
        case KW_DOUBLE:
        {
            Expect(KW_DOUBLE);
            return;
        }
        case KW_STRING:
        {
            Expect(KW_STRING);
            return;
        }
        case KW_UNIT:
        {
            Expect(KW_UNIT);
            return; 
        }
        default: 
        {
            ErrorHandlingSyntax(ERR_SYNTAX, token);
            return;
        }
    }
}

/// @brief 
/// @param indendationLevel current level of backspaces for blocks
void ParseIf(bool checkIndendation, int indendationLevel) 
{
    //if (sum) then 
    //  BLOCK 
    //else
    //  BLOCK
    //CheckIndendation(check_Indendation, indendationLevel);
    Expect(KW_IF);
    ParseExpression(); //sum
    Expect(KW_THEN);
    ParseLine(checkIndendation, indendationLevel + 1); //BLOCK
    CheckIndendation(checkIndendation, indendationLevel);
    Expect(KW_ELSE);
    ParseLine(checkIndendation, indendationLevel + 1); //BLOCK
}

/// @brief Parses a function(without the function ID name)
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
void ParseFunction(bool checkIndendation, int indendationLevel)
{
    //Foo(sum)
    //  BLOCK
    //  RETURN
    CheckIndendation(checkIndendation, indendationLevel);
    Expect(T_LPAREN);
    Peek();
    if (nextToken.type == T_ID)
    {
         ParseCall(); //sum
    }
    ParseLine(checkIndendation, indendationLevel + 1); //BLOCK
    Expect(KW_RETURN);
}

Expression *ParseCall()
{
    //ID(x,y,name,c * 2)
    Expression *e = NewExpression(E_CALL);
    e->data.call.name = CopyName();
    Next();
    Expect(T_LPAREN);
    e->data.call.argc = 0;
    while(nextToken.type != T_RPAREN) //empty == ID()
    {
        e->data.call.argc++; //up argument count
        e->data.call.args = realloc(e->data.call.args, e->data.call.argc * sizeof(Expression)); //argc za
        MallocChecker(e->data.call.args);
        e->data.call.args[e->data.call.argc - 1] = ParseExpression(); //argc jede 1+ a pole 0+
        if (token.type == T_COMMA) 
        {
            Next(); //Johne, máme více argumentů na zpracování
        }
        else break; //Tot vše
    }
    Expect(T_LPAREN);
}

/// @brief Parses a while cycle(without the function ID name)
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
void ParseWhile(bool check_Indendation, int indendationLevel)
{
    //While (sum) do
    //  BLOCK
    //CheckIndendation(check_Indendation, indendationLevel);
    Expect(KW_WHILE);
    ParseExpression(); //sum
    Expect(KW_DO);
    Expect(T_EOL);
    ParseLine(check_Indendation, indendationLevel + 1); //BLOCK
}

/// @brief Parses a variable
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
void ParseVariable(bool check_Indendation, int indendationLevel)
{
    //var ID =
    if (token.type == KW_VAR)
    {
        Expect(KW_VAR);
    }
    else if (token.type == KW_VAL)
    {
        Expect(KW_VAL);
    }
    else 
    {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
    }

    Expect(T_ID);
    if (token.type == T_COLON) //přiřazení var x: INT = *neco*
    {
        Expect(T_COLON);
        ExpectDataType();
        Expect(T_ASSIGN);
        ParseExpressionOrCall(check_Indendation, indendationLevel);
    }
    else if (token.type == T_ASSIGN) //Assing x = *neco*
    {
        Expect(T_ASSIGN);
        ParseExpressionOrCall(check_Indendation, indendationLevel);
    }
    else ErrorHandlingSyntax(ERR_SYNTAX, token);

}


/// @brief Decides if the next token points to function or expression
/// @param check_Indendation 
/// @param indendationLevel 
void ParseExpressionOrCall(bool check_Indendation, int indendationLevel)
{
    //= *neco*
    if (token.type == T_ID)
    {
        Peek();
        if (nextToken.type == T_LPAREN)
        {
            ParseFunction(check_Indendation, indendationLevel);
            return;
        }
    }
    ParseExpression();
}

/// @brief Parses expression
Expression *ParseExpression()
{
    //reset
    previousWasOperand = false;
    parenDepth = 0;
    //set up stack
    Stack *stack = malloc(sizeof(Stack));
    MallocChecker(stack);
    StackInit(stack);

    //set up end token
    Token endToken;
    endToken.type = T_ENDSYMBOL;
    endToken.line = 0;
    StackItem itemEndToken;
    itemEndToken.token = endToken;
    itemEndToken.line = token.line;
    StackPush(stack, itemEndToken);

    Token middleToken;
    middleToken.type = T_MIDDLESYMBOL;
    StackItem itemMiddleToken;
    itemMiddleToken.token = middleToken;

    while (true)
    {
        switch(PTableFinder(stack, token.type))
        {
            case S: //Push with <
            {
                StackItem *top = malloc(sizeof(StackItem)); //Vrchol zasobniku
                MallocChecker(top);
                StackItem item;
                item.token = token; //Kopie Tokenu
                StackTop(stack, &top);

                if (item.token.type == T_MINUS && !previousWasOperand)
                {
                    item.token.type = P_UMINUS; // Zaměnit za unární mínus
                }
                if (top->token.type == P_NONTERM) //pokud je na vrchu zasobniku E
                {
                    StackPopReturn(stack, top); //grab the E
                    StackPush(stack, itemMiddleToken); //<
                    StackPush(stack, *top); //Push E back
                }
                else 
                {
                    StackPush(stack, itemMiddleToken); //<
                }
                StackPush(stack, item); //Token
                UpdateState(token.type);
                Next();
                break;
            }
            case R: //Reduce (Pop)
            {
                StackItem items[TABLE_MAX_RULE_SIZE]; //without the middle symbol
                int lenght = 0;

                StackItem *temp = malloc(sizeof(StackItem));
                MallocChecker(temp);
                StackPopReturn(stack, temp);
                while (temp->token.type != T_MIDDLESYMBOL)
                {
                    if (lenght >= TABLE_MAX_RULE_SIZE) ErrorHandlingSyntax(ERR_SYNTAX, token);
                    if (temp->token.type == T_ENDSYMBOL) ErrorHandlingSyntax(ERR_SYNTAX, token);
                    items[lenght] = *temp;
                    lenght++;
                    StackPopReturn(stack, temp);
                }
                StackItem *result = malloc(sizeof(StackItem));
                MallocChecker(result);
                if (!ReduceByRule(items, lenght, result)) 
                {
                    ErrorHandlingSyntax(ERR_SYNTAX, token);
                }
                if (result != NULL)
                {
                    StackPush(stack, *result); //always E
                }

                free(result);
                free(temp);
                break;
            }
            case E: //Push without <
            {
                StackItem itemToken;
                itemToken.token = token;
                StackPush(stack, itemToken); //push el Token
                UpdateState(token.type);
                Next();
                break;
            }
            case F:
            {
                if (parenDepth != 0) ErrorHandlingSyntax(ERR_SYNTAX, token); //Not every ( has its )
                StackItem *temp = malloc(sizeof(StackItem));
                MallocChecker(temp);
                if (StackIsEmpty(stack)) ErrorHandlingSyntax(ERR_SYNTAX, token);
                StackPopReturn(stack, temp);
                if (temp->token.type != P_NONTERM) ErrorHandlingSyntax(ERR_SYNTAX, token); //Didnt end on $ E
                StackPopReturn(stack, temp);
                if (temp->token.type != T_ENDSYMBOL) ErrorHandlingSyntax(ERR_SYNTAX, token); //Didnt end on $
                free(temp);
                StackFree(stack);
                free(middleToken);
                free(endToken);
                return; //found the end
            }
            case X: //syntax error
            {
                ErrorHandlingSyntax(ERR_SYNTAX, token);
                StackFree(stack);
                free(middleToken);
                free(endToken);
                return;
            }
            default:
            {
                printf("Procedure table error: Not alocated space in the 2D table");
                ErrorHandlingSyntax(ERR_INTERNAL, token);
                StackFree(stack);
                free(middleToken);
                free(endToken);
                return;
            }
        }
    }
}

void UpdateState(TokType token)
{
    previousWasOperand = (token == T_INT_LIT    ||
                          token == T_DOUBLE_LIT ||
                          token == T_STRING_LIT || 
                          token == T_ID ||
                          token == T_RPAREN);
    if (token == T_LPAREN) parenDepth++;
    else if (token == T_RPAREN) parenDepth--;
}

Expression *ExpressionTransform(StackItem *item)
{
    {
    Expression *e;
    switch (item->token.type)
    {
        case T_ID: 
        {
            e = NewExpr(E_VAR); 
            e->data.name = strdup(item->token.val.s); //IDname 
            break;
        }
        case T_INT_LIT:
        {
            e = NewExpr(E_INT);
            e->data.intValue = item->token.val.i;
            break;
        }
        case T_DOUBLE_LIT:    
        {
            e = NewExpr(E_DOUBLE);
            e->data.doubleValue = item->token.val.d;
            break;
        }
        case T_STRING_LIT:   
        {
             e = NewExpr(E_STR);
             e->data.stringValue = strdup(item->token.val.s); 
             break;
        }
        default: return NULL;
    }
    return e;
}
}

bool ReduceByRule(StackItem *items, int lenght, StackItem *out)
{
    out->token.type = P_NONTERM;
    out->e = NULL;

    // E → i
    if (lenght == 1 && IsTerm(items[0].token.type)) 
    { 
        out->token.type = P_NONTERM;
        out->e = ExpressionTransform(&items[0]);
        return true;
    }

    // E → ( E )
    if (lenght == 3 && items[2].token.type == T_LPAREN && items[1].token.type == P_NONTERM && items[0].token.type == T_RPAREN)
        {
            out->token.type = P_NONTERM;
            return true; 
        }

    // E → E op E
    if (lenght == 3 && items[2].token.type == P_NONTERM && IsBinaryOperator(items[1].token.type) && items[0].token.type == P_NONTERM)
        { 
            out->e->type = E_BIN;
            out->token.type = P_NONTERM;
            out->e->data.binary.left = items[2].e;
            out->e->data.binary.right = items[0].e;
            return true; 
        }

    // E → -u E
    if (lenght == 2 && items[1].token.type == P_UMINUS && items[0].token.type == P_NONTERM)
        { 
            if (items[1].token.type != P_UMINUS || items[0].token.type != P_NONTERM) //Kontrola
            {
                return false;
            }
            out->e->type = E_UNARY;
            out->e->operation = T_MINUS;
            out->e->data.unitary = items[0].token.type; 
            out->token.type = P_NONTERM;
            return true; 
        }
    return false; //Rule not found
}

bool IsBinaryOperator(TokType token)
{
    switch (token)
    {
        case T_PLUS: case T_MINUS: case T_MUL: case T_DIV:
        case T_LT: case T_LE: case T_GT: case T_GE:
        case T_EQ: case T_NEQ:
            return true;
        default:
            return false;
    }
}

bool IsTerm(TokType token)
{
    switch (token)
    {
        case T_INT_LIT: case T_DOUBLE_LIT: case T_STRING_LIT: case T_ID:
            return true;
        default:
            return false;
    }
}

/// @brief Returns the procedure table "word" based on stack and token
/// @param stack //Pointer to stack
/// @param token_Input //Token
/// @return 
char PTableFinder(Stack *stack, TokType token_Input)
{
    Token *stackToken = malloc(sizeof(Token));
    MallocChecker(stackToken);
    StackTop(stack, &stackToken);
    return p_Table[PTableIndexFinder_3000(stackToken->type, false)][PTableIndexFinder_3000(token_Input, true)];
}

/// @brief Returns the index for the procedure table finder
/// @param tokenType TokenType
/// @param stack Pointer to stack
/// @return 
int PTableIndexFinder_3000(TokType tokenType, bool isIndexInput)
{
    switch (tokenType)
    {
        case T_MINUS:
        {
            if (isIndexInput) //Na inputu je potřeba rozlišit uminus/minus
            {
                if (previousWasOperand) return 2; //Bi minus
                return 0; //Uni Minus
            }
            return 2; //Na stacku se už rozlišuje uminus/minus
        }
        case P_UMINUS: return 0;
        case T_PLUS: return 2;
        case T_MUL: return 1;
        case T_DIV: return 1;
        case T_LT: return 3;
        case T_LE: return 3;
        case T_GT: return 3;
        case T_GE:return 3;
        case T_EQ: return 4;
        case T_NEQ: return 4;
        case T_LPAREN: return 5;
        case T_RPAREN: return 6;
        //Operand
        case T_INT_LIT: return 7;
        case T_DOUBLE_LIT: return 7;
        case T_STRING_LIT: return 7;
        case T_ID: return 7;
        //Operand end
        case T_ENDSYMBOL: return 8;
        default: return 8; //(EOF, then and other), return endSymbol
    }
}

/// @brief Parses a line of code
/// @param checkIndendation If indendation should be checked
/// @param indendationLevel Current level of backspaces for blocks
void ParseLine(bool check_Indendation, int indendationLevel)
{
    CheckIndendation(check_Indendation, indendationLevel);
    while (token.type != T_EOL)
    {
        switch (token.type) 
        {
            case KW_VAR: case KW_VAL: 
            {
                ParseVariable(check_Indendation, indendationLevel);  
                break;
            }
            case KW_IF:               
            {
                ParseIf(check_Indendation, indendationLevel);       
                break;
            }
            case KW_WHILE:            
            {
                ParseWhile(check_Indendation, indendationLevel);    
                break;
            }
            case KW_RETURN:           
            {
                ParseReturn(check_Indendation, indendationLevel); 
                break;
            }
            case T_ID:     
            {
                ParseExpressionOrCall(check_Indendation, indendationLevel);
                break;
            }
            default:                 
            {
                ErrorHandlingSyntax(ERR_SYNTAX, token);
                break; 
            }
        }
    }
    Expect(T_EOL); //sežere EOL na konci bloku
}

void ParseReturn(bool check_Indendation, int indendationLevel)
{
    Expect(KW_RETURN);
    switch (token.type)
    {
        //Return
        case T_EOL:
        case T_EOF:          
            break;
        //Return *sum*
        default:
            ParseExpressionOrCall(check_Indendation, indendationLevel);
            break;
    }
}

/// @brief Checks indendation level
/// @param check If indendation should be checked
/// @param indendationLevel Current level of backspaces for blocks
void CheckIndendation(bool check, int indendationLevel)
{
    if (check || indendationLevel > 0)
    {
        for (int i = 0; i < indendationLevel; i++)
        {
            Expect(T_INDENDATION);
        }
    }
    return;
}

bool SyntaxChecker()
{
    Next(); //Very first token
    Expect(KW_IMPORT); //import ifj26.*

    while (token.type != T_EOF)
    {
        ParseLine(true, 0);
    }
    return foundError;
}

Expression *NewExpression(ExpressionEnum type)
{
    Expression *ex = malloc(sizeof(Expression));
    ex->line = token.line;
    ex->type = type;
    return ex;
}

Statement *NewStatement(StatementEnum type)
{
    Statement *st = malloc(sizeof(Statement));
    st->line = token.line;
    st->type = type;
    return st;
}

Expression *MakeBin(TokType operation, Expression *left, Expression *right)
{
    Expression *e = NewExpression(E_BIN);
    e->operation = operation;
    e->data.binary.left = left;
    e->data.binary.right = right;
    return e;
}

static void BlockAppend(Block *b, Statement *s) 
{ 
    if (b->last) 
    {
        b->last->next = s; 
    }    
    else 
    {
        b->first = s;
    }
    b->last = s; 
}

/// @brief Parses from DataType Token to DataType Sematix
/// @return 
DataType ParseType() 
{
    DataType type = DT_NONE;
    switch (token.type) 
    {
        case KW_INT: 
            type = DT_INT;
            break;   
        case KW_DOUBLE: 
            type = DT_DOUBLE; 
            break;
        case KW_STRING: 
            type = DT_STRING; 
            break;  
        case KW_UNIT: 
            type = DT_UNIT; 
            break;
        default: SyntaxError();
    }
    Next(); 
    return type;
}

/// @brief Okopíruje název z tokenu
/// @return 
char *CopyName()
{
    return strdup(token.val);
}


#endif