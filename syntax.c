// IFJ-2026-Překladač
// xfranek00 - Kamil Franek
// xhrusko00 - Ondřej Hruška
// xkonvim00 - Martin Konvička
// xklimat00 - Tomáš Klíma

#ifndef SYNTAX_C // Nedefinuj soubor znova, pokud uz je definovan
#define SYNTAX_C

// include
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codeStruct.h"
#include "structure.h"
#include "syntax.h"

static bool previousWasOperand = false; // pro Predidure tabulku
static int parenDepth = 0;              // pro Precidure tabulku
static int currentIndendationLevel = 0; // global počet odsazení

// PROCEDURE TABLE
#pragma region

// akce
#define S 's' // '<'  shift (vlož marker + token)
#define R 'r' // '>'  reduce
#define E 'e' // '='  shift bez markeru
#define X 'x' // '·'  chyba
#define F 'f' // hotovo ($ vs $)
#define $ '$' // End symbol
#define P_TABLE_SIZE 11
#define TABLE_MAX_RULE_SIZE 3

/// @brief LL Tabulka
// AI: Vygeneruj mi 2D tabulku v C podle téhle SA tabulky (*SA tabulka z
// dokumentace)
static const char p_Table[P_TABLE_SIZE][P_TABLE_SIZE] = {
    //       -u  */  +-  rel eq &&  || (   )   i   $
    /* u   */ {S, R, R, R, R, R, R, S, R, S, R},
    /* //  */ {S, R, R, R, R, R, R, S, R, S, R},
    /* +-  */ {S, S, R, R, R, R, R, S, R, S, R},
    /* rel */ {S, S, S, X, R, R, R, S, R, S, R},
    /* eq  */ {S, S, S, S, X, R, R, S, R, S, R},
    /* &&  */ {S, S, S, S, S, R, R, S, R, S, R},
    /* ||  */ {S, S, S, S, S, S, R, S, R, S, R},
    /* (   */ {S, S, S, S, S, S, S, S, E, S, X},
    /* )   */ {X, R, R, R, R, R, R, X, R, X, R},
    /* i   */ {X, R, R, R, R, R, R, X, R, X, R},
    /* $   */ {S, S, S, S, S, S, S, S, X, S, F},
    // S == Shift, R == Reduce, E == Equals, X == Unreachable syntax, F ==
    // Finito
};

#pragma endregion

static Token token;              // Token od Lexoru
static Token nextToken;          // Další token pro Peek()
static bool tokenPeaked = false; // Jestli je další token uložen v nextToken
static bool imported = false;    // Jestli přišel token KW_IMPORT

/// @brief Vezme další token z Lexoru (pokud je další token v nextToken, vezme
/// si ten token)
void Next() {
  if (tokenPeaked) // další token je v nextToken
  {
    token = nextToken;
    tokenPeaked = false; // proti zacyklinenim
    switch (token.type) {
    case T_SKIP:
      Next();
      return;
      break;
    default:
      break;
    }
  } else // Token z Lexoru
  {
    token = TokenGiver();
    switch (token.type) {
    case T_SKIP:
      Next();
      return;
      break;
    default:
      break;
    }
    if (imported &&
        token.type ==
            KW_IMPORT) // Kontrola více Import (Import může být pouze 1)
    {
      ErrorHandlingSyntax(ERR_SYNTAX, token);
    }
  }
}

/// @brief Vezme další token z lexoru a uloží do nextToken
void Peek() {
  if (!tokenPeaked) // Peek může vidět pouze 1 token dopředu
  {
    tokenPeaked = true;
    do {
      nextToken = TokenGiver();
    } while (nextToken.type == T_SKIP);
    if (imported &&
        nextToken.type ==
            KW_IMPORT) // Kontrola více Import (Import může být pouze 1)
    {
      ErrorHandlingSyntax(ERR_SYNTAX, token);
    }
  }
}

