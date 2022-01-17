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
#include "phases/_05_varalloc/stack_layout.h"
#include "phases/_05_varalloc/varalloc.h"
#include "table/entry.h"
#include "table/identifier.h"
#include "table/table.h"
#include "types/types.h"

#define FIRST_REGISTER 8
#define LAST_REGISTER 23

#define FRAME_POINTER  25
#define STACK_POINTER  29

#define COMPATIBLE_MODE
int register_stack_pointer = FIRST_REGISTER - 1;
int label_counter = 0;


void increase_stack_pointer(){
	if(register_stack_pointer >= LAST_REGISTER){
		registerOverflow();
	}
	register_stack_pointer += 1;
}
void decrease_stack_pointer(){
	if(register_stack_pointer < FIRST_REGISTER){
		error("Register Underflow");
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
void genStatementList(StatementList *statement_list, SymbolTable *local_table, FILE *out);

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


void write_comment(FILE *out, char *format) {
#ifndef COMPATIBLE_MODE
	emit(out, "; %s", format);
#endif

}

void genReversedComparisonBinaryOperator(BinaryOperator operator, FILE *out, int op1, int op2, char* label){
	write_comment(out, "comparison operator");
	switch(operator){
		case ABSYN_OP_EQU:
			emitRRL(out, "bne", op1, op2, label);
			break;
		case ABSYN_OP_NEQ:
			emitRRL(out, "beq", op1, op2, label);
			break;
		case ABSYN_OP_GRT:
			emitRRL(out, "ble", op1, op2, label);
			break;
		case ABSYN_OP_GRE:
			emitRRL(out, "blt", op1, op2, label);
			break;
		case ABSYN_OP_LST:
			emitRRL(out, "bge", op1, op2, label);
			break;
		case ABSYN_OP_LSE:
			emitRRL(out, "bgt", op1, op2, label);
			break;
		default:
			error("Expected comparison operator but found arithmetic");
		}

}
void genArithmeticBinaryOperator(BinaryOperator operator, FILE *out){
	write_comment(out, "arithmetic operator");
	switch(operator){
		case ABSYN_OP_ADD:;
			emitRRR(out, "add", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		case ABSYN_OP_SUB:
			emitRRR(out, "sub", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		case ABSYN_OP_MUL:
			emitRRR(out, "mul", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		case ABSYN_OP_DIV:
			emitRRR(out, "div", register_stack_pointer-1, register_stack_pointer-1, register_stack_pointer);
			break;
		default:
			error("Expected arithetic operator but found comparision");
	}
	decrease_stack_pointer();
}

void genExpression(Expression *expression, SymbolTable *local_table, FILE *out);

void genIntLiteral(Expression *expression, FILE *out){
	write_comment(out, "intlit");
	int value = expression->u.intLiteral.value;
	increase_stack_pointer();
	emitRRI(out, "add", register_stack_pointer, 0, value);

}
//arr[3 * i] := arr
void genVariableExpression(Expression *expression, SymbolTable *local_table, FILE *out){
	write_comment(out, "variable Expression");
	Variable *variable = expression->u.variableExpression.variable;
	genVariable(variable, local_table, out);
	emitRRI(out, "ldw", register_stack_pointer, register_stack_pointer, 0);
}

void genArithmeticBinaryExpression(Expression *expression, SymbolTable *local_table, FILE *out){
	Expression *leftOperand = expression->u.binaryExpression.leftOperand;
	Expression *rightOperand = expression->u.binaryExpression.rightOperand;
	BinaryOperator operator = expression->u.binaryExpression.operator;

	genExpression(leftOperand, local_table, out);
	genExpression(rightOperand, local_table, out);
	genArithmeticBinaryOperator(operator, out);
}
void genJumpIfComparisonIsFalse(Expression *comparisonExpression, SymbolTable *local_table, FILE *out, char* label){
	Expression *leftOperand = comparisonExpression->u.binaryExpression.leftOperand;
	Expression *rightOperand = comparisonExpression->u.binaryExpression.rightOperand;
	BinaryOperator operator = comparisonExpression->u.binaryExpression.operator;

	genExpression(leftOperand, local_table, out);
	int index_left = register_stack_pointer;
	genExpression(rightOperand, local_table, out);
	int index_right = register_stack_pointer;
	genReversedComparisonBinaryOperator(operator, out, index_left, index_right, label);
	decrease_stack_pointer();
	decrease_stack_pointer();
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
			genArithmeticBinaryExpression(expression, local_table, out);
			break;
	}
}

/**
 * Load a variable and put its address on the register stack
 * When variable is an array-access, it loads the adress of the array + the offset of the index
 * Afterwards register_stack_pointer in increased
 * Generate at least one "add" instruction as last instruction
 */
void genVariable(Variable *variable, SymbolTable *local_table, FILE *out){
	switch(variable->kind){
		case VARIABLE_NAMEDVARIABLE:
			;
			write_comment(out, "named variable");
			Identifier *name = variable->u.namedVariable.name;
			Entry *looked_up = lookup(local_table, name);
			increase_stack_pointer();
			emitRRI(out, "add", register_stack_pointer, FRAME_POINTER, looked_up->u.varEntry.offset);
			

			break;
		case VARIABLE_ARRAYACCESS:
			;// for example arr[5]
			write_comment(out, "variable, array access");
			Variable *array = variable->u.arrayAccess.array;
			Expression *index = variable->u.arrayAccess.index;
			genVariable(array, local_table, out); // write address of arr on the stack
			int address_array = register_stack_pointer;

			genExpression(index, local_table, out); // write the result of the index expression. Here 5
			int address_index = register_stack_pointer; 

			//check index as unsigned int
			// jump when index >= array size
			int index_array_size = pushc(array->dataType->u.arrayType.size, out);
			emitRRL(out, "bgeu", address_index, index_array_size, "_indexError");
			decrease_stack_pointer();// remove array size

			//calculate offset = index * byteSize of baseType . In our example 5*4
			emitRRI(out, "mul", register_stack_pointer, address_index, array->dataType->u.arrayType.baseType->byteSize);
			int offset = register_stack_pointer;
			decrease_stack_pointer();

			//push the result = `address of array + offset from the index` on the stack
			//					destination			op1					op2
			emitRRR(out, "add", register_stack_pointer, address_array , offset);

			break;
	}
}
void genAssignStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	write_comment(out, "assign_statement");
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
	decrease_stack_pointer();
	decrease_stack_pointer();
	
}
void genIfStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	write_comment(out, "if_statement");
	Expression *condition = statement->u.ifStatement.condition;
	Statement *than_part = statement->u.ifStatement.thenPart;
	Statement *else_part = statement->u.ifStatement.elsePart;
	int label_index = label_counter++;
	char else_label[100] ;
	char endif_label[100] ;
#ifdef COMPATIBLE_MODE
	bool has_else = else_part->kind != STATEMENT_EMPTYSTATEMENT;
	label_counter--;
	sprintf(else_label, "L%d", label_counter);
	label_counter++;
	sprintf(endif_label, "L%d", label_counter);
	genJumpIfComparisonIsFalse(condition, local_table, out, else_label);
	genStatement(than_part, local_table, out);
	if(has_else){
		emitJump(out, endif_label);
	}
	emitLabel(out, else_label);
	if(has_else){
		genStatement(else_part, local_table, out);
		emitLabel(out, endif_label);
	}
#else
	sprintf(endif_label, "endif_%d", label_index);
	sprintf(else_label, "else_%d", label_index);

	emit(out, ";if_%d:", label_index);
	genJumpIfComparisonIsFalse(condition, local_table, out, else_label);
	emit(out, ";then_%d:", label_index);
	genStatement(than_part, local_table, out);
	emitJump(out, endif_label);
	emitLabel(out, else_label);
	genStatement(else_part, local_table, out);
	emitLabel(out, endif_label);
#endif

}
void genWhileStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	write_comment(out, "while_statement");
	Expression *condition = statement->u.whileStatement.condition;
	Statement *body = statement->u.whileStatement.body;

	int label_index = label_counter++;
	char end_label[100] ;
	char start_label[100] ;

#ifdef COMPATIBLE_MODE
	sprintf(start_label, "L%d", label_index);
	label_index = label_counter++;
	sprintf(end_label, "L%d", label_index);
#else
	sprintf(start_label, "while_start_%d", label_index);
	sprintf(end_label, "while_end_%d", label_index);
#endif

	emitLabel(out, start_label);
	genJumpIfComparisonIsFalse(condition, local_table, out, end_label);

	//if ergebniss der expression != true
	genStatement(body, local_table, out);
	emitJump(out, start_label);

	emitLabel(out, end_label);



}
void genCompoundStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	write_comment(out, "compound_statement");
	StatementList *statements = statement->u.compoundStatement.statements;
	genStatementList(statements, local_table, out);
}

