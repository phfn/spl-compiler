/*
 * errors.h -- error reporting
 */

#ifndef SPL_ERRORS_H
#define SPL_ERRORS_H

#include <table/identifier.h>
#include <absyn/position.h>

/**
 * Displays an error to the user and aborts execution.
 *
 * Calling this function will exit the program with an exit code of 1.
 *
 * @param fmt A format string used to display the error message.
 * @param ... Additional parameters used by the format string.
 */
void error(char *fmt, ...);

/**
 * A function used to mark specific code blocks as "not implemented".
 * Calls to this function have to be removed from said code blocks and replaced
 * by correctly working code as part of the different assignments.
 */
void notImplemented();

void syntaxError(Position position, char *token);
void undefinedType(Position position, Identifier *name);
void notAType(Position position, Identifier *name);
void redeclarationAsType(Position position, Identifier *name);
void mustBeAReferenceParameter(Position position, Identifier *name);
void redeclarationAsProcedure(Position position, Identifier *name);
void redeclarationAsParameter(Position position, Identifier *name);
void redeclarationAsVariable(Position position, Identifier *name);
void assignmentHasDifferentTypes(Position position);
void assignmentRequiresIntegers(Position position);
void ifConditionMustBeBoolean(Position position);
void whileConditionMustBeBoolean(Position position);
void undefinedProcedure(Position position, Identifier *name);
void callOfNonProcedure(Position position, Identifier *name);
void argumentTypeMismatch(Position position, Identifier *name, int argumentIndex);
void argumentMustBeAVariable(Position position, Identifier *name, int argumentIndex);
void tooFewArguments(Position position, Identifier *name);
void tooManyArguments(Position position, Identifier *name);
void operatorDifferentTypes(Position position);
void comparisonNonInteger(Position position);
void arithmeticOperatorNonInteger(Position position);
void undefinedVariable(Position position, Identifier *name);
void notAVariable(Position position, Identifier *name);
void indexingNonArray(Position position);
void indexingWithNonInteger(Position position);
void mainIsMissing();
void mainIsNotAProcedure();
void mainMustNotHaveParameters();
void illegalApostrophe(Position position);
void illegalCharacter(Position position, char character);

void registerOverflow();

#endif //SPL_ERRORS_H