/// @brief Rozhodne, jestli další token je požadovaný token a načte další.
/// @param type Předpokládaný token
void Expect(TokType type) {
  if (token.type != type) {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
  }
  Next();
}

/// @brief Rozhodne, jestli další token je požadovaný token je Typu Data (INT,
/// DOUBLE, STRING, UNIT).
DataType ExpectDataType() {
  switch (token.type) {
  case KW_INT: {
    Expect(KW_INT);
    return DT_INT;
  }
  case KW_DOUBLE: {
    Expect(KW_DOUBLE);
    return DT_DOUBLE;
  }
  case KW_STRING: {
    Expect(KW_STRING);
    return DT_STRING;
  }
  case KW_BOOL: {
    Expect(KW_BOOL);
    return DT_BOOL;
  }
  case KW_UNIT: {
    Expect(KW_UNIT);
    return DT_UNIT;
  }
  default: {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
    return DT_NONE;
  }
  }
}

/// @brief Přeloží příkaz IF a vrátí statement
Statement *ParseIf() {
  // if (sum) then
  //   BLOCK
  // else
  //   BLOCK
  // CheckIndendation(check_Indendation, indendationLevel);
  Statement *s = NewStatement(ST_IF);
  int indendation = currentIndendationLevel;
  Expect(KW_IF);
  s->data.ST_if.condition = ParseExpression(); //(sum)
  Expect(KW_THEN);
  ExpectEndOfLine();
  s->data.ST_if.then = ParseBlock(indendation);
  if (token.type == KW_ELSE && currentIndendationLevel == indendation) {
    Expect(KW_ELSE);
    if (token.type == KW_IF) { // else if => else { if ... }
      Block *b = NewBlock();
      BlockAppend(b, ParseIf());
      s->data.ST_if.els = b;
    } else {
      ExpectEndOfLine();
      s->data.ST_if.els = ParseBlock(indendation);
    }
  }
  return s;
}

/// @brief Přeloží funkci/call (nechává ID) a vrátí statement
Expression *ParseCall() {
  // Foo(sum,sum,sum)
  Expression *e = NewExpression(E_CALL);
  e->data.call.name = ExpectName();
  Expect(T_LPAREN);

  e->data.call.argc = 0;
  while (token.type != T_RPAREN) // empty == ID()
  {
    e->data.call.argc++; // up argument count
    e->data.call.args =
        realloc(e->data.call.args,
                e->data.call.argc *
                    sizeof(Expression *)); // zvětšení arguments podle argc
    MallocChecker(e->data.call.args);
    e->data.call.args[e->data.call.argc - 1] =
        ParseExpression(); // argc jede 1+ a pole 0+
    if (token.type == T_COMMA) {
      Next(); // Johne, máme více argumentů na zpracování
    } else
      break; // Tot vše
  }
  Expect(T_RPAREN);
  return e;
}

/// @brief Přeloží While a vrátí statement
Statement *ParseWhile() {
  Statement *s = NewStatement(ST_WHILE);
  // While (sum) do
  //   BLOCK
  Expect(KW_WHILE);
  int indendation = currentIndendationLevel;
  s->data.ST_while.condition = ParseExpression(); // sum
  Expect(KW_DO);
  ExpectEndOfLine();
  s->data.ST_while.body = ParseBlock(indendation); // BODY
  return s;
}