void genCallStatement(Statement *statement, SymbolTable *local_table, FILE *out){
	write_comment(out, "call_statement");
	Identifier *procedure_name = statement->u.callStatement.procedureName;
	Entry *looked_up = lookup(local_table, procedure_name);

	ExpressionList *arguments = statement->u.callStatement.arguments;
	Expression *current_argument;
	ParameterTypeList *parameter_types = looked_up->u.procEntry.parameterTypes;
	ParameterType *current_parameter_type;

	int argument_place = 0;
	while(!arguments->isEmpty){
		current_argument = arguments->head;
		arguments = arguments->tail;
		current_parameter_type = parameter_types->head;
		parameter_types = parameter_types->tail;

		genExpression(current_argument, local_table, out);
		commentRRI(out, "stw", register_stack_pointer, STACK_POINTER, current_parameter_type->offset, "store argument #%d", argument_place);
		decrease_stack_pointer();

		argument_place += 1;
	}

		emit(out, "\tjal\t%s", procedure_name->string);
	


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
			break;
	}
}
void genStatementList(StatementList *statement_list, SymbolTable *local_table, FILE *out){

	Statement *current;
	while(!statement_list->isEmpty){
		current = statement_list->head;
		statement_list = statement_list->tail;

		genStatement(current, local_table, out);
	}
}
void genProcedureDeclaration(GlobalDeclaration *procedureDeclaration, SymbolTable *globalTable, FILE *out){
	StatementList *statement_list = procedureDeclaration->u.procedureDeclaration.body;
	Entry *looked_up = lookup(globalTable, procedureDeclaration->name);
	SymbolTable *local_table = looked_up->u.procEntry.localTable;
	StackLayout *stack_layout = looked_up->u.procEntry.stackLayout;


	//	– Prozedur-Prolog ausgeben
	int fp_size = REF_BYTE_SIZE;
	int return_size = isLeafProcedure(stack_layout)? 0 :REF_BYTE_SIZE;
	int stack_size = getFrameSize(stack_layout);
	int outgoingAreaSize = stack_layout->outgoingAreaSize != -1? stack_layout->outgoingAreaSize: 0;
	int return_offset = - (stack_layout->localVarAreaSize + fp_size + return_size);

	write_comment(out, "function prolog");

	emit(out, "");
	emitSS(out, ".export", procedureDeclaration->name->string);
	emitLabel(out, "%s", procedureDeclaration->name->string);

	commentRRI(out, "sub", STACK_POINTER, STACK_POINTER, stack_size, "allocate frame");
	commentRRI(out, "stw", FRAME_POINTER, STACK_POINTER, outgoingAreaSize + return_size, "save old frame pointer");
	commentRRI(out, "add", FRAME_POINTER, STACK_POINTER, stack_size, "setup new frame pointer");
	if(!isLeafProcedure(stack_layout)){
		commentRRI(out, "stw", 31, FRAME_POINTER, return_offset, "save return register");
	}

	//	– Code für Prozedurkörper erzeugen✓
	write_comment(out, "function statements");
	genStatementList(statement_list, local_table, out);

	//	– Prozedur-Epilog ausgeben
	write_comment(out, "function epilog");
	if(!isLeafProcedure(stack_layout)){
		commentRRI(out, "ldw", 31, FRAME_POINTER, return_offset, "restore return register");
	}
	commentRRI(out, "ldw", FRAME_POINTER, STACK_POINTER, outgoingAreaSize + return_size, "restore old frame pointer");
	commentRRI(out, "add", STACK_POINTER, STACK_POINTER, stack_size, "release frame");
	commentR(out, "jr", 31, "return");
	
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
