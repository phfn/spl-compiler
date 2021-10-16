%{

/*
 * scanner.flex -- SPL scanner specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <util/errors.h>
#include <util/memory.h>
#include <table/identifier.h>
#include <types/types.h>
#include <absyn/absyn.h>
#include <phases/_01_scanner/scanner.h>
#include <phases/_02_03_parser/parser.h>

static Position currentPosition = {1, 1};

%}

/* Disables the ability to read multiple files (which is not required in SPL). */
%option noyywrap
/* Sets the caching behavior of the generated scanner. */
%option never-interactive
/* This header may not always exist on Windows systems. */
%option nounistd
/* Disables unused functions, so no warnings are displayed during compilation. */
%option nounput
%option noinput

%{
CommandLineOptions *commandLineOptions = NULL;

/**
 * Advances the global column counter (columnNumber) by the length of the most recently matched lexem (yytext) using yyleng.
 */
static void advanceColumnCounter(){
    currentPosition.column += yyleng;
}

/**
 * Advances the current position counter to the start of the next line.
 * The line number is incremented and the column number is reset to 1.
 * Use this when detecting a new line to ensure correct token positions.
 */
static void advancePositionToNextLine(){
    currentPosition.line++;
    currentPosition.column = 1;
}

/**
 * Creates a symbol with a line number and no value (NoVal).
 * Note: Also calls 'advanceColumnCounter' after using the current column number.
 *
 * @param type The token type that was detected (for example ARRAY).
 * @return The given token type.
 */
static int symbol(int type) {
    yylval.noVal.position = currentPosition;
    advanceColumnCounter();
    return type;
}

/**
 * Creates a symbol with a line number and an Identifier* as value (IdentVal).
 * Note: Also calls 'advanceColumnCounter' after using the current column number.
 *
 * @param type The token type that was detected (should always be IDENT).
 * @param value The identifier that was detected. (Use newIdentifier(char* string) to construct an Identifier from yytext)
 * @return The given token type.
 */
static int symbolIdentVal(int type, Identifier* value) {
    yylval.identVal.position = currentPosition;
    advanceColumnCounter();
    yylval.identVal.val = value;
    return type;
}

/**
 * Creates a symbol with a line number and an integer as value (IntVal).
 * Note: Also calls 'advanceColumnCounter' after using the current column number.
 *
 * @param type The token type that was detected (should always be INTLIT).
 * @param value The value of the integer literal that was detected.
 * @return The given token type.
 */
static int symbolIntVal(int type, int value) {
    yylval.intVal.position = currentPosition;
    advanceColumnCounter();
    yylval.intVal.val = value;
    return type;
}
%}

%%

// TODO (assignment 1): The regular expressions for all tokens need to be defined here.


[ \t]					{advanceColumnCounter();}
"//".*\r?\n				{advancePositionToNextLine();}
\r?\n                   {advancePositionToNextLine();}

array					{return symbol(ARRAY);}
else					{return symbol(ELSE);}
if						{return symbol(IF);}
of						{return symbol(OF);}
proc					{return symbol(PROC);}
ref						{return symbol(REF);}
type					{return symbol(TYPE);}
var						{return symbol(VAR);}
while					{return symbol(WHILE);}

"("						{return symbol(LPAREN);}
")"						{return symbol(RPAREN);}
"["						{return symbol(LBRACK);}
"]"						{return symbol(RBRACK);}
"{"						{return symbol(LCURL);}
"}"						{return symbol(RCURL);}
"="						{return symbol(EQ);}
"#"						{return symbol(NE);}
"<"						{return symbol(LT);}
"<="					{return symbol(LE);}
">"						{return symbol(GT);}
">="					{return symbol(GE);}
":="					{return symbol(ASGN);}
":"						{return symbol(COLON);}
","						{return symbol(COMMA);}
";"						{return symbol(SEMIC);}
"+"						{return symbol(PLUS);}
"-"						{return symbol(MINUS);}
"*"						{return symbol(STAR);}
"/"						{return symbol(SLASH);}

'\\n'                   {return symbolIntVal(INTLIT, 10);}
'.'                     {return symbolIntVal(INTLIT, (int)yytext[1]);}
[0-9]+					{return symbolIntVal(INTLIT, atoi(yytext));}
0x[0-9a-fA-F]+			{return symbolIntVal(INTLIT, strtol(yytext+2, 0, 16));} // evtl 0x entfernen
[a-zA-Z_][a-zA-Z_0-9]*  {return symbolIdentVal(IDENT, newIdentifier(yytext));}


.|\n                    {illegalCharacter(currentPosition, yytext[0]);}

%%