/// @brief Přeloží var/val a vrátí statement
Statement *ParseVariable() {
  // var ID =
  Statement *s = NewStatement(ST_VARDEF);
  s->line = token.line;
  if (token.type == KW_VAR) // var x
  {
    Expect(KW_VAR);
    s->data.ST_vardef.isVar = true; // can be modified
  } else if (token.type == KW_VAL)  // val x
  {
    Expect(KW_VAL);
    s->data.ST_vardef.isVar = false; // cannot be modified
  } else {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
  }

  s->data.ST_vardef.name = ExpectName();

  if (token.type == T_COLON) // přiřazení var x: INT = *neco*
  {
    Expect(T_COLON); // sežere :
    s->data.ST_vardef.type = ExpectDataType();
    Expect(T_ASSIGN);
    Expression *e = ParseExpressionOrCall();
    s->data.ST_vardef.init = e;
    return s;
  } else if (token.type == T_ASSIGN) // přiřazení var = *neco* (bez typu)
  {
    Expect(T_ASSIGN);
    Expression *e = ParseExpressionOrCall(); //*neco
    s->data.ST_vardef.init = e;
    return s;
  } else
    ErrorHandlingSyntax(ERR_SYNTAX, token); // Není : nebo =
  return NULL;
}

/// @brief Přeloží ID = výraz-nebo-volání
Statement *ParseAssign() {
  // ID = výraz-nebo-volání
  Statement *s = NewStatement(ST_ASSIGN);
  s->data.ST_assign.name = ExpectName(); // jmeno
  Expect(T_ASSIGN);
  s->data.ST_assign.value = ParseExpressionOrCall(); // výraz-nebo-volání
  return s;
}

/// @brief Rozhodne jestli další token je Call nebo expression a vrátí
/// expression/call
Expression *ParseExpressionOrCall() {
  //= *neco*
  if (token.type == T_ID) {
    return ParseExpression();
  } else if (token.type == T_DOUBLE_LIT || token.type == T_INT_LIT ||
             token.type == T_STRING_LIT || token.type == T_LPAREN ||
             token.type == T_MINUS || token.type == KW_TRUE ||
             token.type == KW_FALSE || token.type == T_NOT) {
    return ParseExpression();
  } else {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
  }
  return NULL;
}

/// @brief Definuje funkci
FuncDef *ParseDefFunction() {
  // def id(parametry) [: návratový-typ] = výraz-nebo-volání
  // def id(parametry) [: návratový-typ] =
  //    odsazený-blok
  int indendation = currentIndendationLevel;
  FuncDef *fd = NewFunction();
  Expect(KW_DEF);
  fd->name = ExpectName();
  Expect(T_LPAREN);
  while (token.type != T_RPAREN) // parametry
  {
    fd->params = realloc(fd->params,
                         sizeof(Param) * (fd->paramCount + 1)); // Zvetsit param
    MallocChecker(fd->params);
    // ID:TYP
    fd->params[fd->paramCount].name = ExpectName(); // jmeno parametru
    Expect(T_COLON);
    fd->params[fd->paramCount].type = ExpectDataType(); // typ parametru
    fd->paramCount++;
    if (token.type == T_COMMA) {
      Expect(T_COMMA); // Hospodo, více parametrů
    } else
      break; // Tot vse
  }
  Expect(T_RPAREN); // sežere )

  if (token.type == T_COLON) //: návratový-typ
  {
    Expect(T_COLON);
    fd->returnType = ExpectDataType();
  } else {
    fd->returnType = DT_NONE;
  }
  Expect(T_ASSIGN);

  switch (token.type) {
  case T_EOL: // odsazený-blok
  {
    ExpectEndOfLine();
    fd->body = ParseBlock(indendation);
    break;
  }
  case T_ID:
  case T_INT_LIT:
  case T_DOUBLE_LIT:
  case T_STRING_LIT:
  case T_LPAREN:
  case T_MINUS: // foo() = výraz-nebo-volání
  case KW_TRUE:
  case KW_FALSE:
  case T_NOT: {
    Statement *s = NewStatement(ST_EXPR);
    s->data.expression = ParseExpressionOrCall();
    fd->body = NewBlock(); // block je 1 řádek
    BlockAppend(fd->body, s);
    ExpectEndOfLine();
    break;
  }

  default: // šéfe, mě se asi něco nepovedlo
  {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
    break;
  }
  }
  return fd;
}

