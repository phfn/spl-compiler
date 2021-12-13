/*
 * procedurebodycheck.c -- semantic analysis
 */

#include "absyn/_declarations.h"
#include "absyn/expressions.h"
#include "absyn/global_declarations.h"
#include "absyn/statements.h"
#include "table/entry.h"
#include <string.h>
#include <assert.h>
#include <util/errors.h>
#include <absyn/absyn.h>
#include <types/types.h>
#include <table/table.h>

void checkProcedure(
		ParameterDeclarationList *parameters,
		VariableDeclarationList *variables,
		StatementList *body,
		SymbolTable* globalTable)
{
	while(!body->isEmpty){
		Statement *current = body->head;
		body = body->tail;
		//error checks
		switch(current->kind){
			case STATEMENT_ASSIGNSTATEMENT:
                // TODO dataType is noch NULL
				if (current->u.assignStatement.target->dataType != current->u.assignStatement.value->dataType){
					assignmentHasDifferentTypes(current->position);
				}
				if (current->u.assignStatement.value->kind != EXPRESSION_INTLITERAL){
					assignmentRequiresIntegers(current->position);
				}

				break;
			case STATEMENT_CALLSTATEMENT:
				break;
			case STATEMENT_COMPOUNDSTATEMENT:
				break;
			case STATEMENT_EMPTYSTATEMENT:
				break;
			case STATEMENT_IFSTATEMENT:
				break;
			case STATEMENT_WHILESTATEMENT:
				break;

		}
	}

}

void checkProcedures(Program *program, SymbolTable *globalTable) {
    //TODO (assignment 4b): Check all procedure bodies for semantic errors
    GlobalDeclarationList *gdl = program->declarations;
    GlobalDeclaration *current = gdl->head;
    while (!gdl->isEmpty) {
        current = gdl->head;
        gdl = gdl->tail;
        if (current->kind == DECLARATION_PROCEDUREDECLARATION) {
            //fuer alle proceduren

            ParameterDeclarationList *parameters = current->u.procedureDeclaration.parameters;
            VariableDeclarationList *variables = current->u.procedureDeclaration.variables;
            StatementList *body = current->u.procedureDeclaration.body;
            checkProcedure(parameters, variables, body, globalTable);

        }

    }
}
