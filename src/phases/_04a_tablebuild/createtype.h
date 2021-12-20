#ifndef _CREATETYPE_H
#define _CREATETYPE_H
#include <absyn/absyn.h>
#include <table/table.h>
Type *createTypeForTypeExpression(TypeExpression *e, SymbolTable *table, Position pos);
#endif