/// @brief Okopíruje název z tokenu a sežere ID
/// @return Kopie názvu
char *ExpectName() {
  if (token.type != T_ID) {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
  }
  char *name = CopyName();
  Next();
  return name;
}

/// @brief Přeloží expression i s ()
Expression *ParseExpression() {
  // reset
  previousWasOperand = false;
  parenDepth = 0;
  // příprava zásobníku
  Stack *stack = malloc(sizeof(Stack));
  MallocChecker(stack);
  StackInit(stack);

  // Příprava END tokenu $
  Token endToken = {0};
  endToken.type = T_ENDSYMBOL;
  endToken.line = 0;
  StackItem itemEndToken = {0};
  itemEndToken.token = endToken;
  itemEndToken.line = token.line;
  StackPush(stack, itemEndToken);

  // Příprava MIDDLE tokenu E
  Token middleToken;
  middleToken.type = T_MIDDLESYMBOL;
  StackItem itemMiddleToken = {0};
  itemMiddleToken.token = middleToken;

  while (true) {
    switch (PTableFinder(stack, token.type)) // najdi hodnotu v tabulce
    {
    case S: // Push with <
    {
      StackItem *top = malloc(sizeof(StackItem)); // Vrchol zasobniku
      MallocChecker(top);
      StackItem item = {0};
      item.token = token; // Kopie Tokenu
      StackTop(stack, top);

      if (item.token.type == T_MINUS && !previousWasOperand) {
        item.token.type = P_UMINUS; // Zaměnit za unární mínus
      }

      // FUNEXP addon
      bool isCall = false;
      int savedParenDepth = parenDepth;
      if (token.type == T_ID) {
        Peek();
        if (nextToken.type == T_LPAREN) {
          isCall = true;
          item.e = ParseCall(); // sežere ID(*sum*) a rekurzivně zavolá
                                // ParseExpression
        }
      }

      if (top->token.type == P_NONTERM) // pokud je na vrchu zasobniku E
      {
        StackPopReturn(stack, top);        // grab the E
        StackPush(stack, itemMiddleToken); //<
        StackPush(stack, *top);            // Push E back
      } else {
        StackPush(stack, itemMiddleToken); //<
      }
      StackPush(stack, item); // Token
      if (isCall)             // FUNEXP addon
      {
        previousWasOperand = true;
        parenDepth = savedParenDepth;
      } else {
        UpdateState(token.type);
        Next();
      }
      free(top);
      break;
    }
    case R: // Reduce (Pop)
    {
      StackItem items[TABLE_MAX_RULE_SIZE]; // bez prostředního symbolu
      int lenght = 0;

      StackItem *temp = malloc(sizeof(StackItem)); // pro výstup ze zásobníku
      MallocChecker(temp);
      StackPopReturn(stack, temp);
      while (temp->token.type != T_MIDDLESYMBOL) {
        if (lenght >= TABLE_MAX_RULE_SIZE)
          ErrorHandlingSyntax(ERR_SYNTAX, token);
        if (temp->token.type == T_ENDSYMBOL)
          ErrorHandlingSyntax(ERR_SYNTAX, token);
        items[lenght] = *temp;
        lenght++;
        StackPopReturn(stack, temp);
      }
      StackItem *result = malloc(sizeof(StackItem));
      MallocChecker(result);
      if (!ReduceByRule(items, lenght, result)) {
        ErrorHandlingSyntax(ERR_SYNTAX, token);
      }
      if (result != NULL) {
        StackPush(stack, *result); // always E
      }

      free(result);
      free(temp);
      break;
    }
    case E: // Push without <
    {
      StackItem itemToken = {0}; // null pro vsechny hodnoty
      itemToken.token = token;
      StackPush(stack, itemToken); // push el Token
      UpdateState(token.type);
      Next();
      break;
    }
    case F: {
      if (parenDepth != 0)
        ErrorHandlingSyntax(ERR_SYNTAX, token); // Not every ( has its )
      StackItem *temp = malloc(sizeof(StackItem));
      MallocChecker(temp);
      if (StackIsEmpty(stack)) {
        free(temp);
        StackFree(stack);
        ErrorHandlingSyntax(ERR_SYNTAX, token);
      }
      StackPopReturn(stack, temp);
      if (temp->token.type != P_NONTERM) {
        free(temp);
        StackFree(stack);
        ErrorHandlingSyntax(ERR_SYNTAX, token); // Didnt end on $ E
      }
      Expression *result = temp->e; // Hotový strom výrazu
      StackPopReturn(stack, temp);
      if (temp->token.type != T_ENDSYMBOL) {
        free(temp);
        StackFree(stack);
        ErrorHandlingSyntax(ERR_SYNTAX, token); // Didnt end on $
      }
      free(temp);
      StackFree(stack);
      return result; // found the end
    }
    case X: // syntax error
    {
      StackFree(stack);
      ErrorHandlingSyntax(ERR_SYNTAX, token);
      return NULL;
    }
    default: {
      // printf("Procedure table error: Not alocated space in the 2D table");
      StackFree(stack);
      ErrorHandlingSyntax(ERR_INTERNAL, token);
      return NULL;
    }
    }
  }
}

