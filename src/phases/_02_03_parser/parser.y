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

%type	<globalDeclarationList>	GlobalDeclarationList;
%type	<globalDeclarationList>	NonEmptyGlobalDeclarationList;
%type	<globalDeclaration>		GlobalDeclaration;
%type	<globalDeclaration>		TypeDeclaration;
%type	<globalDeclaration>		ProcDeclaration;

%type	<parameterList>			ParameterDeclarationList;
%type	<parameterList>			NonEmptyParameterDeclarationList;
%type	<parameterDeclaration>	ParameterDeclaration;

%type	<variableList>			VariableDeclarationList;
%type	<variableDeclaration>	VariableDeclaration;

%type	<typeExpression>		TypeExpression;

%type	<statementList>			StatementList;
%type	<statementList>			NonEmptyStatementList;

%type	<statement>				Statement;
%type	<statement>				EmptyStatement;
%type	<statement>				CompoundStatement;
%type	<statement>				AssignStatement;
%type	<statement>				IfStatement;
%type	<statement>				WhileStatement;
%type	<statement>				CallStatement;

%type	<expressionList>		ExpressionList;
%type	<expressionList>		NonEmptyExpressionList;

%type	<expression>			Expression;
%type	<expression>			ExpressionLower;
%type	<expression>			ExpressionLow;
%type	<expression>			ExpressionMid;
%type	<expression>			ExpressionHigh;
%type	<expression>			ExpressionHigher;

%type	<variable>				Variable;


%start			program
/* ** */

%%

program							: GlobalDeclarationList { *program = newProgram(newPosition(1,1), $1);}
								;

GlobalDeclarationList			: { $$ = emptyGlobalDeclarationList();}
								| NonEmptyGlobalDeclarationList { $$ = $1;}
								;

NonEmptyGlobalDeclarationList	: GlobalDeclaration { $$ = newGlobalDeclarationList($1, emptyGlobalDeclarationList());}
								| GlobalDeclaration NonEmptyGlobalDeclarationList { $$ = newGlobalDeclarationList($1, $2);}
								;

GlobalDeclaration				: ProcDeclaration {$$ = $1;}
								| TypeDeclaration {$$ = $1;}
								;

TypeDeclaration					: TYPE IDENT EQ TypeExpression SEMIC {newTypeDeclaration($1.position, $2.val, $4);}
								;

ProcDeclaration					: PROC IDENT LPAREN ParameterDeclarationList RPAREN LCURL VariableDeclarationList StatementList RCURL
									{ $$ = newProcedureDeclaration($1.position, $2.val, $4, $7, $8);}
								;

ParameterDeclarationList		:
									 {$$ = emptyParameterList();}
								| NonEmptyParameterDeclarationList
									{ $$ = $1;}
								;

NonEmptyParameterDeclarationList: ParameterDeclaration
									{ $$ = newParameterList($1, emptyParameterList());}
								| ParameterDeclaration COMMA NonEmptyParameterDeclarationList
									{ $$ = newParameterList($1, $3) ;}
								;

ParameterDeclaration			: IDENT COLON TypeExpression
									{ $$ = newParameterDeclaration($1.position, $1.val, $3, false);}
								| REF IDENT COLON TypeExpression
									{ $$ = newParameterDeclaration($1.position, $2.val, $4, true);}
								;

VariableDeclarationList			:
									{$$ = emptyVariableList();}
								| VariableDeclaration VariableDeclarationList
									{$$ = newVariableList($1, $2);}
								;

VariableDeclaration				: VAR IDENT COLON TypeExpression SEMIC
									{$$ = newVariableDeclaration($1.position, $2.val, $4);}
								;

TypeExpression					: IDENT
									{$$ = newNamedTypeExpression($1.position, $1.val);}
								| ARRAY LBRACK INTLIT RBRACK OF TypeExpression
									{$$ = newArrayTypeExpression($1.position, $6, $3.val);}
								;


StatementList					: 
									{$$ = emptyStatementList();}
								| NonEmptyStatementList
									{$$ = $1;}
								;

