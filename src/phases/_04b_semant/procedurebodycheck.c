/*
 * procedurebodycheck.c -- semantic analysis
 */

#include "absyn/_declarations.h"
#include "absyn/expressions.h"
#include "absyn/global_declarations.h"
#include "absyn/position.h"
#include "absyn/statements.h"
#include "phases/_04a_tablebuild/tablebuild.h"
#include "table/entry.h"
#include <string.h>
#include <assert.h>
#include <util/errors.h>
#include <absyn/absyn.h>
#include <types/types.h>
#include <table/table.h>

// proc fun(a: int, b: array[9] of int){
// var c: int;
// c := b[4];
// }

void checkVariables(
		VariableDeclarationList *variables,
		SymbolTable* localTable
		) {

	while(!variables->isEmpty){
		VariableDeclaration* current = variables->head;
		variables = variables->tail;

		// var b: int;
		// typeExpression(intlit)
		Type* type = createTypeForTypeExpression(current->typeExpression, localTable, current->position);
//		type->u.

	}
}

// var penis = array[21]of int
// penis[3] = penis[5]
Type *getTypeOfVariable(
        Variable *variable,
        SymbolTable *localTable
) {

    switch (variable->kind) {
        case VARIABLE_ARRAYACCESS:
            return getTypeOfVariable(variable->u.arrayAccess.array, localTable);
        case VARIABLE_NAMEDVARIABLE:
            ;
            Identifier *name = variable->u.namedVariable.name;
            Entry* lookedUp = lookup(localTable, name);
            if(lookedUp == NULL){
                undefinedVariable(variable->position, name);
            }
            if(lookedUp->kind != ENTRY_KIND_VAR){
                notAVariable(variable->position, name);
            }
            Type *t = lookedUp->u.varEntry.type;
            return t;
    }
}
void checkStatement(
        Statement* statement,
        SymbolTable* localTable
    ){
    switch(statement->kind){
        case STATEMENT_ASSIGNSTATEMENT:
            // TODO dataType is noch NULL
            ;
            Type* t = getTypeOfVariable(statement->u.assignStatement.target, localTable);


//            if (statement->u.assignStatement.target->dataType !=statement->u.assignStatement.value->dataType){
//                assignmentHasDifferentTypes(statement->position);
//            }

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
void checkProcedure(
		ParameterDeclarationList *parameters,
		VariableDeclarationList *variables,
		StatementList *body,
		SymbolTable* localTable)
{
	while(!body->isEmpty){
		Statement *current = body->head;
		body = body->tail;
		//error checks
        checkStatement(current, localTable);
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
		SymbolTable* localTable = lookup(globalTable, current->name)->u.procEntry.localTable;

            ParameterDeclarationList *parameters = current->u.procedureDeclaration.parameters;
            VariableDeclarationList *variables = current->u.procedureDeclaration.variables;
            StatementList *body = current->u.procedureDeclaration.body;
            checkProcedure(parameters, variables, body, localTable);

        }

    }
}
