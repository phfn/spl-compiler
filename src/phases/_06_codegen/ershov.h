//calculate ershov-number
#ifndef _ERSHOV_H
#define _ERSHOV_H

#include "absyn/expressions.h"

int calc_ershov(Expression *exp);
int calc_ershov_var(Variable *var);

#endif
