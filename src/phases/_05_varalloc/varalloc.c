/*
 * varalloc.c -- variable allocation
 */

#include <absyn/absyn.h>
#include <table/table.h>
#include <limits.h>
#include <string.h>
#include <util/ascii_table.h>
#include <util/string_ops.h>
#include <util/errors.h>
#include "absyn/_declarations.h"
#include "absyn/global_declarations.h"
#include "absyn/lists.h"
#include "absyn/statements.h"
#include "phases/_05_varalloc/stack_layout.h"
#include "table/entry.h"
#include "types/types.h"
#include "varalloc.h"

DEFINE_LIST_TO_ARRAY(variableDeclarationListToArray, VariableDeclarationList, VariableDeclaration)

DEFINE_LIST_SIZE(variableDeclarationListSize, VariableDeclarationList)

DEFINE_LIST_SIZE(parameterDeclarationListSize, ParameterDeclarationList)

DEFINE_LIST_GET(parameterDeclarationListGet, ParameterDeclarationList, ParameterDeclaration)

DEFINE_LIST_GET(parameterTypeListGet, ParameterTypeList, ParameterType)

/**
 * Formats the variables of a procedure to a human readable format and prints it.
 * @param procDec       The procedure Declaration
 * @param globalTable   The global symbol table
 */
static void showProcedureVarAlloc(GlobalDeclaration *procDec, SymbolTable *globalTable) {
    Entry *entry = lookup(globalTable, procDec->name);
    SymbolTable *localTable = entry->u.procEntry.localTable;
    StackLayout *stackLayout = entry->u.procEntry.stackLayout;

    AsciiGraphicalTableBuilder *builder = newAsciiGraphicalTableBuilder();
    line(builder, ALIGN_CENTER, "...", "");

    //parameters
    {
        int sz = parameterDeclarationListSize(procDec->u.procedureDeclaration.parameters);
        bool *index_ar = (bool *) allocate(sz * sizeof(bool *));

        for (int i = 0; i < sz; ++i) index_ar[i] = true;

        // This loop repeatedly picks the parameter with the highest offset from the list and then removes it until no more elements are available.
        // It essentially is a selection sort without the "sort" part, i.e. just a selection.
        while (true) {
            int maxOffsetIndex = -1;
            int maxOffset = INT_MIN;

            for (int i = 0; i < sz; ++i) {
                if (!index_ar[i]) continue;

                int offset = lookup(entry->u.procEntry.localTable, parameterDeclarationListGet(procDec->u.procedureDeclaration.parameters, i)->name)->u.varEntry.offset;

                if (maxOffsetIndex == -1 || offset > maxOffset) {
                    maxOffsetIndex = i;
                    maxOffset = offset;
                }
            }

            if (maxOffsetIndex < 0) break;
            int index = maxOffsetIndex;
            index_ar[maxOffsetIndex] = false;

            ParameterDeclaration *parDec = parameterDeclarationListGet(procDec->u.procedureDeclaration.parameters, index);
            ParameterType *parType = parameterTypeListGet(entry->u.procEntry.parameterTypes, index);
            //                                   a
            Entry *parEntry = lookup(localTable, parDec->name);

            bool consistent = parEntry->u.varEntry.offset == parType->offset;
            String formattedOffset = consistent
                                     ? formatInt(parEntry->u.varEntry.offset, "NULL")
                                     : format("INCONSISTENT(%s/%s)", formatInt(parEntry->u.varEntry.offset, "NULL"), formatInt(parType->offset, "NULL"));

            line(builder, ALIGN_LEFT, format("par %s", parDec->name->string), format("<- FP + %s", formattedOffset));
        }

        release(index_ar);
    }

    sep(builder, ALIGN_CENTER, "BEGIN", "<- FP");

    //variables
    if (!procDec->u.procedureDeclaration.variables->isEmpty) {
        int sz = variableDeclarationListSize(procDec->u.procedureDeclaration.variables);

        VariableDeclaration **ar = (VariableDeclaration **) (allocate(sz * sizeof(VariableDeclaration *)));
        variableDeclarationListToArray(ar, procDec->u.procedureDeclaration.variables);

        // This loop repeatedly picks the variable with the highest offset from the list and then removes it until no more elements are available.
        // It essentially is a selection sort without the "sort" part, i.e. just a selection.
        while (true) {
            int maxOffsetIndex = -1;
            int maxOffset = INT_MIN;

            //Find the index of the variable with the biggest offset;
            for (int i = 0; i < sz; ++i) {
                if (ar[i] == NULL) continue;

                int offset = lookup(localTable, ar[i]->name)->u.varEntry.offset;

                if (maxOffsetIndex == -1 || offset > maxOffset) {
                    maxOffset = offset;
                    maxOffsetIndex = i;
                }
            }

            if (maxOffsetIndex < 0) break;

            Entry *varEntry = lookup(localTable, ar[maxOffsetIndex]->name);

            int offset = varEntry->u.varEntry.offset;
            line(builder, ALIGN_LEFT, format("var %s", ar[maxOffsetIndex]->name->string), format("<- FP - %s", offset == INT_MIN ? "NULL" : formatInt(-offset, "NULL")));
            ar[maxOffsetIndex] = NULL; //Remove from list
        }

        release(ar);

        sep(builder, ALIGN_CENTER, "", "");
    }

    line(builder, ALIGN_LEFT, "Old FP", format("<- SP + %s", formatInt(getOldFramePointerOffSet(stackLayout), "UNKNOWN")));

    if (isLeafProcedure(stackLayout)) close(builder, "END", "<- SP");
    else {
        int oldReturn = getOldReturnAddressOffset(stackLayout);
        line(builder, ALIGN_LEFT, "Old Return", format("<- FP - %s", oldReturn == INT_MIN ? "UNKNOWN" : formatInt(-oldReturn, "UNKNOWN")));

        //outgoing area
        if (stackLayout->outgoingAreaSize == INT_MIN || stackLayout->outgoingAreaSize > 0) {
            sep(builder, ALIGN_CENTER, "outgoing area", "");

            if (stackLayout->outgoingAreaSize != INT_MIN) {
                int max_args = stackLayout->outgoingAreaSize / 4;

                for (int i = 0; i < max_args; ++i) {
                    line(builder, ALIGN_LEFT, format("arg %d", max_args - i), format("<- SP + %d", (max_args - i - 1) * 4));
                }

            } else {
                line(builder, ALIGN_LEFT, "UNKNOWN SIZE", "");
            }
        }

        sep(builder, ALIGN_CENTER, "END", "<- SP");
        line(builder, ALIGN_CENTER, "...", "");
    }

    printf("Variable allocation for procedure '%s':\n", procDec->name->string);
    printf("  - size of argument area = %s\n", formatInt(stackLayout->argumentAreaSize, "NULL"));
    printf("  - size of localvar area = %s\n", formatInt(stackLayout->localVarAreaSize, "NULL"));
    printf("  - size of outgoing area = %s\n", formatInt(stackLayout->outgoingAreaSize, "NULL"));
    printf("  - frame size = %s\n", formatInt(getFrameSize(stackLayout), "UNKNOWN"));
    printf("\n");
    printf("  Stack layout:\n");
    printTable(builder, stdout, 4);
    printf("\n");
}

