%{

/*
 * parser.y -- SPL parser specification
 */

#include <stdlib.h>
#include <util/errors.h>
#include <table/identifier.h>
#include <types/types.h>
#include <absyn/absyn.h>
#include <phases/_01_scanner/scanner.h>
#include <phases/_02_03_parser/parser.h>

void yyerror(Program**, char *);

%}

// This section is placed into the header to make functions available to other modules
%code requires{
	/**
	  @return The name of the token class signalled by the given id.
	 */
	char const * tokenName(int token_class_id);
}

%token-table
%expect 1 //TODO Change?
%parse-param {Program** program}

%union {
  NoVal noVal;
  IntVal intVal;
  IdentVal identVal;

  Expression *expression;
  Variable *variable;
  Statement *statement;
  TypeExpression *typeExpression;
  GlobalDeclaration *globalDeclaration;
  VariableDeclaration *variableDeclaration;
  ParameterDeclaration *parameterDeclaration;

  StatementList *statementList;
  ExpressionList *expressionList;
  VariableDeclarationList *variableList;
  ParameterDeclarationList *parameterList;
  GlobalDeclarationList *globalDeclarationList;
}

%token	<noVal>		ARRAY ELSE IF OF PROC
%token	<noVal>		REF TYPE VAR WHILE
%token	<noVal>		LPAREN RPAREN LBRACK
%token	<noVal>		RBRACK LCURL RCURL
%token	<noVal>		EQ NE LT LE GT GE
%token	<noVal>		ASGN COLON COMMA SEMIC
%token	<noVal>		PLUS MINUS STAR SLASH
%token	<identVal>	IDENT
%token	<intVal>	INTLIT

/* %type	<globalDeclarationList>	GlobalDeclarationList; */
/* %type	<globalDeclarationList>	NonEmptyGlobalDeclarationList; */
/* %type	<globalDeclaration>		GlobalDeclaration; */
/* %type	<globalDeclaration>		ProcDeclaration; */
/* %type	<globalDeclaration>		TypeDeclaration; */

/* %type	<typeExpression>		TypeExpression; */

/* %type	<statementList>			StatementList; */
/* %type	<statementList>			NonEmptyStatementList; */
/* %type	<statement>				IfStatement; */
/* %type	<statement>				WhileStatement; */
/* %type	<statement>				AssignStatement; */
/* %type	<statement>				CallStatement; */
/* %type	<statement>				EmptyStatement; */
/* %type	<statement>				CompoundStatement; */

/* %type	<parameterList>			ParameterDeclarationList; */
/* %type	<parameterDeclaration>	ParameterDeclaration; */

/* %type	<variableList>			VariableDeclarationList; */
/* %type	<variableDeclaration>	VariableDeclaration; */

/* %type	<expressionList>		ExpressionList; */
/* %type	<expression>			Expression; */




%start			program
/* ** */

%%

program							: GlobalDeclarationList { *program = newProgram(newPosition(1,1), $1);}
								;

GlobalDeclarationList			: { $$ = emptyGlobalDeclarationList();}
								| NonEmptyGlobalDeclarationList { $$ = $1;}
								;

NonEmptyGlobalDeclarationList	: GlobalDeclaration { $$ = newGlobalDeclarationList($1, EmptyGlobalDeclarationList());}
								| GlobalDeclaration NonEmptyGlobalDeclarationList { $$ = newGlobalDeclarationList($1, $2);}
								;

GlobalDeclaration				: ProcDeclaration //{$$=new GlobalDeclaration($1);}
								| TypeDeclaration
								;

TypeDeclaration					: TYPE IDENT EQ TypeExpression SEMIC
								;

ProcDeclaration					: PROC IDENT LPAREN ParameterDeclarationList RPAREN LCURL VariableDeclarationList StatementList RCURL
								;

ParameterDeclarationList		: 
								| NonEmptyParameterDeclarationList
								;

NonEmptyParameterDeclarationList: ParameterDeclaration
								| ParameterDeclaration COMMA NonEmptyParameterDeclarationList
								;

ParameterDeclaration			:     IDENT COLON TypeExpression
								| REF IDENT COLON TypeExpression
								;

VariableDeclarationList			:
								| VariableDeclaration VariableDeclarationList
								;

VariableDeclaration				: VAR IDENT COLON TypeExpression SEMIC
								;

TypeExpression					: IDENT
								| ARRAY LBRACK INTLIT RBRACK OF TypeExpression
								;


StatementList					: 
								| NonEmptyStatementList
								;

NonEmptyStatementList			: Statement
								| Statement NonEmptyStatementList
								;

Statement						: CallStatement
								| CompoundStatement
								| AssignStatement
								| IfStatement
								| WhileStatement
								| EmptyStatement
								/* | DoWhileStatement */
								;

EmptyStatement					: SEMIC
								;

CompoundStatement				: LCURL StatementList RCURL
								;

AssignStatement					: Variable ASGN Expression SEMIC
								;

/* IfStatement						: IF LPAREN BoolExpression RPAREN Statement */
/* 								| IF LPAREN BoolExpression RPAREN Statement ELSE Statement */
IfStatement						: IF LPAREN Expression RPAREN Statement
								| IF LPAREN Expression RPAREN Statement ELSE Statement
								;

/* WhileStatement					: WHILE LPAREN BoolExpression RPAREN Statement */
WhileStatement					: WHILE LPAREN Expression RPAREN Statement
								;

CallStatement					: IDENT LPAREN ParameterList RPAREN SEMIC
								;

ParameterList					:
								| NonEmptyParameterList
								;

NonEmptyParameterList			: Expression
								| Expression COMMA NonEmptyParameterList
								;


/* < + = */
/* Expression						: INTLIT */
/* 								| LPAREN Expression RPAREN */
/* 								| Expression PLUS Expression */
								/* ; */

Expression						: ExpressionLower
								;

// < > <= >= = #
ExpressionLower					: ExpressionLower ComparisonOperator ExpressionLow
								| ExpressionLow
								;

// + -
ExpressionLow					: ExpressionLow PlusMinusOperator ExpressionMid
								| ExpressionMid
								;

// * /
ExpressionMid					: ExpressionMid MulDivOperator ExpressionHigh
								| ExpressionHigh
								;
// -1
ExpressionHigh					: MINUS ExpressionHigher
								| ExpressionHigher
								;

/* Klammern oder intlit */
ExpressionHigher				: LPAREN ExpressionLower RPAREN
								| Variable
								| INTLIT
								;

Variable						: IDENT
								| IDENT Arrayzeugs
								;

Arrayzeugs						: LBRACK Expression RBRACK
								| Arrayzeugs LBRACK Expression RBRACK
								;

PlusMinusOperator				: PLUS
								| MINUS
								;

MulDivOperator					: STAR
								| SLASH
								;

ComparisonOperator				: LT | LE | GT | GE | EQ | NE ;

%%

void yyerror(Program** program, char *msg) {
    // The first parameter is needed because of '%parse-param {Program** program}'.
    // Both parameters are unused and gcc would normally emits a warning for them.
    // The following lines "use" the parameters, but do nothing. They are used to silence the warning.
    (void)program;
    (void)msg;
    syntaxError(yylval.noVal.position, yytext);
}

// This function needs to be defined here because yytname and YYTRANSLATE are only available in the parser's implementation file.
char const *tokenName(int token_class_id) {
  // 0 is a special case because token_name(token) return "$end" instead of EOF.
  return token_class_id == 0 ? "EOF" : yytname[YYTRANSLATE(token_class_id)];
}
