/*
 * Phun Interpreter
 * Evaluator Code
 * Tami Meredith, July 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "phun.h"

#define SUM  0
#define DIFF 1
#define PROD 2
#define DIV  3

/*
 * Evaluate the arguments for specific keywords and produce an expression
 */
expr *doQuote(exprs *ls) {
    expr *val;

    if (ls == NULL)
        fatalError("Missing value for quote");
    val = ls->e;
    if (ls->n != NULL)
        fatalError("Extra expressions in a quote");
    /* Quoted, don't evaluate */
    return (val);
}

expr *doCar(exprs *ls) {
    expr *head;
    if (ls == NULL)
        fatalError("Missing argument for car");
    head = eval(ls->e);
    if (head->type != eExprList)
        fatalError("car not used on a list");
    return (head->eVal->e);
}

expr *doCdr(exprs *ls) {
    expr *tail;
    if (ls == NULL)
        fatalError("Missing argument for cdr");
    tail = eval(ls->e);
    if (tail->type != eExprList)
        fatalError("cdr not used on a list");
    return (newListExpr(tail->eVal->n));
}

expr *doCons(exprs *ls) {
    expr *head, *tail;
    if (ls == NULL)
        fatalError("Missing values for cons");
    head = eval(ls->e);
    ls = ls->n;
    if (ls == NULL)
        fatalError("Missing second value for cons");
    tail = eval(ls->e);
    if (tail->type != eExprList)
        fatalError("Second value for cons is not a list");

    return (newListExpr (newExprList (head, tail->eVal)));
}

expr *doList(exprs *ls) {
    expr *head, *tail;
    if (ls == NULL)
        return (newListExpr (ls));
    head = eval (ls->e);
    tail = doList(ls->n);
    if (tail == NULL)
        return (newListExpr (newExprList (head, NULL)));
    else
        return (newListExpr (newExprList (head, tail->eVal)));
}

expr *doDefine(exprs *ls) {
    expr *name;
    expr *val;
    symbol *s;
	function *f = NULL;
    if (ls == NULL)
        fatalError("Definition missing name and value");
    name = ls->e;
    if (name->type != eIdent)
        fatalError("Missing identifier in a definition");
	if (ls->n->e->type == eExprList
		&& !strcmp(ls->n->e->eVal->e->sVal, "lambda")) {
		name = ls->n->e->eVal->e;
		doLambda(ls);
	}
    ls = ls->n;
    if (ls == NULL)
        fatalError("Definition missing a value");
    val = ls->e;
    if (ls->n != NULL)
        fatalError("Extra expressions in a definition");
    val = eval(val);
    s = lookup(name->sVal);
	f = functionLookup(name->sVal);
    if (s != NULL) {
        s->data = val;
    } else if(f!=NULL){
		evalLambda(f, ls);
	} else {
        bind(name->sVal,val);
    }
    return(NULL); /* don't want main to print anything, so return NULL */
}



function *doLambda(exprs *ls) {
	function *f;

	expr *params;
	expr *data;
	char *name;

	name = ls->e->sVal;

	ls = ls->n->e->eVal->n;

	if (ls->e->eVal != NULL) {
		params = ls->e;
	}
	else {
		fatalError("no arguments provided");
	}
	if (ls->n != NULL) {
		data = ls->n->e;
	}
	else {
		fatalError("function body is empty");
	}

	f = functionBind(name, params, data);

	return(f); /* don't want main to print anything, so return NULL */
}

expr *evalLambda(function * f, exprs *ls)
{
	exprs *paramsPointer = f->params->eVal;
	symbol *s;
	function *innerF;

	while (paramsPointer != NULL) {
		if (ls == NULL) {
			fatalError("too few arguments");
		}
		if (ls->e->type == eInt || ls->e->type == eString) {
			bind(paramsPointer->e->sVal,ls->e);
		}
		if (ls->e->type == eIdent) {
			s = lookup(ls->e->sVal);
			if (s != NULL) {
				bind(paramsPointer->e->sVal,eval(ls->e));
			}
			else
			{
				innerF = functionLookup(ls->e->sVal);
				if (innerF != NULL) {
					bind(paramsPointer->e->sVal, evalLambda(innerF,ls));
				}
				else
				{
					fatalError("couldn't evaluate");
				}
			}
		}
		ls = ls->n;
		paramsPointer = paramsPointer->n;
	}
	
	return eval(f->data);
}