/**
  * Formats the variable allocation to a human readable format and prints it.
  *
  * @param program      The abstract syntax tree of the program
  * @param globalTable  The symbol table containing all symbols of the spl program
  */
static void showVarAllocation(Program *program, SymbolTable *globalTable) {
    GlobalDeclarationList *declarationList = program->declarations;

    while (!declarationList->isEmpty) {
        if (declarationList->head->kind == DECLARATION_PROCEDUREDECLARATION) {
            showProcedureVarAlloc(declarationList->head, globalTable);
        }

        declarationList = declarationList->tail;
    }
}

int calcArgSize(ParameterTypeList *ptl, ParameterDeclarationList *pdl, SymbolTable *local_table){
	ParameterType *current;
    ParameterDeclaration *current_parameter_declaration;
	int argSize = 0;
	while(!ptl->isEmpty){
		current = ptl->head;
		ptl = ptl->tail;

        current_parameter_declaration = pdl->head;
        pdl = pdl->tail;

		//TODO
		current->offset = argSize;

        Entry *looked_up = lookup(local_table, current_parameter_declaration->name);
        looked_up->u.varEntry.offset = argSize;
        if(current->isRef) {
            argSize +=  REF_BYTE_SIZE;
        } else{
            argSize += current->type->byteSize;
        }
	}
	return argSize;
}
int calcLocalVarSize(VariableDeclarationList *local_vars, SymbolTable *local_table){
	VariableDeclaration *current;
	int varSize = 0;
	while(!local_vars->isEmpty){
		current = local_vars->head;
		local_vars = local_vars->tail;

		varSize += current->typeExpression->dataType->byteSize;
		Entry *looked_up = lookup(local_table, current->name);
		looked_up->u.varEntry.offset = -varSize;


	}
	return varSize;
}

