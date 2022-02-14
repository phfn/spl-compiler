//calculate ershov-number
#include "ershov.h"
#include "absyn/_declarations.h"
#include "absyn/expressions.h"
#include "absyn/global_declarations.h"
#include "absyn/lists.h"
#include "absyn/variables.h"
#include <stdlib.h>

// void initializeErshov(GlobalDeclarationList *gdl){
// 	GlobalDeclaration *current_global_declaration;
// 	while (!gdl->isEmpty) {
// 		current_global_declaration = gdl->head;
// 		gdl = gdl->tail;
//
// 		if(current_global_declaration->kind == DECLARATION_PROCEDUREDECLARATION){
// 			StatementList *stl =  current_global_declaration->u.procedureDeclaration.body;
// 			Statement *current_statement;
// 			while (!stl->isEmpty) {
// 				current_statement = stl->head;
// 				stl=stl->tail;
//
// 				if(current_statement){}
// 			}
//
// 		}
//
// 	}
//
// }
//
int calc_ershov_var(Variable *var){
	switch (var->kind) {
		case VARIABLE_NAMEDVARIABLE:
			return var->ershov;
			break;
		case VARIABLE_ARRAYACCESS:
			;
			int a, b;
			a = var->u.arrayAccess.array->ershov;
			if (var->u.arrayAccess.index->ershov < 2){
				var->u.arrayAccess.index->ershov = 2;
			}
			b = var->u.arrayAccess.index->ershov;
			if(a>b){
				return a;
			}
			if(b>a){
				return b;
			}
			return a + 1;

			break;
		default:
			exit(1);
	}
}
int calc_ershov(Expression *exp){
	switch (exp->kind) {
		case EXPRESSION_INTLITERAL:
			return 1;
		case EXPRESSION_VARIABLEEXPRESSION:
			return exp->u.variableExpression.variable->ershov;
		case EXPRESSION_BINARYEXPRESSION:
			;
			int a, b;
			a = exp->u.binaryExpression.leftOperand->ershov;
			b = exp->u.binaryExpression.rightOperand->ershov;
			if(a>b){
				return a;
			}
			if(b>a){
				return b;
			}
			return a + 1;
		default:
			exit(1);
	}
}