/*
 * Evaluate the arguments for binary+ operators and then apply the operation
 * - repeat as necessary for 3+ arguments in the list
 */
expr *doBinaryOp(int op, exprs *ls) {
    expr *val1, *val2;
    int   i, j, k;
    if (ls == NULL)
        fatalError("Missing values for binary operator");
    val1 = eval(ls->e);
    if (val1->type != eInt)
        fatalError("First value for binary operation is not an int");
    i = val1->iVal;
getAnother:
    ls = ls->n;
    if (ls == NULL)
        fatalError("Missing second+ value for binary operation");
    val2 = eval(ls->e);
    if (val2->type != eInt)
        fatalError("Second+ value for binary operation is not an int");
    j = val2->iVal;
    switch (op) {
        case SUM:  k = i + j; break;
        case DIFF: k = i - j; break;
        case PROD: k = i * j; break;
        case DIV:  k = i / j; break;
        default:
            fatalError("Unknown error: Applying invalid operation");
    }
    i = k;
    if (ls->n != NULL) goto getAnother;
    return (newIntExpr(k));
}

/*
 * Evaluate an Expression
 */
expr *eval(expr *e) {
    expr   *op;
    exprs  *list;
    symbol *s;
	function *f;

    switch (e->type) {
        case eString:
            /* Not needed in assignment 6 */
            /* Fall through */
        case eInt:
            return (e);
            break;
        case eIdent:
            s = lookup(e->sVal);
            if (s == NULL)
                fatalError("Unbound symbol");
            return (s->data);
            break;
        case eExprList:
            list = e->eVal;
            op = list->e;
            list = list->n;
            if (op->type != eIdent) {
                fatalError("Invalid operator in function application");
            }
            if (!strcmp (op->sVal,"define")) {
                return (doDefine(list));
			}
			else if (!strcmp(op->sVal, "lambda")) {
				return NULL;
			}
			else if (!strcmp(op->sVal, "car")) {
				return (doCar(list));
			}
			else if (!strcmp (op->sVal,"cdr")) {
                return (doCdr(list));
            } else if (!strcmp (op->sVal,"cons")) {
                return (doCons(list));
            } else if (!strcmp (op->sVal, "quote")) {
                return (doQuote(list));
            } else if (!strcmp (op->sVal,"list")) {
                return (doList(list));
            }  else if (!strcmp (op->sVal,"+")) {
                return (doBinaryOp(SUM,list));
            } else if (!strcmp (op->sVal,"-")) {
                return (doBinaryOp(DIFF,list));
            } else if (!strcmp (op->sVal,"*")) {
                return (doBinaryOp(PROD,list));
            } else if (!strcmp (op->sVal,"/")) {
                return (doBinaryOp(DIV,list));
            } else {
				f = functionLookup(op->sVal);
				if (f != NULL) {
					evalLambda(f,list);
				}
				else {
					fatalError("Unbound operator in function application");
				}
            }
            break;
        default:
            break;
    }
}

/*
 * Print expressions in the format needed by the interpreter
 */
void exprPrint(expr *e) {
    switch (e->type) {
        case eString:
            /* fall through */
        case eIdent:
            printf("%s", e->sVal);
            break;
        case eInt:
            printf("%d", e->iVal);
            break;
        case eExprList:
            printf("(");
            listPrint(e->eVal);
            printf(")");
            break;
        default:
            break;
    }
}

void listPrint(exprs *l) {
    if (l == NULL) return;
    exprPrint(l->e);
    if (l->n != NULL) {
      printf(" ");
      listPrint(l->n);
    }
}

/* end of eval.c */