// int getSizeOfArguments(ExpressionList *arguments){
// 	Expression *current;
// 	int sum = 0;
// 	while(!arguments->isEmpty){
// 		current = arguments->head;
// 		arguments = arguments->tail;
//
// 		sum += current->dataType->byteSize;
// 	}
// 	return sum;

int calcOutgoingAreaSize(StatementList *body, SymbolTable *global_table){
	int max_out_size = -1;

	Statement *current;
	while(!body->isEmpty){
		current = body->head;
		body = body->tail;

		int statement_size;
		switch(current->kind){
			case STATEMENT_CALLSTATEMENT:
				;
				Entry *looked_up = lookup(global_table, current->u.callStatement.procedureName);
				statement_size= looked_up->u.procEntry.stackLayout->argumentAreaSize;
                break;
			case STATEMENT_COMPOUNDSTATEMENT:
				statement_size = calcOutgoingAreaSize(current->u.compoundStatement.statements, global_table);
                break;
			case STATEMENT_WHILESTATEMENT:
			case STATEMENT_DOWHILESTATEMENT:
				;
				StatementList *while_wrapper = newStatementList(current->u.whileStatement.body, emptyStatementList());
				statement_size = calcOutgoingAreaSize(while_wrapper, global_table);
				break;
			case STATEMENT_IFSTATEMENT:
				;
				StatementList *wrapper_then = newStatementList(current->u.ifStatement.thenPart, emptyStatementList());
				StatementList *wrapper_else = newStatementList(current->u.ifStatement.elsePart, emptyStatementList());
				int if_size = calcOutgoingAreaSize(wrapper_then, global_table);
				int else_size = calcOutgoingAreaSize(wrapper_else, global_table);
				statement_size = if_size > else_size? if_size: else_size;
				break;

			default:
				continue;
		}
		if (statement_size > max_out_size){
			max_out_size = statement_size;
		}

	}

	return max_out_size;
}

//bool callsFunction(StatementList *body){
//    Statement *current;
//    while(!body->isEmpty) {
//        current = body->head;
//        body = body->tail;
//        switch(current->kind) {
//			case STATEMENT_CALLSTATEMENT:
//				return true;
//			case STATEMENT_COMPOUNDSTATEMENT:
//				return callsFunction(current->u.compoundStatement.statements);
//			default:
//				continue;
//		}
//    }
//    return false;
//}

void allocVars(Program *program, SymbolTable *globalTable, bool showVarAlloc, bool ershovOptimization) {
    //TODO (assignment 5): Allocate stack slots for all parameters and local variables
	GlobalDeclarationList *global_declaration_list = program->declarations;
	GlobalDeclaration *current;
	while(!global_declaration_list->isEmpty){
		current = global_declaration_list->head;
		global_declaration_list = global_declaration_list->tail;
		if(current->kind == DECLARATION_PROCEDUREDECLARATION){

			 ParameterDeclarationList *pdl = current->u.procedureDeclaration.parameters;
			// ParameterDeclaration *p = parameters->head;
			Entry *e = lookup(globalTable, current->name);
			ParameterTypeList *ptl = e->u.procEntry.parameterTypes;
			VariableDeclarationList *local_vars = current->u.procedureDeclaration.variables;
			SymbolTable *local_table = e->u.procEntry.localTable;
			e->u.procEntry.stackLayout->argumentAreaSize = calcArgSize(ptl, pdl, local_table);
			e->u.procEntry.stackLayout->localVarAreaSize = calcLocalVarSize(local_vars, local_table);


			
			// VariableDeclarationList *variables = current->u.procedureDeclaration.variables;
		}
	}
	global_declaration_list = program->declarations;
	while(!global_declaration_list->isEmpty){
		current = global_declaration_list->head;
		global_declaration_list = global_declaration_list->tail;
		if(current->kind == DECLARATION_PROCEDUREDECLARATION){
			Entry *e = lookup(globalTable, current->name);
            StatementList *body = current->u.procedureDeclaration.body;
			StackLayout *stackLayout = e->u.procEntry.stackLayout;
			stackLayout->outgoingAreaSize = calcOutgoingAreaSize(body, globalTable);
		}
	}


    if (showVarAlloc) {
		showVarAllocation(program, globalTable);
	}
}