/// @brief Nastaví hloubku () a previousWasOperand
/// @param tokType Typ tokenu
void UpdateState(TokType tokType) {
  previousWasOperand =
      (tokType == T_INT_LIT || tokType == T_DOUBLE_LIT ||
       tokType == T_STRING_LIT || tokType == T_ID || tokType == T_RPAREN ||
       tokType == KW_TRUE || tokType == KW_FALSE);
  if (tokType == T_LPAREN)
    parenDepth++;
  else if (tokType == T_RPAREN)
    parenDepth--;
}

/// @brief Vytvoření expression pro strom ze zásobníku (Procedure table)
Expression *ExpressionTransform(StackItem *item) {
  Expression *e;
  switch (item->token.type) {
  case T_ID: {
    if (item->e != NULL) // pokud už je expression vyplnen, vrat ho
    {
      return item->e;
    }
    e = NewExpression(E_VAR);
    e->data.name = strDuplicate(item->token.val.s); // IDname
    break;
  }
  case T_INT_LIT: {
    e = NewExpression(E_INT);
    e->data.intValue = item->token.val.i;
    break;
  }
  case KW_TRUE: // misto KW_BOOL pro BOOLTHEN
  case KW_FALSE: {
    e = NewExpression(E_BOOL);
    e->data.boolValue = item->token.val.b;
    break;
  }
  case T_DOUBLE_LIT: {
    e = NewExpression(E_DOUBLE);
    e->data.doubleValue = item->token.val.d;
    break;
  }
  case T_STRING_LIT: {
    e = NewExpression(E_STR);
    e->data.stringValue = strDuplicate(item->token.val.s);
    break;
  }
  default:
    return NULL;
  }
  return e;
}

