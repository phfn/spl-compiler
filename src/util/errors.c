/*
 * errors.c -- error reporting
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "errors.h"

void error(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "An error occurred: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(1);
}

void notImplemented() {
    error("You have not implemented this operation yet!");
}

static void splError(int errorCode, Position position, const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "An error occurred:\n");
    if (position.line >= 0) fprintf(stderr, "Line %d, Column %d: ", position.line, position.column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(errorCode);
}

void syntaxError(Position position, char *token) {
    splError(100, position, "syntax error. Unexpected token '%s'", token);
}

void undefinedType(Position position, Identifier *name) {
    splError(101, position, "undefined type %s", name->string);
}

void notAType(Position position, Identifier *name) {
    splError(102, position, "%s is not a type", name->string);
}

void redeclarationAsType(Position position, Identifier *name) {
    splError(103, position, "redeclaration of %s as type", name->string);
}

void mustBeAReferenceParameter(Position position, Identifier *name) {
    splError(104, position, "parameter %s must be a reference parameter", name->string);
}

void redeclarationAsProcedure(Position position, Identifier *name) {
    splError(105, position, "redeclaration of %s as procedure", name->string);
}

void redeclarationAsParameter(Position position, Identifier *name) {
    splError(106, position, "redeclaration of %s as parameter", name->string);
}

void redeclarationAsVariable(Position position, Identifier *name) {
    splError(107, position, "redeclaration of %s as variable", name->string);
}

void assignmentHasDifferentTypes(Position position) {
    splError(108, position, "assignment has different types");
}

void assignmentRequiresIntegers(Position position) {
    splError(109, position, "assignment requires integer variable");
}

void ifConditionMustBeBoolean(Position position) {
    splError(110, position, "'if' test expression must be of type boolean");
}

void whileConditionMustBeBoolean(Position position) {
    splError(111, position, "'while' test expression must be of type boolean");
}

void undefinedProcedure(Position position, Identifier *name) {
    splError(112, position, "undefined procedure %s", name->string);
}

void callOfNonProcedure(Position position, Identifier *name) {
    splError(113, position, "call of non-procedure %s", name->string);
}

void argumentTypeMismatch(Position position, Identifier *name, int argumentIndex) {
    splError(114, position, "procedure %s argument %d type mismatch", name->string, argumentIndex);
}

void argumentMustBeAVariable(Position position, Identifier *name, int argumentIndex) {
    splError(115, position, "procedure %s argument %d must be a variable", name->string, argumentIndex);
}

void tooFewArguments(Position position, Identifier *name) {
    splError(116, position, "procedure %s called with too few arguments", name->string);
}

void tooManyArguments(Position position, Identifier *name) {
    splError(117, position, "procedure %s called with too many arguments", name->string);
}

void operatorDifferentTypes(Position position) {
    splError(118, position, "expression combines different types");
}

void comparisonNonInteger(Position position) {
    splError(119, position, "comparison requires integer operands");
}

void arithmeticOperatorNonInteger(Position position) {
    splError(120, position, "arithmetic operation requires integer operands");
}

void undefinedVariable(Position position, Identifier *name) {
    splError(121, position, "undefined variable %s", name->string);
}

void notAVariable(Position position, Identifier *name) {
    splError(122, position, "%s is not a variable", name->string);
}

void indexingNonArray(Position position) {
    splError(123, position, "illegal indexing a non-array");
}

void indexingWithNonInteger(Position position) {
    splError(124, position, "illegal indexing with a non-integer");
}

void mainIsMissing() {
    splError(125, ERROR_POSITION, "procedure 'main' is missing");
}

void mainIsNotAProcedure() {
    splError(126, ERROR_POSITION, "'main' is not a procedure");
}

void mainMustNotHaveParameters() {
    splError(127, ERROR_POSITION, "procedure 'main' must not have any parameters");
}

void illegalApostrophe(Position position) {
    splError(99, position, "illegal use of apostrophe");
}

void illegalCharacter(Position position, char character) {
    splError(99, position, "Illegal character 0x%02x", (unsigned char) character);
}

void registerOverflow() {
    splError(140, ERROR_POSITION, "There are not enough registers to run this program!");
}

