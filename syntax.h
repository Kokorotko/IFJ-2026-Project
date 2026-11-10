//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef SYNTAX_H //Nedefinuj soubor znova, pokud uz je definovan
#define SYNTAX_H

// include
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

#include "lex.h"
#include "structure.h"
#include "linkedList.h"
#include "stack.h"

void Next();
void Peek();
void Error_Handling_Syntax(ErrorCode errorCode);
void Expect(int type);
void ExpectDataType();
void Parse_If(bool check_Indendation, int indendation_Level);
void ParseFunction(bool check_Indendation, int indendation_Level);
void Parse_While(bool check_Indendation, int indendation_Level);
void Parse_Variable(bool check_Indendation, int indendation_Level);
void Parse_ExpressionOrCall(bool check_Indendation, int indendation_Level);
void Parse_Expression();
static void UpdateState(TokType token);
bool ReduceByRule(StackItem *items, int lenght, StackItem *out);
char PTable_Finder(Stack *stack, TokType token_Input);
int PTable_Index_Finder_3000(TokType token_Type, Stack *stack);
void Parse_Block(bool check_Indendation, int indendation_Level);
void CheckIndendation(bool check, int indendation_Level);
bool Syntax_Checker();
void Parse_Return(bool check_Indendation, int indendation_Level);
bool Is_Binary_Operator(TokType token);
bool Is_Term(TokType token);

static bool previous_Was_Operand = false;
static bool found_Error = false;
static int paren_Depth = 0;

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
#define TableMaxRuleSize 3

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
static int NumberOfIndendations = 0;

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

void Error_Handling_Syntax(ErrorCode errorCode)
{
    found_Error = true;
    printf("Error on line: %i", token.line);
    exit(errorCode);
}

/// @brief Rozhodne, jestli další token je požadovaný token. Může hodit syntax_error
/// @param type Předpokládaný token
void Expect(int type)
{
    if (token.type != type)
    {
        Error_Handling_Syntax(ERR_SYNTAX);
    }
    Next();
}

/// @brief Rozhodne, jestli další token je požadovaný token typu data (INT, DOUBLE, STRING). Může hodit syntax_error
void ExpectDataType()
{
    switch(token.type)
    {
        case KW_INT:
        {
                return;
        }
        case KW_DOUBLE:
        {
            return;
        }
        case KW_STRING:
        {
            return;
        }
        default: 
        {
            Error_Handling_Syntax(ERR_SYNTAX);
        }
    }
}

/// @brief 
/// @param indendation_Level current level of backspaces for blocks
void Parse_If(bool check_Indendation, int indendation_Level) 
//if (sum) then 
//  BLOCK 
//else
//  BLOCK
{
    //CheckIndendation(check_Indendation, indendation_Level);
    Expect(KW_IF);
    Parse_Expression(); //sum
    Expect(KW_THEN);
    Expect(T_EOL);
    Parse_Block(check_Indendation, indendation_Level + 1); //BLOCK
    Expect(T_EOL);
    Expect(KW_ELSE);
    Expect(T_EOL);
    Parse_Block(check_Indendation, indendation_Level + 1); //BLOCK
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
         Parse_Expression(); //sum
    }
    Expect(T_RPAREN);
    Expect(T_EOL);
    Parse_Block(checkIndendation, indendationLevel + 1); //BLOCK
    Expect(KW_RETURN);
    Expect(T_EOL);
}

/// @brief Parses a while cycle(without the function ID name)
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
void Parse_While(bool check_Indendation, int indendation_Level)
{
    //While (sum) do
    //  BLOCK
    CheckIndendation(check_Indendation, indendation_Level);
    Expect(KW_WHILE);
    Parse_Expression(); //sum
    Expect(KW_DO);
    Expect(T_EOL);
    Parse_Block(check_Indendation, indendation_Level + 1); //BLOCK
}

