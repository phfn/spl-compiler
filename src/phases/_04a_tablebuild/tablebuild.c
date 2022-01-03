/*
 * tablebuild.c -- symbol table creation
 */

#include "tablebuild.h"

#include <types/types.h>
#include <absyn/absyn.h>
#include <table/table.h>
#include <stdio.h>
#include <util/errors.h>
#include <phases/_04a_tablebuild/createtype.h>

#define BOOL_BYTE_SIZE   4    /* size of a bool in bytes */
#define INT_BYTE_SIZE    4    /* size of an int in bytes */


/**
 * Prints the local symbol table of a procedure together with a heading-line
 * NOTE: You have to call this after completing the local table to support '--tables'.
 * @param name           The name of the procedure
 * @param procedureEntry The entry of the procedure to print
 */
static void printSymbolTableAtEndOfProcedure(Identifier *name, Entry *procedureEntry) {
    printf("Symbol table at end of procedure '%s':\n", name->string);
    showTable(procedureEntry->u.procEntry.localTable);
    printf("\n");
}



ParameterTypeList *push(ParameterTypeList *list, ParameterType *el) {
    ParameterTypeList *current = list;
    while (!current->isEmpty) {
        current = current->tail;
    }
    current->isEmpty = false;
    current->tail = newParameterTypeList(el, emptyParameterTypeList());
    return list;
}

ParameterTypeList *createParameterTypeList(ParameterDeclarationList *pdl, SymbolTable *local_table, Position pos) {
    ParameterTypeList *ptl = emptyParameterTypeList();
    ParameterTypeList *last_item = ptl;
    while (!pdl->isEmpty) {
        //new parameter type
        ParameterDeclaration *current = pdl->head;
        pdl = pdl->tail;

        current->typeExpression->dataType = createTypeForTypeExpression(current->typeExpression, local_table, current->position);
        if (current->typeExpression->dataType->kind== TYPE_KIND_ARRAY && !current->isReference) {
            mustBeAReferenceParameter(pos, current->name);
        }
        Type *type = createTypeForTypeExpression(current->typeExpression, local_table, pos);
        ParameterType *parameterType = newParameterType(type, current->isReference);

        //Element ganz hinten anfügen
        if (last_item->isEmpty) {
            last_item->head = parameterType;
            last_item->tail = emptyParameterTypeList();
            last_item->isEmpty = false;
        } else {
            last_item->tail = newParameterTypeList(parameterType, emptyParameterTypeList());
            last_item->isEmpty = false;
            last_item = last_item->tail;
        }
    }
    return ptl;
}

SymbolTable *createProcedureSymbolTable(ParameterDeclarationList *parameters, VariableDeclarationList *variables,
                                        SymbolTable *upperLevel, Position pos) {
    SymbolTable *local_table = newTable(upperLevel);

    //alle parameter erzeugen
    while (!parameters->isEmpty) {
        ParameterDeclaration *current = parameters->head;
        parameters = parameters->tail;
        //error checks
        Identifier *name = current->name;
        //proc test(a: int, a: int){}
        Entry *lookedUpEntry = lookup(local_table, name);
        if (lookedUpEntry != NULL && lookedUpEntry->kind == ENTRY_KIND_VAR) {
            redeclarationAsParameter(pos, name);
        }
        Type *type = createTypeForTypeExpression(current->typeExpression, local_table, pos);
        Entry *entry = newVarEntry(type, current->isReference);
        enter(local_table, name, entry);
    }

    //alle variablen
    while (!variables->isEmpty) {
        VariableDeclaration *current = variables->head;
        variables = variables->tail;
        Type *type = createTypeForTypeExpression(current->typeExpression, local_table, pos);
        Identifier *name = current->name;
        //error checks
        Entry *lookedUpEntry = lookup(local_table, name);
        if (lookedUpEntry != NULL && lookedUpEntry->kind == ENTRY_KIND_VAR) {
            redeclarationAsVariable(pos, name);
        }
        Entry *entry = newVarEntry(type, false);

        enter(local_table, name, entry);
        //TODO nach errors gucken siehe vl folie 15 seite 5
    }

    return local_table;
}

SymbolTable *buildSymbolTable(Program *program, bool showSymbolTables) {
    intType = newPrimitiveType("int", INT_BYTE_SIZE);
    boolType = newPrimitiveType("boolean", BOOL_BYTE_SIZE);

    GlobalDeclarationList *decList = program->declarations;
    SymbolTable *globalTable = initializeGlobalTable();

    while (!decList->isEmpty) {
        GlobalDeclaration *current = decList->head;
        decList = decList->tail;
        switch (current->kind) {
            case DECLARATION_TYPEDECLARATION:
                //new Entry for the type declaration
                ;
                Type *type = createTypeForTypeExpression(current->u.typeDeclaration.typeExpression, globalTable,
                                                         current->position);
                Entry *type_entry = enter(globalTable, current->name, newTypeEntry(type));
                if (type_entry == NULL) {
                    redeclarationAsType(current->position, current->name);
                }
                break;
            case DECLARATION_PROCEDUREDECLARATION:
                //new Entry for the procedure declaration
                ;
                ParameterTypeList *parameterTypeList = createParameterTypeList(
                        current->u.procedureDeclaration.parameters, globalTable, current->position);
                ParameterDeclarationList *parameters = current->u.procedureDeclaration.parameters;
                VariableDeclarationList *variables = current->u.procedureDeclaration.variables;

                SymbolTable *procedureTable = createProcedureSymbolTable(parameters, variables, globalTable,
                                                                         current->position);
                Entry *proc_entry = enter(globalTable, current->name, newProcEntry(parameterTypeList, procedureTable));

                if (proc_entry == NULL) {
                    redeclarationAsProcedure(current->position, current->name);
                }

                if (showSymbolTables) {
                    printSymbolTableAtEndOfProcedure(current->name, proc_entry);
                }
                //for every procedure declaration, create a local symbol table
                break;
        }
    }

    // main procedure must exist and must not have parameters
    Entry* mainProcedure = lookup(globalTable, newIdentifier("main"));
    if (mainProcedure == NULL)
        mainIsMissing();
    if (mainProcedure->kind != ENTRY_KIND_PROC)
        mainIsNotAProcedure();
    if (!mainProcedure->u.procEntry.parameterTypes->isEmpty)
        mainMustNotHaveParameters();

    return globalTable;
}
