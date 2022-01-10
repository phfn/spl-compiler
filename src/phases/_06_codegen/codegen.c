/*
 * codegen.c -- ECO32 code generator
 */

#include "codegen.h"

#include <util/errors.h>
#include <assert.h>
#include "absyn/_declarations.h"
#include "absyn/expressions.h"
#include "absyn/global_declarations.h"
#include "absyn/statements.h"
#include "absyn/variables.h"
#include "codeprint.h"
#include "table/entry.h"
#include "table/identifier.h"
#include "table/table.h"

#define FIRST_REGISTER 8
#define LAST_REGISTER 23

#define FRAME_POINTER  25
#define STACK_POINTER  29

int register_stack_pointer = FIRST_REGISTER - 1;
void increase_stack_pointer(){
	if(register_stack_pointer >= LAST_REGISTER){
		notImplemented();
	}
	register_stack_pointer += 1;
}
void decrease_stack_pointer(){
	if(register_stack_pointer < FIRST_REGISTER){
		notImplemented();
	}
	register_stack_pointer -= 1;
}

void genVariable(Variable *variable, SymbolTable *global_table, FILE *out);

/**
 * Emits needed import statements, to allow usage of the predefined functions and sets the correct settings
 * for the assembler.
 * @param out The file pointer where the output has to be emitted to.
 */
void assemblerProlog(FILE *out) {
    emitImport(out, "printi");
    emitImport(out, "printc");
    emitImport(out, "readi");
    emitImport(out, "readc");
    emitImport(out, "exit");
    emitImport(out, "time");
    emitImport(out, "clearAll");
    emitImport(out, "setPixel");
    emitImport(out, "drawLine");
    emitImport(out, "drawCircle");
    emitImport(out, "_indexError");

    emit(out, "");
    emit(out, "\t.code");
    emit(out, "\t.align\t4");
}