/// @brief Parses a variable
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
void Parse_Variable(bool check_Indendation, int indendation_Level)
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
    else Error_Handling_Syntax(ERR_SYNTAX);

    Expect(T_ID);
    Next();
    if (token.type == T_COLON) //přiřazení var x: INT = *neco*
    {
        Next();
        switch (token.type)
        {
        case KW_INT:
            break;
        case KW_DOUBLE:
        case KW_STRING:
        default:
            Error_Handling_Syntax(ERR_SYNTAX);
            break;
        }
        Expect(T_ASSIGN);
    }
    else if (token.type == T_ASSIGN) //Assing x = *neco*
    {
        Expect(T_ASSIGN);
        Parse_ExpressionOrCall(check_Indendation, indendation_Level);
    }
    else Error_Handling_Syntax(ERR_SYNTAX);

}


/// @brief Decides if the next token points to function or expression
/// @param check_Indendation 
/// @param indendation_Level 
void Parse_ExpressionOrCall(bool check_Indendation, int indendation_Level)
{
    //= *neco*
    Expect(T_ID);
    if (nextToken.type == T_LPAREN)
    {
        ParseFunction(check_Indendation, indendation_Level);
    }
    else
    {
        Parse_Expression();
    }
}

/// @brief Parses expression
void Parse_Expression()
{
    //reset
    previous_Was_Operand = false;
    paren_Depth = 0;
    //set up stack
    Stack *stack = malloc(sizeof(Stack));
    MallocChecker(stack);
    StackInit(stack);

    //set up end token
    Token *endToken = malloc(sizeof(Token));
    MallocChecker(endToken);
    endToken->type = T_ENDSYMBOL;
    endToken->line = 0;
    StackPush(stack, *endToken);

    Token *middleToken = malloc(sizeof(Token));
    MallocChecker(middleToken);
    middleToken->type = T_MIDDLESYMBOL;
    endToken->line = 0;
    while (true)
    {
        switch(PTable_Finder(stack, token.type))
        {
            case S: //Push with <
            {
                StackItem *temp = malloc(sizeof(StackItem));
                MallocChecker(temp);
                UpdateState(token.type);
                StackTop(stack, temp);
                if (temp->type == P_NONTERM) //pokud je na vrchu zasobniku E
                {
                    StackPopReturn(stack, temp); //grab the E
                    StackPush(stack, *middleToken); //<
                    StackPush(stack, *temp); //Push E back
                }
                else 
                {
                    StackPush(stack, *middleToken); //<
                }
                StackPush(stack, token); //Token
                free(temp);
                Next();
                break;
            }
            case R: //Reduce (Pop)
            {
                StackItem items[TableMaxRuleSize]; //without the middle symbol
                int lenght = 0;

                StackItem *temp = malloc(sizeof(StackItem));
                MallocChecker(temp);
                StackPopReturn(stack, temp);
                while (temp->type != T_MIDDLESYMBOL)
                {
                    if (lenght >= TableMaxRuleSize) Error_Handling_Syntax(ERR_SYNTAX);
                    if (temp->type == T_ENDSYMBOL) Error_Handling_Syntax(ERR_SYNTAX);
                    items[lenght] = *temp;
                    lenght++;
                    StackPopReturn(stack, temp);
                }
                StackItem *result = malloc(sizeof(StackItem));
                if (!ReduceByRule(items, lenght, result)) Error_Handling_Syntax(ERR_SYNTAX);
                StackPush(stack, *result); //always E

                free(result);
                free(temp);
                break;
            }
            case E: //Push without <
            {
                StackPush(stack, token); //push el Token
                UpdateState(token.type);
                Next();
                break;
            }
            case F:
            {
                if (paren_Depth != 0) Error_Handling_Syntax(ERR_SYNTAX); //Not every ( has its )
                StackItem *temp = malloc(sizeof(StackItem));
                MallocChecker(temp);
                if (StackIsEmpty(stack)) Error_Handling_Syntax(ERR_SYNTAX);
                StackPopReturn(stack, temp);
                if (temp->type != P_NONTERM) Error_Handling_Syntax(ERR_SYNTAX); //Didnt end on $ E
                StackPopReturn(stack, temp);
                if (temp->type != T_ENDSYMBOL) Error_Handling_Syntax(ERR_SYNTAX); //Didnt end on $
                free(temp);
                StackFree(stack);
                free(middleToken);
                free(endToken);
                return; //found the end
            }
            case X: //syntax error
            {
                Error_Handling_Syntax(ERR_SYNTAX);
                StackFree(stack);
                free(middleToken);
                free(endToken);
                return;
            }
            default:
            {
                printf("Procedure table error: Not alocated space in the 2D table");
                Error_Handling_Syntax(ERR_INTERNAL);
                StackFree(stack);
                free(middleToken);
                free(endToken);
                return;
            }
        }
    }
    StackFree(stack);
    free(middleToken);
    free(endToken);
}

