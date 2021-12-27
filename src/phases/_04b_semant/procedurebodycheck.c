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
            //rekursiv den typ der variable ermitteln
            return getTypeOfVariable(variable->u.arrayAccess.array, localTable);
        case VARIABLE_NAMEDVARIABLE:
            ;
            //variable muss in symvoltable sein und eine variable sein
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

Type *getTypeOfExpression(
        Expression *exp,
        SymbolTable *local_table
        ) {
    switch (exp->kind) {
        // 5
        case EXPRESSION_INTLITERAL:
            return intType;

            // a
        case EXPRESSION_VARIABLEEXPRESSION:
            ;
            Variable *variable = exp->u.variableExpression.variable;

            variable->dataType = getTypeOfVariable(variable, local_table);
            //TODO error checks
            //vermutlich diese
//            void indexingNonArray(Position position);
//            void indexingWithNonInteger(Position position);

            return exp->u.variableExpression.variable->dataType;

            //3 + 5
        case EXPRESSION_BINARYEXPRESSION:
            ;
            Expression *left = exp->u.binaryExpression.leftOperand;
            Expression *right = exp->u.binaryExpression.rightOperand;
            BinaryOperator operator = exp->u.binaryExpression.operator;

            left->dataType = getTypeOfExpression(left, local_table);
            right->dataType = getTypeOfExpression(right, local_table);
            if (left->dataType != right->dataType) {
                operatorDifferentTypes(exp->position);
            }

            if (left->dataType->kind != TYPE_KIND_PRIMITIVE ||
                right->dataType->kind != TYPE_KIND_PRIMITIVE) {
                switch (operator) {
                    case ABSYN_OP_ADD:
                    case ABSYN_OP_SUB:
                    case ABSYN_OP_MUL:
                    case ABSYN_OP_DIV:
                        arithmeticOperatorNonInteger(exp->position);
                        break;
                    case ABSYN_OP_EQU:
                    case ABSYN_OP_NEQ:
                    case ABSYN_OP_GRE:
                    case ABSYN_OP_GRT:
                    case ABSYN_OP_LSE:
                    case ABSYN_OP_LST:
                        comparisonNonInteger(exp->position);
                        break;
                    default:
                        break;
                }
            }
            break;
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

            Variable *target = statement->u.assignStatement.target;
            Expression *value = statement->u.assignStatement.value;

            //variable muss in symvoltable sein und eine variable sein
            //rekursiv den typ der variable ermitteln
            target->dataType = getTypeOfVariable(target, localTable);

//            statement->u.assignStatement.value->dataType = getTypeOfExpression(statement->u.assignStatement.value, localTable);
            value->dataType = getTypeOfExpression(value, localTable);


            //prüfen dass links und rechts typkompatible sind
            if (target->dataType != value->dataType){
                assignmentHasDifferentTypes(statement->position);
            }
            if (value->dataType->kind != TYPE_KIND_PRIMITIVE){
                notImplemented();
                assignmentRequiresIntegers(statement->position);
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
