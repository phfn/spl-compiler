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

Type *getTypeOfExpression( Expression *exp, SymbolTable *local_table );

Type *getTypeOfVariable(
        Variable *variable,
        SymbolTable *localTable
) {

    switch (variable->kind) {
        case VARIABLE_ARRAYACCESS:
            //rekursiv den typ der variable ermitteln
            ;
            Type *type_von_unserm_array_das_wir_accessen = getTypeOfVariable(variable->u.arrayAccess.array, localTable);
//            when type_von_unserm_array_das_wir_accessen primitiv is haetten wir ja keinen array access drauf machen duerfen
            switch (type_von_unserm_array_das_wir_accessen->kind){
                case TYPE_KIND_PRIMITIVE:
                    indexingNonArray(variable->position);
                    break;
                case TYPE_KIND_ARRAY:
                    if(getTypeOfExpression(variable->u.arrayAccess.index, localTable) != intType){
                        indexingWithNonInteger(variable->position);
                    }
                    variable->dataType = type_von_unserm_array_das_wir_accessen->u.arrayType.baseType;
                    return variable->dataType;
            }
        case VARIABLE_NAMEDVARIABLE:
            ;
            //variable muss in symboltable sein und eine variable sein
            Identifier *name = variable->u.namedVariable.name;
            Entry* lookedUp = lookup(localTable, name);
            if(lookedUp == NULL){
                undefinedVariable(variable->position, name);
            }
            if(lookedUp->kind != ENTRY_KIND_VAR){
                notAVariable(variable->position, name);
            }
            variable->dataType = lookedUp->u.varEntry.type;
            return variable->dataType;
    }
}
Type *getTypeOfBinaryExpression(Expression *exp, SymbolTable *local_table){
    Expression *left = exp->u.binaryExpression.leftOperand;
    Expression *right = exp->u.binaryExpression.rightOperand;
    BinaryOperator operator = exp->u.binaryExpression.operator;

    left->dataType = getTypeOfExpression(left, local_table);
    right->dataType = getTypeOfExpression(right, local_table);
    if (left->dataType != right->dataType) {
        operatorDifferentTypes(exp->position);
    }

    switch (operator) {
        case ABSYN_OP_ADD:
        case ABSYN_OP_SUB:
        case ABSYN_OP_MUL:
        case ABSYN_OP_DIV:
            if (left->dataType->kind != TYPE_KIND_PRIMITIVE ||
                right->dataType->kind != TYPE_KIND_PRIMITIVE) {
                arithmeticOperatorNonInteger(exp->position);
            }
            exp->dataType = intType;
            break;
        case ABSYN_OP_EQU:
        case ABSYN_OP_NEQ:
        case ABSYN_OP_GRE:
        case ABSYN_OP_GRT:
        case ABSYN_OP_LSE:
        case ABSYN_OP_LST:
            if (left->dataType->kind != TYPE_KIND_PRIMITIVE ||
                right->dataType->kind != TYPE_KIND_PRIMITIVE) {
                comparisonNonInteger(exp->position);
            }
            exp->dataType = boolType;
            break;
        default:
            break;
    }
    return exp->dataType;
}

Type *getTypeOfVariableExpression(Expression *exp, SymbolTable *local_table){

    Variable *variable = exp->u.variableExpression.variable;

    variable->dataType = getTypeOfVariable(variable, local_table);
    return exp->u.variableExpression.variable->dataType;
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
            return getTypeOfVariableExpression(exp, local_table);

            //3 + 5
        case EXPRESSION_BINARYEXPRESSION:
            return getTypeOfBinaryExpression(exp, local_table);
            break;
    }
}

void checkStatement(
        Statement* statement,
        SymbolTable* localTable
    ){
    switch(statement->kind){
        case STATEMENT_ASSIGNSTATEMENT:
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
void checkProcedureBody(
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
    //Task: (assignment 4b): Check all procedure bodies for semantic errors

    GlobalDeclarationList *gdl = program->declarations;
    GlobalDeclaration *current;
    while (!gdl->isEmpty) {
        current = gdl->head;
        gdl = gdl->tail;
        if (current->kind == DECLARATION_PROCEDUREDECLARATION) {
            //fuer alle Prozeduren
            SymbolTable* localTable = lookup(globalTable, current->name)->u.procEntry.localTable;

            StatementList *body = current->u.procedureDeclaration.body;
            checkProcedureBody(body, localTable);

        }
    }
}