static void UpdateState(TokType token)
{
    previous_Was_Operand = (token == T_INT_LIT  ||
                            token == T_DOUBLE_LIT ||
                            token == T_STRING_LIT || 
                            token == T_ID ||
                            token == T_RPAREN);
    if (token == T_LPAREN) paren_Depth++;
    else if (token == T_RPAREN) paren_Depth--;
}

bool ReduceByRule(StackItem *items, int lenght, StackItem *out)
{
    // E → i
    if (lenght == 1 && Is_Term(items[0].type)) 
    { 
        out->type = P_NONTERM;
        return true;
    }

    // E → ( E )
    if (lenght == 3 && items[2].type == T_LPAREN && items[1].type == P_NONTERM && items[0].type == T_RPAREN)
        {
            return true; 
        }

    // E → E op E
    if (lenght == 3 && items[2].type == P_NONTERM && Is_Binary_Operator(items[1].type) && items[0].type == P_NONTERM)
        { 
            /* typová kontrola, AST uzel */ 
            return true; 
        }

    // E → -u E
    if (lenght == 2 && items[1].type == P_UMINUS && items[0].type == P_NONTERM)
        { 
            /* typ musí být Int/Double */ 
            return true; 
        }
    return false; //Rule not found
}

bool Is_Binary_Operator(TokType token)
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

bool Is_Term(TokType token)
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
char PTable_Finder(Stack *stack, TokType token_Input)
{
    Token *stackToken;
    StackTop(stack, stackToken);
    return p_Table[PTable_Index_Finder_3000(stackToken->type, stack)][PTable_Index_Finder_3000(token_Input, stack)];
}

/// @brief Returns the index for the procedure table finder
/// @param tokenType TokenType
/// @param stack Pointer to stack
/// @return 
int PTable_Index_Finder_3000(TokType tokenType, Stack *stack)
{
    switch (tokenType)
    {
        case T_MINUS:
        {
            if (previous_Was_Operand) return 2; //Bi minus
            else return 0; //Uni Minus
        }
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
void Parse_Block(bool check_Indendation, int indendation_Level)
{
    CheckIndendation(check_Indendation, indendation_Level);
    while (token.type != T_EOL)
    {
        switch (token.type) 
        {
            case KW_VAR: case KW_VAL: 
            {
                Parse_Variable(check_Indendation, indendation_Level);  
                break;
            }
            case KW_IF:               
            {
                Parse_If(check_Indendation, indendation_Level);       
                break;
            }
            case KW_WHILE:            
            {
                Parse_While(check_Indendation, indendation_Level);    
                break;
            }
            case KW_RETURN:           
            {
                Parse_Return(check_Indendation, indendation_Level); 
                break;
            }
            case T_ID:     
            {
                Parse_ExpressionOrCall(check_Indendation, indendation_Level);
                break;
            }
            default:                 
            {
                Error_Handling_Syntax(ERR_SYNTAX);
                break; 
            }
        }
    }
}

void Parse_Return(bool check_Indendation, int indendation_Level)
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
            Parse_ExpressionOrCall(check_Indendation, indendation_Level);
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

bool Syntax_Checker()
{
    Next(); //Very first token
    Expect(KW_IMPORT); //import ifj26.*

    while (token.type != T_EOF)
    {
        Parse_Block(true, 0);
    }
    return found_Error;
}

#endif