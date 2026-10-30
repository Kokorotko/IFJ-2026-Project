//IFJ-2026-Překladač
//xfranek00 - Kamil Franek
//xhrusko00 - Ondřej Hruška
//xkonvim00 - Martin Konvička
//xklimat00 - Tomáš Klíma

#ifndef SYNTAX //Nedefinuj soubor znova, pokud uz je definovan
#define SYNTAX

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

void Syntax_Error()
{
    printf("Syntax error on line: %i", token.line);
    exit(ERR_SYNTAX);
}


void Expect(int type)
{
    if (token.type != type)
    {
        Syntax_Error();
    }
    Next();
}

/// @brief 
/// @param indendationLevel current level of backspaces for blocks
void Parse_If(int indendationLevel) 
//if (sum) then 
//  BLOCK 
//else
//  BLOCK
{
    CheckIndendation(indendationLevel);
    Expect(T_LPAREN);
    Parse_expr(); //sum
    Expect(T_RPAREN);
    Expect(KW_THEN);
    Expect(T_EOL);
    ParseStatement(indendationLevel++); //BLOCK
    Expect(T_EOL);
    Expect(KW_ELSE);
    Expect(T_EOL);
    ParseStatement(indendationLevel++); //BLOCK
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
         Parse_expr(); //sum
    }
    Expect(T_RPAREN);
    Expect(T_EOL);
    ParseStatement(indendationLevel++); //BLOCK
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
    Expect(T_LPAREN);
    Peek();
    Parse_expr(); //sum
    Expect(T_RPAREN);
    Expect(KW_DO);
    Expect(T_EOL);
    ParseStatement(indendationLevel++); //BLOCK
}

/// @brief Parses a while cycle(without the function ID name)
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
void Parse_Variable(bool check_Indendation, int indendation_Level)
{
    //var ID = 
    //  BLOCK
    CheckIndendation(check_Indendation, indendation_Level);
    Expect(T_LPAREN);
    Peek();
    Parse_expr(); //sum
    Expect(T_RPAREN);
    Expect(KW_DO);
    Expect(T_EOL);
    ParseStatement(indendationLevel++); //BLOCK
}

/// @brief Parses a line of code
/// @param checkIndendation If indendation should be checked
/// @param indendationLevel Current level of backspaces for blocks
void ParseStatement(bool checkIndendation, int indendationLevel)
{
    CheckIndendation(checkIndendation, indendationLevel);
    while (token.type != T_EOL)
    {
        switch (token.type) 
        {
            case KW_VAR: case KW_VAL: 
            {
                Parse_Variable();  
                break;
            }
            case KW_IF:               
            {
                Parse_If();       
                break;
            }
            case KW_WHILE:            
            {
                Parse_while();    
                break;
            }
            case KW_RETURN:           
            {
                Parse_return(); 
                break;
            }
            case T_ID:     
            {
                //x = ... or
                //function
                Peek();
                switch (nextToken.type)
                {
                case T_LPAREN:
                    ParseFunction();
                    break;
                case T_EQ:
                    ParseStatement(false, 0);
                    break;
                default:
                    Syntax_Error();
                    break;
                }
                break; 
            }
            default:                 
            {
                break; 
            }
        }
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
            Expect(T_INTENDATION);
        }
    }
    return;
}

int SyntaxChecker(void)
{
    Next(); //first call
    return Parse_Program(); //ifj26 nebo jak to začíná
}

int Parse_Program()
{
    //import ifj.*
    Expect(KW_IMPORT);
    Expect(T_ID); 
    Expect(T_DOT);
    Expect(T_STAR);
    Expect(T_EOL);

    while (token.type != T_EOF)
    {
        ParseStatement(true, 0);
    }
    return 0;
}

#endif