/// @brief "redukuje" ze zásobníku podle pravidel
/// @param items Položky na redukci
/// @param lenght počet pravidel
/// @param out výstupní NONTERM (E)
/// @return Jestli byl najito pravidlo
bool ReduceByRule(StackItem *items, int lenght, StackItem *out) {
  out->token.type = P_NONTERM;
  out->e = NULL;

  // E → i
  if (lenght == 1 && IsTerm(items[0].token.type)) {
    out->token.type = P_NONTERM;
    out->e = ExpressionTransform(&items[0]);
    return true;
  }

  // ( )
  if (lenght == 2 && items[0].token.type == T_RPAREN &&
      items[1].token.type == T_LPAREN) {
    out->e = NewExpression(E_UNIT); // Vzducho prazdno v zavorkach
    return true;
  }

  // E → ( E )
  if (lenght == 3 && items[2].token.type == T_LPAREN &&
      items[1].token.type == P_NONTERM && items[0].token.type == T_RPAREN) {
    out->e = items[1].e;
    return true;
  }

  // E → E op E
  if (lenght == 3 && items[2].token.type == P_NONTERM &&
      IsBinaryOperator(items[1].token.type) &&
      items[0].token.type == P_NONTERM) {
    out->e = MakeBin(items[1].token.type, items[2].e, items[0].e);
    return true;
  }

  // E → -u E
  if (lenght == 2 && items[1].token.type == P_UMINUS &&
      items[0].token.type == P_NONTERM) {
    if (items[1].token.type != P_UMINUS ||
        items[0].token.type != P_NONTERM) // Kontrola
    {
      return false;
    }
    out->e = NewExpression(E_UNARY);
    out->e->operation = T_MINUS;
    out->e->data.unitary = items[0].e;
    return true;
  }

  // E → !E
  if (lenght == 2 && items[1].token.type == T_NOT &&
      items[0].token.type == P_NONTERM) {
    if (items[1].token.type != T_NOT ||
        items[0].token.type != P_NONTERM) // Kontrola
    {
      return false;
    }
    out->e = NewExpression(E_UNARY);
    out->e->operation = T_NOT;
    out->e->data.unitary = items[0].e;
    return true;
  }
  return false; // Rule not found
}

/// @brief Je token binární?
/// @param token Typ tokenu
/// @return true == Binary; false == Uni
bool IsBinaryOperator(TokType token) {
  switch (token) {
  case T_PLUS:
  case T_MINUS:
  case T_MUL:
  case T_DIV:
  case T_LT:
  case T_LE:
  case T_GT:
  case T_GE:
  case T_EQ:
  case T_NEQ:
  case T_AND:
  case T_OR:
    return true;
  default:
    return false;
  }
}

/// @brief Je token term? (INT,DOUBLE,STRING,ID)
/// @param token Typ tokenu
/// @return True == term, false == NoTerm
bool IsTerm(TokType token) {
  switch (token) {
  case T_INT_LIT:
  case T_DOUBLE_LIT:
  case T_STRING_LIT:
  case T_ID:
  case KW_TRUE:
  case KW_FALSE:
    return true;
  default:
    return false;
  }
}

/// @brief Vytvoří a vrátí nový block
Block *NewBlock() {
  Block *b = calloc(1, sizeof(Block));
  MallocChecker(b);
  return b;
}

/// @brief Vrátí hodnotu z tabulky podle dvou pozic
/// @param stack //Pointer to stack
/// @param token_Input //Token
/// @return
char PTableFinder(Stack *stack, TokType tokenInput) {
  StackItem top;
  StackTop(stack, &top);
  TokType stackType = top.token.type;
  if (stackType == P_NONTERM) // skip E
  {
    StackItem saved;
    StackPopReturn(stack, &saved); // vyndat E
    StackTop(stack, &top);
    stackType = top.token.type;
    StackPush(stack, saved); // vrátit E zpět
  }
  return p_Table[PTableIndexFinder_3000(stackType, false)]
                [PTableIndexFinder_3000(tokenInput, true)];
}

/// @brief Vrací index z typu tokenu pro tabulku
/// @param tokenType TokenType
/// @param isIndexInput Jestli je token ze vstupu/zásobníku
/// @return
int PTableIndexFinder_3000(TokType tokenType, bool isIndexInput) {
  switch (tokenType) {
  case T_MINUS: {
    if (isIndexInput) // Na inputu je potřeba rozlišit uminus/minus
    {
      if (previousWasOperand)
        return 2; // Bi minus
      return 0;   // Uni Minus
    }
    return 2; // Na stacku se už rozlišuje uminus/minus
  }
  case P_UMINUS:
    return 0;
  case T_NOT:
    return 0;
  case T_PLUS:
    return 2;
  case T_MUL:
    return 1;
  case T_DIV:
    return 1;
  case T_LT:
    return 3;
  case T_LE:
    return 3;
  case T_GT:
    return 3;
  case T_GE:
    return 3;
  case T_EQ:
    return 4;
  case T_NEQ:
    return 4;
  case T_AND:
    return 5;
  case T_OR:
    return 6;
  case T_LPAREN:
    return 7;
  case T_RPAREN: {
    if (parenDepth == 0 && isIndexInput)
      return 10; // první je ) pro vstupní
    return 8;
  }
  // Operand
  case T_INT_LIT:
  case T_DOUBLE_LIT:
  case T_STRING_LIT:
  case T_ID:
    return 9;
  case KW_TRUE:
    return 9;
  case KW_FALSE:
    return 9;
  // Operand end
  case T_ENDSYMBOL:
  default:
    return 10; //(EOF a další), vrací endSymbol
  }
}

