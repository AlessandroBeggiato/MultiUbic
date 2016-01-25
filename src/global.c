/*
 * global.c
 *
 *  Created on: Dec 18, 2015
 *      Author: alessandro
 */

#include "global.h"

/* ALESSANDRO: p \in t- ? */
int isInPreNegative(struct place * p, struct trans * t){
	return al_test(&t->pre, p) && !al_test(&t->post, p);
}
/* ALESSANDRO: p \in t+ ? */
int isInPostPositive(struct place * p, struct trans * t){
	return al_test(&t->post, p) && !al_test(&t->pre, p);
}
