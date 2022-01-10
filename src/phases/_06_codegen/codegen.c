/*
 * codegen.c -- ECO32 code generator
 */

#include "codegen.h"

#include <stdbool.h>
#include <stdio.h>
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
int label_counter = 0;


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
int pushc(int value, FILE *out){
	increase_stack_pointer();
	emitRRI(out, "add", register_stack_pointer, 0, value);
	return register_stack_pointer;
}

void genVariable(Variable *variable, SymbolTable *local_table, FILE *out);
void genStatement(Statement *statement, SymbolTable *local_table, FILE *out);

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

void genExpression(Expression *expression, SymbolTable *local_table, FILE *out);

void genIntLiteral(Expression *expression, FILE *out){
	int value = expression->u.intLiteral.value;
	increase_stack_pointer();
	emitRRI(out, "add", register_stack_pointer, 0, value);

}
//arr[3 * i] := arr
void genVariableExpression(Expression *expression, SymbolTable *local_table, FILE *out){
	Variable *variable = expression->u.variableExpression.variable;
	genVariable(variable, local_table, out);
}

void genBinaryExpression(Expression *expression, SymbolTable *local_table, FILE *out){
	Expression *leftOperand = expression->u.binaryExpression.leftOperand;
	Expression *rightOperand = expression->u.binaryExpression.rightOperand;
	BinaryOperator operator = expression->u.binaryExpression.operator;

	genExpression(leftOperand, local_table, out);
	genExpression(rightOperand, local_table, out);
	genBinaryOperator(operator, out);
}

void genExpression(Expression *expression, SymbolTable *local_table, FILE *out){
	switch(expression->kind){
		case EXPRESSION_INTLITERAL:
			genIntLiteral(expression, out);
			break;
		case EXPRESSION_VARIABLEEXPRESSION:
			genVariableExpression(expression, local_table, out);
			break;
		case EXPRESSION_BINARYEXPRESSION:
			genBinaryExpression(expression, local_table, out);
			break;
	}
}
void genVariable(Variable *variable, SymbolTable *local_table, FILE *out){
	switch(variable->kind){
		case VARIABLE_NAMEDVARIABLE:
			;
			Identifier *name = variable->u.namedVariable.name;
			Entry *looked_up = lookup(local_table, name);
			increase_stack_pointer();
			emitRRI(out, "add", register_stack_pointer, 25, looked_up->u.varEntry.offset);
			

			break;
		case VARIABLE_ARRAYACCESS:
			;
			Variable *array = variable->u.arrayAccess.array;
			Expression *index = variable->u.arrayAccess.index;
			genVariable(array, local_table, out); // start of array
			int index_array = register_stack_pointer;

			genExpression(index, local_table, out); // index
			int index_index = register_stack_pointer;

			//check index as unsigned int
			// jump when index >= array size
			int index_array_size = pushc(array->dataType->u.arrayType.size, out);
			emitRRL(out, "bgeu", index_index, index_array_size, "_indexError");

			//offset = index * byteSize
			increase_stack_pointer();
			emitRRI(out, "mul", register_stack_pointer, index_index, array->dataType->byteSize);
			int offset = register_stack_pointer;

			increase_stack_pointer();
			//						scr				reg					offset
			emitRRR(out, "add", index_array, register_stack_pointer , offset);

			break;
	}
}
void genAssignStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	Variable *target = statement->u.assignStatement.target;
	Expression *value = statement->u.assignStatement.value;
	//iwas fuer target
	//lege adresse der variable auf stack
	genVariable(target, local_table, out);//register_stack_pointer-1
	

	//iwas fuer value
	//lege ergebniss von expression auf stack
	genExpression(value, local_table, out); //register_stack_pointer

	//assign					src						reg				offset
	emitRRI(out, "stw", register_stack_pointer, register_stack_pointer-1, 0);
	
}
void genIfStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	notImplemented();
}
void genWhileStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	Expression *condition = statement->u.whileStatement.condition;
	Statement *body = statement->u.whileStatement.body;

	int label_index = ++label_counter;
	char end_label[100] ;
	char start_label[100] ;
	sprintf(start_label, "while_start_%08d", label_index);
	sprintf(end_label, "while_end_%08d", label_index);

	emitLabel(out, start_label);
	genExpression(condition, local_table, out);
	int index_condition = register_stack_pointer;

	//if ergebniss der expression != true
	int index_true = pushc(true, out);
	emitRRL(out, "bne", index_condition, index_true, end_label);
	genStatement(body, local_table, out);
	emitJump(out, start_label);

	emitLabel(out, end_label);



}
void genCompoundStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	notImplemented();
}
void genCallStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	notImplemented();
}
void genEmptyStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	notImplemented();
}

void genStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	switch(statement->kind){
		case STATEMENT_ASSIGNSTATEMENT:
			genAssignStatement(statement, local_table, out);
			break;
		case STATEMENT_IFSTATEMENT:
			genIfStatement(statement, local_table, out);
			break;
		case STATEMENT_WHILESTATEMENT:
			genWhileStatement(statement, local_table, out);
			break;
		case STATEMENT_COMPOUNDSTATEMENT:
			genCompoundStatement(statement, local_table, out);
			break;
		case STATEMENT_CALLSTATEMENT:
			genCallStatement(statement, local_table, out);
			break;
		case STATEMENT_EMPTYSTATEMENT:
			genEmptyStatement(statement, local_table, out);
			break;
	}
}
void genProcedureDeclaration(GlobalDeclaration *procedureDeclaration, SymbolTable *globalTable, FILE *out){
	StatementList *statement_list = procedureDeclaration->u.procedureDeclaration.body;
	Statement *current;
	while(!statement_list->isEmpty){
		current = statement_list->head;
		statement_list = statement_list->tail;
		Entry *looked_up = lookup(globalTable, procedureDeclaration->name);
		SymbolTable *local_table = looked_up->u.procEntry.localTable;

		genStatement(current, local_table, out);
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