/// @brief Konec jednoduchého příkazu: ';' (další příkaz zůstává na stejném
/// řádku, proto se odsazení nemění) nebo konec řádku
static void ExpectStatementEnd() {
  if (token.type == T_SEMI) {
    Next(); // sežere ;
  } else {
    ExpectEndOfLine();
  }
}

/// @brief Zpracuje řádek v kodu a vrátí statement
Statement *ParseLine() {
  Statement *s = NULL;
  switch (token.type) {
  case KW_VAR:
  case KW_VAL: {
    s = ParseVariable(); // var x = 5
    ExpectStatementEnd();
    break;
  }
  case KW_IF: {
    s = ParseIf();
    break;
  }
  case KW_WHILE: {
    s = ParseWhile();
    break;
  }
  case KW_RETURN: {
    s = ParseReturn();
    ExpectStatementEnd();
    break;
  }
  case T_ID: {
    Peek();
    if (nextToken.type == T_ASSIGN) // x = *neco*
    {
      s = ParseAssign();
    } else // x + 1 / foo(a, b)
    {
      s = NewStatement(ST_EXPR);
      s->data.expression = ParseExpressionOrCall();
    }
    ExpectStatementEnd();
    break;
  }
  case T_INT_LIT:
  case T_DOUBLE_LIT:
  case T_STRING_LIT:
  case T_LPAREN:
  case T_MINUS: // Samostatně stojící výraz
  case KW_TRUE:
  case KW_FALSE:
  case T_NOT: {
    s = NewStatement(ST_EXPR);
    s->data.expression = ParseExpression();
    ExpectStatementEnd();
    break;
  }
  default: {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
    break;
  }
  }
  return s;
}

/// @brief Překládá block (podle mezer)
/// @param parentIndendation Odsazení otce (If,While etc.)
Block *ParseBlock(int parentIndendation) // 0
{
  // if (sum)      0
  //   sum         1
  //   sum         1

  // if (sum)     1
  // sum   2
  // sum   2

  if (token.type == T_EOF ||
      currentIndendationLevel <=
          parentIndendation) // Blok nemůže být prázdný ani neodsazený
  {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
  }

  Block *b = NewBlock();

  int indendation = currentIndendationLevel;
  while (token.type != T_EOF && currentIndendationLevel == indendation) {
    BlockAppend(b, ParseLine());
  }
  return b;
}

/// @brief Zpracuje KW_RETURN
/// @return Vrátí statement
Statement *ParseReturn() {
  Statement *s = NewStatement(ST_RETURN);
  Expect(KW_RETURN);
  switch (token.type) {
  // Return
  case T_EOL:
  case T_EOF:
  case T_SEMI:
    s->data.retur = NULL;
    break;

  // Return *sum*
  default:
    s->data.retur = ParseExpressionOrCall();
  }
  return s;
}

/// @brief Vrátí počet odsazení (a sežere Indedndation)
int NumberOfIntendations() {
  if (token.type == T_INDENDATION) {
    int temp = token.val.i; // počet odsazení
    Next();
    return temp;
  } else
    return 0;
}

