/*
 * tablebuild.c -- symbol table creation
 */

#include "tablebuild.h"

#include <types/types.h>
#include <absyn/absyn.h>
#include <table/table.h>
#include <stdio.h>
#include <util/errors.h>

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

Type* createTypeForTypeExpression(TypeExpression* e, SymbolTable* table, Position pos) {
    switch (e->kind) {
        case TYPEEXPRESSION_NAMEDTYPEEXPRESSION:
            ;
            Identifier *identifier = e->u.namedTypeExpression.name;

            // gibt es den Typ überhaupt?
            Entry *lookedUpEntry = lookup(table, identifier);
            if ( lookedUpEntry == NULL) {
                undefinedType(pos, identifier);// wenn als typ ein nicht existierender typ angegeben wurde
            }
            if(lookedUpEntry->kind != ENTRY_KIND_TYPE) {
                notAType(pos, identifier); //wenn als typ ein prozedurname angegeben wurde
            }
            return lookedUpEntry->u.typeEntry.type;
            break;

        case TYPEEXPRESSION_ARRAYTYPEEXPRESSION:
            return newArrayType(e->u.arrayTypeExpression.arraySize, createTypeForTypeExpression(e->u.arrayTypeExpression.baseType, table, pos));
            break;
    }
}

ParameterTypeList* createParameterTypeList(ParameterDeclarationList* pdl, SymbolTable* table, Position pos) {
    ParameterTypeList * ptl = emptyParameterTypeList();
    while(!pdl->isEmpty){
        //new parameter type
        ParameterDeclaration *current = pdl->head;
        pdl = pdl->tail;
        //type penis = array[21] of int
        //fun iwas(a: int, ref b: penis){}
        //a: int
        // b: array....
        if(current->typeExpression->kind == TYPEEXPRESSION_ARRAYTYPEEXPRESSION && !current->isReference){
            mustBeAReferenceParameter(pos, current->name);
        }
        Type *type = createTypeForTypeExpression(current->typeExpression, table, pos);
        ParameterType *parameterType = newParameterType(type, current->isReference);
//        current->typeExpression->dataType = type;
        ptl = newParameterTypeList(parameterType, ptl);
    }
    return ptl;
}

SymbolTable* createProcedureSymbolTable(ParameterDeclarationList* parameters, VariableDeclarationList* variables, SymbolTable* upperLevel, Position pos){
    SymbolTable* table = newTable(upperLevel);
    printf("");

    //alle parameter erzeugen
    while(!parameters->isEmpty) {
        ParameterDeclaration * current = parameters->head;
        parameters = parameters->tail;
        //error checks
        Identifier* name = current->name; //TODO is das richtig
        Type *type = createTypeForTypeExpression(current->typeExpression, table, pos);
        Entry* entry = newVarEntry(type, current->isReference);
        enter(table, name, entry);


    }

    //alle variablen
    while(!variables->isEmpty) {
        VariableDeclaration* current = variables->head;
        variables = variables->tail;
        Type *type = current->typeExpression->dataType;
        //error checks
        Entry *entry = newVarEntry(type, false);

        enter(table, current->name, entry);
    }


    return table;
}

SymbolTable *buildSymbolTable(Program *program, bool showSymbolTables) {
    intType = newPrimitiveType("int", INT_BYTE_SIZE);
    boolType = newPrimitiveType("boolean", BOOL_BYTE_SIZE);

	GlobalDeclarationList *decList = program->declarations;
    SymbolTable *globalTable = initializeGlobalTable();

    while(!decList->isEmpty){
        GlobalDeclaration *current = decList->head;
        decList = decList->tail;
        switch (current->kind) {
            case DECLARATION_TYPEDECLARATION:
                //new Entry for the type declaration
                ;
                Type *type = createTypeForTypeExpression(current->u.typeDeclaration.typeExpression, globalTable, current->position);
                Entry *type_entry = enter(globalTable, current->name, newTypeEntry(type));
                if (type_entry == NULL) {
                    redeclarationAsType(current->position, current->name);
                }
                break;
            case DECLARATION_PROCEDUREDECLARATION:
                //new Entry for the procedure declaration
                ;
                ParameterTypeList* parameterTypeList = createParameterTypeList(current->u.procedureDeclaration.parameters, globalTable, current->position);
                ParameterDeclarationList* parameters = current->u.procedureDeclaration.parameters;
                VariableDeclarationList* variables = current->u.procedureDeclaration.variables;
                //StatementList* body = current->u.procedureDeclaration.body;
                SymbolTable *procedureTable = createProcedureSymbolTable(parameters, variables, globalTable, current->position);
                Entry *proc_entry = enter(globalTable, current->name, newProcEntry(parameterTypeList, procedureTable));
                printSymbolTableAtEndOfProcedure(current->name, proc_entry);
                if(proc_entry == NULL) {
                    redeclarationAsProcedure(current->position, current->name);
                }
                //for every procedure declaration, create a local symbol table
                break;

        }
        
        
        
        
        // type x = intType;
        // type x = intType





    }

    //TODO (assignment 4a): Initialize a symbol table with all predefined symbols and fill it with user-defined symbols
//    notImplemented();
}
