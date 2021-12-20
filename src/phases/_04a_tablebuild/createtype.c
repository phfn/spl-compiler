#include <types/types.h>
#include <absyn/absyn.h>
#include <table/table.h>
#include <util/errors.h>

Type *createTypeForTypeExpression(TypeExpression *e, SymbolTable *table, Position pos) {
    switch (e->kind) {
        case TYPEEXPRESSION_NAMEDTYPEEXPRESSION:;
            Identifier *identifier = e->u.namedTypeExpression.name;

            // gibt es den Typ überhaupt?
            Entry *lookedUpEntry = lookup(table, identifier);
            if (lookedUpEntry == NULL) {
                undefinedType(pos, identifier);// wenn als typ ein nicht existierender typ angegeben wurde
            }
            if (lookedUpEntry->kind != ENTRY_KIND_TYPE) {
                notAType(pos, identifier); //wenn als typ ein prozedurname angegeben wurde
            }
            return lookedUpEntry->u.typeEntry.type;
            break;

        case TYPEEXPRESSION_ARRAYTYPEEXPRESSION:
            return newArrayType(e->u.arrayTypeExpression.arraySize,
                                createTypeForTypeExpression(e->u.arrayTypeExpression.baseType, table, pos));
            break;
    }
}
