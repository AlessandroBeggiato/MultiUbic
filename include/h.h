
/* 
 * Copyright (C) 2014 Francesco Burato
 * based on the Ubic project
 * Copyright (C) 2010, 2011  Cesar Rodriguez <cesar.rodriguez@lsv.ens-cachan.fr>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _H_H_
#define _H_H_

#include "nodelist.h"
#include "dls/dls.h"
#include "global.h"
#include "al/al.h"
// #include "glue.h"
#include "config.h"

struct parikh {
	struct trans *t;
	int count;
};

struct h {
	int id;			/* internal history identifier */
	struct event * e;	/* h is associated to this event */
	struct al nod;		/* adjacent histories */
	struct dls auxnod; 	/* FIXME -- do we really need this? */
	struct al ecl;		/* enriched conditions conforming h */

	int m;			/* general purpose mark */
	int size;		/* number of events in the history */
	int depth;		/* 1 + max(h->nod.adj[i]->depth) */

	struct al rd;		/* events in h reading the cut of h */
	struct al sd;		/* events in h reading from pre(h->e) */
	struct nl *marking;	/* marking associated to the history */
	struct nl *Lambda;  /* ALESSANDRO: levels of the places in the associated marking */
	struct nl * delta;	/* ALESSANDRO: downgrading relation, a list of struct relation* */
	struct nl * kappa;	/* ALESSANDRO: kinfolk relation, a list of struct relation* */
	struct nl * frontier;	/* ALESSANDRO: I memoize also the conditions in the frontier, minus those in post(h->e) */
	int hash;			/* hash value of the marking (see marking_hash) */
	struct h *corr;		/* if h is cutoff, corresponding history */
	struct {
		int size;
		struct parikh *tab;
	} parikh;		/* we need this to compute the <_F order */
  // FRANCESCO
  //remember the conditions which constitutes the extended marking
  struct nl * downgrading_marking;
  // \FRANCESCO
#ifdef CONFIG_DEBUG
	int debugm;
	struct dls debugnod;
#endif
};

void h_init (void);
void h_term (void);
struct h * h_alloc (struct event * e);
void h_free (struct h *h);

void h_add (struct h * h, struct h * hp);

/*
*
* computes the marking of a history h and stores it in h->marking
* computes the extended marking, storing \lambda p for each p \in h->marking in h->Lambda
* computes a hash value from h->marking and stores it in h->hash; it will later be used in marking_add
* computes the parikh vector of transitions, necessary to compute <_F in [ERV02]
* computes two lists h->rd, h->sd, of interest only for contextual unfoldings;
*
*/
void h_marking (struct h *h);
void h_list (struct dls *l, struct h *h);
int h_cmp (struct h *h1, struct h *h2);
// FRANCESCO
/*
 * Find the history minimal in size for the event e
 */
struct h * h_find_minimal_history(struct event * e);
int find_event_in_history(struct event * e, struct h * h);
// \FRANCESCO

/*
 * ALESSANDRO: structures needed for the extended marking, the intransitve marking, and
 * the algorithmic intransitive marking
 */
struct extendedPlace {	// Function Lambda of the extended marking is a list of these
	struct place * p;
	int lvl;
};
/*
 * ALESSANDRO: comparator for extended places: compares the pointers to the place
 */
int cmpExtPlace1(const void * ep1, const void * ep2);
/*
 * ALESSANDRO: compares the pointers to the place and if they are equal compares levels
 */
int cmpExtPlace2(const void * ep1, const void * ep2);

struct relation{		// This structure is used encode both delta and k
	struct place * p;
	struct nl * Q;	// Q = {q | pRq}
};

/*
 * ALESSANDRO: comparator for struct relation: compares the pointers to the place
 */
int cmpRelation1(const void * r1, const void * r2);
/*
 * ALESSANDRO: compares the pointers to the place and if they are equal compares their Qs
 */
int cmpRelation2(const void * r1, const void * r2);

/*
 * ALESSANDRO: utility functions to make the code clearer
 */

// ALESSANDRO: h<t?
// side effect: if there exist d. h<d<=t such that h.level ~> d.level, absorbing is set to 1
int strictlyCausallyPrecedes(struct event * h, struct event * t, int * absorbing);

//ALESSANDRO: p->fp delta q->fp ?
int absorbs(struct cond* p, struct cond* q);

#endif

