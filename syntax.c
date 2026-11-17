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
#include <string.h>

#include "structure.h"
#include "syntax.h"
#include "codeStruct.h"


static bool previousWasOperand = false;
static int parenDepth = 0;
static int currentIndendationLevel = 0; //global počet odsazení

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
static bool tokenPeaked = false;
static bool imported = false;

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
        if (imported && token.type == KW_IMPORT) //Kontrola více Import
        {
            ErrorHandlingSyntax(ERR_SYNTAX, token);
        }
    }
}

void Peek()
{
    if (!tokenPeaked)
    {
        tokenPeaked = true;
        nextToken = TokenGiver();
        if (imported && nextToken.type == KW_IMPORT) //Kontrola více Import
        {
            ErrorHandlingSyntax(ERR_SYNTAX, token);
        }
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
DataType ExpectDataType()
{
    switch(token.type)
    {
        case KW_INT:
        {
            Expect(KW_INT);
            return DT_INT;
        }
        case KW_DOUBLE:
        {
            Expect(KW_DOUBLE);
            return DT_DOUBLE;
        }
        case KW_STRING:
        {
            Expect(KW_STRING);
            return DT_STRING;
        }
        case KW_UNIT:
        {
            Expect(KW_UNIT);
            return DT_UNIT; 
        }
        default: 
        {
            ErrorHandlingSyntax(ERR_SYNTAX, token);
            return DT_NONE;
        }
    }
}

/// @brief 
Statement *ParseIf() 
{
    //if (sum) then 
    //  BLOCK 
    //else
    //  BLOCK
    //CheckIndendation(check_Indendation, indendationLevel);
    Statement *s = NewStatement(ST_IF);
    int indendation = currentIndendationLevel;
    Expect(KW_IF);
    s->data.ST_if.condition = ParseExpression(); //sum
    Expect(KW_THEN);
    ExpectEndOfLine();
    s->data.ST_if.then = ParseBlock(indendation); //BLOCK

    if (currentIndendationLevel != indendation) //If a Else musí být na stejném odsazení
    {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
    }

    Expect(KW_ELSE);
    ExpectEndOfLine();
    s->data.ST_if.els = ParseBlock(indendation); //BLOCK
    return s;
}

/// @brief Zpracuje funkci/call (nechává ID)
Expression *ParseCall()
{
    //Foo(sum,sum,sum)
    Expression *e = NewExpression(E_CALL);
    e->data.call.name = ExpectName();
    Expect(T_LPAREN);

    e->data.call.argc = 0;
    while(token.type != T_RPAREN) //empty == ID()
    {
        e->data.call.argc++; //up argument count
        e->data.call.args = realloc(e->data.call.args, e->data.call.argc * sizeof(Expression *)); //argc za
        MallocChecker(e->data.call.args);
        e->data.call.args[e->data.call.argc - 1] = ParseExpression(); //argc jede 1+ a pole 0+
        if (token.type == T_COMMA) 
        {
            Next(); //Johne, máme více argumentů na zpracování
        }
        else break; //Tot vše
    }
    Expect(T_RPAREN);
    return e;
}

/// @brief Parses a while cycle(without the function ID name)
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
Statement *ParseWhile()
{
    Statement *s = NewStatement(ST_WHILE);
    //While (sum) do
    //  BLOCK
    Expect(KW_WHILE);
    int indendation = currentIndendationLevel;
    s->data.ST_while.condition = ParseExpression(); //sum
    Expect(KW_DO);
    ExpectEndOfLine();
    s->data.ST_while.body = ParseBlock(indendation); //BODY
    return s;
}

/// @brief Parses a variable
/// @param checkIndendation If Indendation should be checked
/// @param indendationLevel Current Indendation Level
Statement *ParseVariable()
{
    //var ID =
    Statement *s = NewStatement(ST_VARDEF);
    s->line = token.line;
    if (token.type == KW_VAR)
    {
        Expect(KW_VAR);
        s->data.ST_vardef.isVar = true; //can be modified
    }
    else if (token.type == KW_VAL)
    {
        Expect(KW_VAL);
         s->data.ST_vardef.isVar = false; //cannot be modified
    }
    else 
    {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
    }

    s->data.ST_vardef.name = ExpectName();

    if (token.type == T_COLON) //přiřazení var x: INT = *neco*
    {
        Expect(T_COLON); //sežere :
        s->data.ST_vardef.type = ExpectDataType();
        Expect(T_ASSIGN);        
        Expression *e = ParseExpressionOrCall();
        s->data.ST_vardef.init = e;
        return s;
    }
    else if (token.type == T_ASSIGN) //Assing x = *neco*
    {
        Expect(T_ASSIGN);
        Expression *e = ParseExpressionOrCall();
        s->data.ST_vardef.init = e;
        return s;
    }
    ErrorHandlingSyntax(ERR_SYNTAX, token);
    return NULL;
}

/// @brief Parses an assign
Statement *ParseAssign()
{
    //ID = výraz-nebo-volání
    Statement *s = NewStatement(ST_ASSIGN);
    s->data.ST_assign.name = ExpectName(); //jmeno
    Expect(T_ASSIGN);
    s->data.ST_assign.value = ParseExpressionOrCall(); //výraz-nebo-volání
    return s;
}

/// @brief Decides if the next token points to function or expression
/// @param check_Indendation 
/// @param indendationLevel 
Expression *ParseExpressionOrCall()
{
    //= *neco*
    if (token.type == T_ID)
    {
        Peek();
        if (nextToken.type == T_LPAREN)
        {
            return ParseCall();
        }
    }
    return ParseExpression();
}

/// @brief Definuje funkci
/// @return 
FuncDef *ParseDefFunction()
{
    //def id(parametry) [: návratový-typ] = výraz-nebo-volání
    //def id(parametry) [: návratový-typ] =
    //   odsazený-blok
    int indendation = currentIndendationLevel;
    FuncDef *fd = NewFunction();
    Expect(KW_DEF);
    fd->name = ExpectName();
    Expect(T_LPAREN);
    while (token.type != T_RPAREN) //parametry
    {
        fd->params = realloc(fd->params, sizeof(Param) * (fd->paramCount + 1)); //Zvetsit param
        MallocChecker(fd->params);
        //ID:TYP
        fd->params[fd->paramCount].name = ExpectName(); //jmeno parametru        
        Expect(T_COLON);
        fd->params[fd->paramCount].type= ExpectDataType(); //typ parametru   
        fd->paramCount++;     
        if (token.type == T_COMMA)
        {
            Expect(T_COMMA);
        }        
        else break;  //Tot vse
    }  
    Expect(T_RPAREN); //sežere )

    if (token.type == T_COLON) //: návratový-typ
    {
        Expect(T_COLON);
        fd->returnType = ExpectDataType();
    }
    else
    {
        fd->returnType = DT_NONE;
    }
    Expect(T_ASSIGN);

    switch (token.type)
    {
        case T_EOL: //odsazený-blok
        {
            ExpectEndOfLine();
            fd->body = ParseBlock(indendation);
            break;
        }
        case T_ID: case T_INT_LIT: case T_DOUBLE_LIT: case T_STRING_LIT: case T_LPAREN: case T_MINUS: //= výraz-nebo-volání
        {
            Statement *s = NewStatement(ST_EXPR);
            s->data.expression = ParseExpressionOrCall();
            fd->body = NewBlock();
            BlockAppend(fd->body, s);
            ExpectEndOfLine();
            break;
        }

        default: //something is missing
        {
            ErrorHandlingSyntax(ERR_SYNTAX, token);
            break;
        }
    }
    return fd;

}

/// @brief Okopíruje název z tokenu a sežere ID 
/// @return Kopie názvu
char *ExpectName()
{
    if (token.type != T_ID)
    {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
    }
    char *name = CopyName();
    Next();
    return name;
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
                StackTop(stack, top);

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
                free(top);
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
                Expression *result = temp->e; //Hotový strom výrazu
                StackPopReturn(stack, temp);
                if (temp->token.type != T_ENDSYMBOL) ErrorHandlingSyntax(ERR_SYNTAX, token); //Didnt end on $
                free(temp);
                StackFree(stack);
                return result; //found the end
            }
            case X: //syntax error
            {
                ErrorHandlingSyntax(ERR_SYNTAX, token);
                StackFree(stack);
                return NULL;
            }
            default:
            {
                printf("Procedure table error: Not alocated space in the 2D table");
                ErrorHandlingSyntax(ERR_INTERNAL, token);
                StackFree(stack);
                return NULL;
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
    Expression *e;
    switch (item->token.type)
    {
        case T_ID: 
        {
            e = NewExpression(E_VAR); 
            e->data.name = strDuplicate(item->token.val.s); //IDname 
            break;
        }
        case T_INT_LIT:
        {
            e = NewExpression(E_INT);
            e->data.intValue = item->token.val.i;
            break;
        }
        case T_DOUBLE_LIT:    
        {
            e = NewExpression(E_DOUBLE);
            e->data.doubleValue = item->token.val.d;
            break;
        }
        case T_STRING_LIT:   
        {
             e = NewExpression(E_STR);
             e->data.stringValue = strDuplicate(item->token.val.s); 
             break;
        }
        default: return NULL;
    }
    return e;
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
            out->e = items[1].e;
            return true; 
        }

    // E → E op E
    if (lenght == 3 && items[2].token.type == P_NONTERM && IsBinaryOperator(items[1].token.type) && items[0].token.type == P_NONTERM)
        { 
            out->e = MakeBin(items[1].token.type, items[2].e, items[0].e);
            return true; 
        }

    // E → -u E
    if (lenght == 2 && items[1].token.type == P_UMINUS && items[0].token.type == P_NONTERM)
        { 
            if (items[1].token.type != P_UMINUS || items[0].token.type != P_NONTERM) //Kontrola
            {
                return false;
            }
            out->e = NewExpression(E_UNARY);
            out->e->operation = T_MINUS;
            out->e->data.unitary = items[0].e;
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

Block *NewBlock()
{
    Block *b = calloc(1, sizeof(Block));
    MallocChecker(b);
    return b;
}

/// @brief Returns the procedure table "word" based on stack and token
/// @param stack //Pointer to stack
/// @param token_Input //Token
/// @return 
char PTableFinder(Stack *stack, TokType tokenInput)
{
    StackItem top;
    StackTop(stack, &top);
    TokType stackType = top.token.type;
    if (stackType == P_NONTERM) //skip E
    {
        StackItem saved;
        StackPopReturn(stack, &saved); //vyndat E
        StackTop(stack, &top);
        stackType = top.token.type;
        StackPush(stack, saved); //vrátit E zpět
    }
    return p_Table[PTableIndexFinder_3000(stackType, false)][PTableIndexFinder_3000(tokenInput, true)];
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
        case T_RPAREN: 
        {
            if (parenDepth == 0 && isIndexInput) return 8; //první je ) pro vstupní
            return 6;
        }
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
Statement *ParseLine()
{
    Statement *s = NULL;
    switch (token.type) 
    {
        case KW_VAR: case KW_VAL: 
        {
            s = ParseVariable();  
            ExpectEndOfLine();
            break;
        }
        case KW_IF:               
        {
            s = ParseIf();       
            break;
        }
        case KW_WHILE:            
        {
            s = ParseWhile();    
            break;
        }
        case KW_RETURN:           
        {
            s = ParseReturn(); 
            ExpectEndOfLine();
            break;
        }
        case T_ID:     
        {
            Peek();
            if (nextToken.type == T_ASSIGN) //x = *neco*
            {
                s = ParseAssign();
            }
            else //x + 1 / foo(a, b)
            {
                s = NewStatement(ST_EXPR);
                s->data.expression = ParseExpressionOrCall();
            }
            ExpectEndOfLine();
            break;
        }
        case T_INT_LIT: case T_DOUBLE_LIT: case T_STRING_LIT: case T_LPAREN: case T_MINUS: //Samostatně stojící výraz
        {
            s = NewStatement(ST_EXPR);
            s->data.expression = ParseExpression();
            ExpectEndOfLine();
            break;
        }
        default:                 
        {
            ErrorHandlingSyntax(ERR_SYNTAX, token);
            break; 
        }
    }
    return s;
}

/// @brief Překládá strom a žere odsazení
/// @param checkIndendation If indendation should be checked
/// @param indendationLevel Current level of backspaces for blocks
Block *ParseBlock(int parentIndendation) //0
{
    //if (sum)      0
    //  sum         1
    //  sum         1

    // if (sum)     1
            //sum   2
            //sum   2
    
     if (token.type == T_EOF || currentIndendationLevel <= parentIndendation) //Blok nemůže být prázdný ani neodsazený
    {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
    }

    Block *b = NewBlock();

    int indendation = currentIndendationLevel;
    while (token.type != T_EOF && currentIndendationLevel == indendation)
    {
        BlockAppend(b, ParseLine());
    }
    return b;
}

Statement *ParseReturn()
{
    Statement *s = NewStatement(ST_RETURN);
    Expect(KW_RETURN);
    switch (token.type)
    {
        //Return
        case T_EOL:
            s->data.retur = NULL;
            break;
        case T_EOF:       
            s->data.retur = NULL;   
            break;

        //Return *sum*
        default:
            s->data.retur = ParseExpressionOrCall();
    }
    return s;
}

/// @brief Vrátí počet odsazení (a sežere Indedndation)
int NumberOfIntendations()
{
    if (token.type == T_INDENDATION)
    {
        int temp = token.val.i;  //počet odsazení
        Next();
        return temp;
    }
    else return 0;
}

/// @brief Kontroluje vstup pomocí tokenů a vrací
/// @param program Vrací "strom" pro Sematiku a CodeGen
/// @return Vrací jestli je vstup syntakticky správně
void SyntaxChecker(FuncDef **program)
{
    *program = NULL;
    FuncDef *last = NULL;

    Next(); //První token
    
    Expect(KW_IMPORT);
    imported = true;
    ExpectEndOfLine();

    while (token.type != T_EOF)
    {
        bool nextIsMain = false;
        if (token.type == KW_MAIN)
        {
            nextIsMain = true;
            Next();
        }

        if (token.type != KW_DEF || currentIndendationLevel != 0) //Na nejvyšší úrovni jsou jen funkce bez odsazení
        {
            ErrorHandlingSyntax(ERR_SYNTAX, token);
        }        
        FuncDef *fd = ParseDefFunction();
        if (nextIsMain)
        {
            fd->isMain = true;
        }
        if (last == NULL) *program = fd;
        else last->next = fd;
        last = fd;
    }
}

Expression *NewExpression(ExpressionEnum type)
{
    Expression *ex = calloc(1, sizeof(Expression));
    MallocChecker(ex);
    ex->line = token.line;
    ex->type = type;
    return ex;
}

Statement *NewStatement(StatementEnum type)
{
    Statement *st = calloc(1, sizeof(Statement));
    MallocChecker(st);
    st->line = token.line;
    st->type = type;
    return st;
}

FuncDef *NewFunction()
{
    FuncDef *fd = calloc(1, sizeof(FuncDef));
    MallocChecker(fd);
    return fd;
}

Expression *MakeBin(TokType operation, Expression *left, Expression *right)
{
    Expression *e = NewExpression(E_BIN);
    e->operation = operation;
    e->data.binary.left = left;
    e->data.binary.right = right;
    return e;
}

void BlockAppend(Block *b, Statement *s) 
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

/// @brief Okopíruje název z tokenu
/// @return 
char *CopyName()
{
    return strDuplicate(token.val.s);
}


void ExpectEndOfLine()
{
    if (token.type == T_EOL)
    {
        Next();
    }
    else if (token.type != T_EOF) //konec souboru
    {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
    }
    currentIndendationLevel = NumberOfIntendations(); //odsazení dalšího řádku
}

/// @brief Vlastní strdup (strdup není ve striktním C standardu)
/// @param s Řetězec, který se zkopíruje
/// @return Kopie řetězce na haldě (NULL pro NULL)
char *strDuplicate(const char *s)
{
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1; //+1 kvůli \0
    char *copy = malloc(len);
    MallocChecker(copy);
    memcpy(copy, s, len);
    return copy;
}
#endif