void genBinaryOperator(BinaryOperator operator, FILE *out){
	switch(operator){
		case ABSYN_OP_ADD:;
			emitRRI(out, "add", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		case ABSYN_OP_SUB:
			emitRRI(out, "sub", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		case ABSYN_OP_MUL:
			emitRRI(out, "mul", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		case ABSYN_OP_DIV:
			emitRRI(out, "div", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		case ABSYN_OP_EQU:
			notImplemented();
			break;
		case ABSYN_OP_NEQ:
			notImplemented();
			break;
		case ABSYN_OP_GRT:
			notImplemented();
			break;
		case ABSYN_OP_GRE:
			notImplemented();
			break;
		case ABSYN_OP_LST:
			notImplemented();
			break;
		case ABSYN_OP_LSE:
			notImplemented();
			break;
	}
	decrease_stack_pointer();
}

void genExpression(Expression *expression, SymbolTable *global_table, FILE *out);

void genIntLiteral(Expression *expression, SymbolTable *global_table, FILE *out){
	int value = expression->u.intLiteral.value;
	emitRRI(out, "add", register_stack_pointer, 0, value);
	increase_stack_pointer();

}
//arr[3 * i] := arr
void genVariableExpression(Expression *expression, SymbolTable *global_table, FILE *out){
	Variable *variable = expression->u.variableExpression.variable;
	genVariable(variable, global_table, out);
}

void genBinaryExpression(Expression *expression, SymbolTable *global_table, FILE *out){
	Expression *leftOperand = expression->u.binaryExpression.leftOperand;
	Expression *rightOperand = expression->u.binaryExpression.rightOperand;
	BinaryOperator operator = expression->u.binaryExpression.operator;

	genExpression(leftOperand, global_table, out);
	genExpression(rightOperand, global_table, out);
	genBinaryOperator(operator, out);
}

void genExpression(Expression *expression, SymbolTable *global_table, FILE *out){
	switch(expression->kind){
		case EXPRESSION_INTLITERAL:
			genIntLiteral(expression, global_table, out);
			break;
		case EXPRESSION_VARIABLEEXPRESSION:
			genVariableExpression(expression, global_table, out);
			break;
		case EXPRESSION_BINARYEXPRESSION:
			genBinaryExpression(expression, global_table, out);
			break;
	}
}

void genVariable(Variable *variable, SymbolTable *global_table, FILE *out){
	switch(variable->kind){
		case VARIABLE_NAMEDVARIABLE:
			;
			Identifier *name = variable->u.namedVariable.name;
			Entry *looked_up = lookup(global_table, name);
			increase_stack_pointer();
			emitRRI(out, "add", register_stack_pointer, 25, looked_up->u.varEntry.offset);
			

			break;
		case VARIABLE_ARRAYACCESS:
			notImplemented();
			break;
	}
}
void genAssignStatement(Statement *statement, SymbolTable *global_table, FILE *out){
	Variable *target = statement->u.assignStatement.target;
	Expression *value = statement->u.assignStatement.value;
	//iwas fuer target
	//lege adresse der variable auf stack
	genVariable(target, global_table, out);//register_stack_pointer-1
	

	//iwas fuer value
	//lege ergebniss von expression auf stack
	genExpression(value, global_table, out); //register_stack_pointer

	//assign					src						reg				offset
	emitRRI(out, "stw", register_stack_pointer, register_stack_pointer-1, 0);
	
}
void genIfStatement(Statement *statement, SymbolTable *global_table, FILE *out){
	notImplemented();
}
void genWhileStatement(Statement *statement, SymbolTable *global_table, FILE *out){
	notImplemented();
}
void genCompoundStatement(Statement *statement, SymbolTable *global_table, FILE *out){
	notImplemented();
}
void genCallStatement(Statement *statement, SymbolTable *global_table, FILE *out){
	notImplemented();
}
void genEmptyStatement(Statement *statement, SymbolTable *global_table, FILE *out){
	notImplemented();
}

void genStatement(Statement *statement, SymbolTable *global_table, FILE *out){
	switch(statement->kind){
		case STATEMENT_ASSIGNSTATEMENT:
			genAssignStatement(statement, global_table, out);
			break;
		case STATEMENT_IFSTATEMENT:
			genIfStatement(statement, global_table, out);
			break;
		case STATEMENT_WHILESTATEMENT:
			genWhileStatement(statement, global_table, out);
			break;
		case STATEMENT_COMPOUNDSTATEMENT:
			genCompoundStatement(statement, global_table, out);
			break;
		case STATEMENT_CALLSTATEMENT:
			genCallStatement(statement, global_table, out);
			break;
		case STATEMENT_EMPTYSTATEMENT:
			genEmptyStatement(statement, global_table, out);
			break;
	}
}
void genProcedureDeclaration(GlobalDeclaration *procedureDeclaration, SymbolTable *globalTable, FILE *out){
	StatementList *statement_list = procedureDeclaration->u.procedureDeclaration.body;
	Statement *current;
	while(!statement_list->isEmpty){
		current = statement_list->head;
		statement_list = statement_list->tail;

		genStatement(current, globalTable, out);
	}
	
}
void genTypeDeclaration(GlobalDeclaration *procedureDeclaration, SymbolTable *globalTable, FILE *out){
	notImplemented();
}

void genCode(Program *program, SymbolTable *globalTable, FILE *out, bool ershovOptimization) {
    assemblerProlog(out);

    //TODO (assignment 6): generate eco32 assembler code for the spl program
	GlobalDeclarationList *global_declaration_list = program->declarations;
	GlobalDeclaration *current;
	while(!global_declaration_list->isEmpty){
		current = global_declaration_list->head;
		global_declaration_list = global_declaration_list->tail;

		switch(current->kind){
			case DECLARATION_PROCEDUREDECLARATION:
				genProcedureDeclaration(current, globalTable, out);
			break;
			case DECLARATION_TYPEDECLARATION:
				genTypeDeclaration(current, globalTable, out);
			break;

		}
	}

    notImplemented();
}