NonEmptyStatementList			: Statement
									{$$ = newStatementList($1, emptyStatementList());}
								| Statement NonEmptyStatementList
									{$$ = newStatementList($1, $2);}
								;

Statement						: CallStatement {$$ = $1;}
								| CompoundStatement {$$ = $1;}
								| AssignStatement {$$ = $1;}
								| IfStatement {$$ = $1;}
								| WhileStatement {$$ = $1;}
								| EmptyStatement {$$ = $1;}
								/* | DoWhileStatement */
								;

EmptyStatement					: SEMIC
									{ $$ = newEmptyStatement($1.position);}
								;

CompoundStatement				: LCURL StatementList RCURL
									{$$ = newCompoundStatement($1.position, $2);}
								;

AssignStatement					: Variable ASGN Expression SEMIC
									{$$ = newAssignStatement($1->position, $1, $3);}
								;

IfStatement						: IF LPAREN Expression RPAREN Statement
									{$$ = newIfStatement($1.position, $3, $5, newEmptyStatement($5->position));}
								| IF LPAREN Expression RPAREN Statement ELSE Statement
									{$$ = newIfStatement($1.position, $3, $5, $7);}
								;

WhileStatement					: WHILE LPAREN Expression RPAREN Statement
									{$$ = newWhileStatement($1.position, $3, $5);}
								;

CallStatement					: IDENT LPAREN ExpressionList RPAREN SEMIC
									{ $$ = newCallStatement($1.position, $1.val, $3);}
								;

ExpressionList					:
									{ $$ = emptyExpressionList();}
								| NonEmptyExpressionList
									{ $$ = $1;}
								;

NonEmptyExpressionList			: Expression
									{ $$ = newExpressionList($1, emptyExpressionList());}
								| Expression COMMA NonEmptyExpressionList
									{ $$ = newExpressionList($1, $3);}
								;


Expression						: ExpressionLower
									{ $$ = $1;}
								;

// < > <= >= = #
ExpressionLower					: ExpressionLower LT ExpressionLow
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_LST, $1, $3);}
								| ExpressionLower LE ExpressionLow
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_LSE, $1, $3);}
								| ExpressionLower GT ExpressionLow
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_GRT, $1, $3);}
								| ExpressionLower GE ExpressionLow
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_GRE, $1, $3);}
								| ExpressionLower EQ ExpressionLow
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_EQU, $1, $3);}
								| ExpressionLower NE ExpressionLow
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_NEQ, $1, $3);}
								| ExpressionLow
									{ $$ = $1;}
								;

// + -
ExpressionLow					: ExpressionLow PLUS ExpressionMid
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_ADD, $1, $3);}
								| ExpressionLow MINUS ExpressionMid
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_SUB, $1, $3);}
								| ExpressionMid
									{ $$ = $1;}
								;

// * /
ExpressionMid					: ExpressionMid STAR ExpressionHigh
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_MUL, $1, $3);}
								| ExpressionMid SLASH ExpressionHigh
									{ $$ = newBinaryExpression($1->position, ABSYN_OP_DIV, $1, $3);}
								| ExpressionHigh
									{ $$ = $1;}
								;
// -1
ExpressionHigh					: MINUS ExpressionHigher
									{ $$ = newBinaryExpression($1.position, ABSYN_OP_SUB, newIntLiteral($1.position, 0), $2);}
								| ExpressionHigher
									{ $$ = $1;}
								;

/* Klammern oder intlit */
ExpressionHigher				: LPAREN ExpressionLower RPAREN
									{ $$ = $2;}
								| Variable
									{ $$ = newVariableExpression($1->position, $1);}
								| INTLIT
									{ $$ = newIntLiteral($1.position, $1.val);}
								;

Variable						: IDENT
									{ $$ = newNamedVariable($1.position, $1.val);}
								| Variable LBRACK Expression RBRACK
									{ $$ = newArrayAccess($1->position, $1, $3);}
								;

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