/// @brief Kontroluje vstup pomocí tokenů a syntaktických pravidel
/// @param program Vrací AST pro Sematiku a CodeGen
/// @return Vrací jestli je vstup syntakticky správně
void SyntaxChecker(FuncDef **program) {
  *program = NULL;
  FuncDef *last = NULL;

  Next(); // První token

  Expect(KW_IMPORT);
  imported = true;
  ExpectEndOfLine();

  while (token.type != T_EOF) // projede celý vstup
  {
    bool nextIsMain = false;
    if (token.type ==
        KW_MAIN) // nastavení @main funkce (více @main si reší sematika)
    {
      nextIsMain = true;
      Next();                  // sežere KW_MAIN
      if (token.type == T_EOL) // @main def ... na jednom řádku je taky platné
      {
        ExpectEndOfLine();
      }
    }

    if (token.type != KW_DEF ||
        currentIndendationLevel !=
            0) // Na nejvyšší úrovni jsou jen funkce bez odsazení
    {
      ErrorHandlingSyntax(ERR_SYNTAX, token);
    }
    FuncDef *fd = ParseDefFunction();
    if (nextIsMain) {
      fd->isMain = true;
    }
    if (last == NULL)
      *program = fd;
    else
      last->next = fd;
    last = fd;
  }
}

/// @brief Alokuje nový expression a nastaví typ
/// @param type Typ expression z Enum
/// @return
Expression *NewExpression(ExpressionEnum type) {
  Expression *ex = calloc(1, sizeof(Expression));
  MallocChecker(ex);
  ex->line = token.line;
  ex->type = type;
  ex->varId = -1;
  return ex;
}

/// @brief Alokuje nový statement a nastaví typ
/// @param type Typ statementu z Enum
/// @return
Statement *NewStatement(StatementEnum type) {
  Statement *st = calloc(1, sizeof(Statement));
  MallocChecker(st);
  st->line = token.line;
  st->type = type;
  st->data.ST_vardef.id = -1;
  st->data.ST_assign.id = -1;
  return st;
}

/// @brief Alokuje novou funkci
FuncDef *NewFunction() {
  FuncDef *fd = calloc(1, sizeof(FuncDef));
  MallocChecker(fd);
  return fd;
}

/// @brief Vytvoří rozdělení ve stromu
/// @param operation Typ tokenu
/// @param left Expression která půjde vlevo
/// @param right Expression která půjde vpravo
/// @return
Expression *MakeBin(TokType operation, Expression *left, Expression *right) {
  Expression *e = NewExpression(E_BIN);
  e->operation = operation;
  e->data.binary.left = left;
  e->data.binary.right = right;
  return e;
}

/// @brief Přidá blok do struktury
/// @param b Pointer na block
/// @param s Pointer na statement, která se přidává
void BlockAppend(Block *b, Statement *s) {
  if (b->last) {
    b->last->next = s;
  } else {
    b->first = s;
  }
  b->last = s;
}

/// @brief Okopíruje název z tokenu
/// @return Název tokenu
char *CopyName() { return strDuplicate(token.val.s); }

/// @brief Čeká a sežere EOL pokud je. Nastaví odsazení dalšího řádku
void ExpectEndOfLine() {
  if (token.type == T_EOL) {
    Next();
  } else if (token.type != T_EOF) // konec souboru
  {
    ErrorHandlingSyntax(ERR_SYNTAX, token);
  }
  currentIndendationLevel = NumberOfIntendations(); // odsazení dalšího řádku
}

/// @brief Vlastní strdup (strdup není ve striktním C standardu) :(
/// @param s Řetězec, který se zkopíruje
/// @return Kopie řetězce (NULL pro NULL)
char *strDuplicate(const char *s) {
  if (s == NULL)
    return NULL;
  size_t len = strlen(s) + 1; //+1 kvůli \0
  char *copy = malloc(len);
  MallocChecker(copy);
  memcpy(copy, s, len);
  return copy;
}
#endif