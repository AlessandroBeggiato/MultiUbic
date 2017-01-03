/*
 * Copyright (C) 2015 Alessandro Beggiato <alessandro.beggiato@imtlucca.it>
 * based on the Ubic2 project
 * Copyright (C) 2014 Francesco Burato
 * based on the Ubic project
 * Copyright (C) 2013 Paolo Baldan, Alberto Carraro
 * based on the Cunf project
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

/****************************************************************************/
/* netconv.c                                                                */
/*                                                                          */
/* Functions for creating and modifying Petri net elements		    */
/****************************************************************************/

#include <string.h>

#include "nodelist.h"
#include "unfold.h"
#include "global.h"
#include "debug.h"
#include "ls/ls.h"
#include "glue.h"

#include "netconv.h"

/****************************************************************************/
/* nc_create_net							    */
/* Creates a new net without places or transitions.			    */

void nc_create_net()
{
	ls_init (&u.net.places);
	ls_init (&u.net.trans);
	u.net.numpl = u.net.numtr = 0;
	u.net.t0 = 0;
	u.net.isplain = 1;
}

void nc_create_unfolding()
{
	ls_init (&u.unf.conds);
	ls_init (&u.unf.events);
	u.unf.numcond = 0;
	u.unf.numev = 0;
	u.unf.numh = 0;

	u.unf.numcutoffs = 0;
	u.unf.numgen = 0;
	u.unf.numread = 0;
	u.unf.numcomp = 0;

	u.unf.numr = 0;
	u.unf.nums = 0;
	u.unf.numco = 0;
	u.unf.numrco = 0;
	u.unf.nummrk = 0;
	u.unf.numeblack = 0;
	u.unf.numegray = 0;
	u.unf.numewhite = 0;

	u.unf.numepost = 0;
	u.unf.numecont = 0;
	u.unf.numepre = 0;

	u.unf.e0 = 0;
}

/****************************************************************************/
/* nc_create_{place,transition}						    */
/* Creates a new place or transition in the given net. The new node has no  */
/* incoming or outgoing arcs and is unmarked.				    */

struct place * nc_create_place (void)
{
	struct place * p;
	
	p = gl_malloc (sizeof (struct place));
	ls_insert (&u.net.places, &p->nod);

	p->id = u.net.numpl++;
	al_init (&p->pre);
	al_init (&p->post);
	al_init (&p->cont);
	ls_init (&p->conds);
	p->m = 0;
	p->causal=0;//ALESSANDRO
	return p;
}

struct trans * nc_create_transition (void)
{
	struct trans * t;

	t = gl_malloc (sizeof (struct trans));
	ls_insert (&u.net.trans, &t->nod);

	t->id = ++u.net.numtr;
	al_init (&t->pre);
	al_init (&t->post);
	al_init (&t->cont);
	ls_init (&t->events);
	t->m = 0;
	t->parikhcnt1 = 0;
	t->parikhcnt2 = 0;

	return t;
}

/****************************************************************************/
/* nc_create_arc							    */
/* Create an arc between two nodes (place->transition or transition->place) */

int nc_create_arc (struct al * src_post, struct al * dst_pre,
		void * src, void * dst)
{
	if (al_test (src_post, dst)) return 0;
	al_add (src_post, dst);
	al_add (dst_pre, src);
	return 1;
}

/*****************************************************************************/
void nc_static_checks (const char * stoptr)
{
	struct trans *t;
	struct ls *n;

	ASSERT (u.stoptr == 0);
	for (n = u.net.trans.next; n; n = n->next) {
		t = ls_i (struct trans, n, nod);
		if (t->pre.deg == 0 && t != u.net.t0 && t->post.deg != 0) {
			gl_warn ("%s is not restricted", t->name);
		}
		if (stoptr && !strcmp (t->name, stoptr)) {
			u.stoptr = t;
		}
	}

	if (stoptr && u.stoptr == 0) {
		gl_err ("Transition '%s' not found", stoptr);
	}

	ASSERT (u.net.t0);
	if (u.net.t0->post.deg == 0) gl_err ("No initial marking!");
}

/******************************************************************************/
/*  PAOLO: causal reduction (apply causal reduction to the net read from input)
 *  ALESSANDRO: updated to MDS. While computing the reduction we also check if
 *  there are candidate causal/conflict places
 */
/*******************************************************************************/
int causal_reduction() {
  struct ls * transitions1, * transitions2;	/* for iterating lists of transitions */
  struct ls * orig_transitions;				/* start of the original list of transitions */

  struct place * pH, * ph, * p;
  struct trans * h, * ch;
  struct trans * l, * clh;
  
  int i, j;
  int places = 0;		//this flag is set to 1 if we find a possible causal/conflict place

  /* initialize the starts of the original lists of transitions and
     places: these are used to iterate over the places and transitions
     of the original net. It is safe as new elements are added at the
     beginning of the list ... a bit tricky */
  orig_transitions = u.net.trans.next;
  orig_transitions = orig_transitions->next;// ->next to avoid considering the fake t0 event
  /* create pH */
  pH = nc_create_place ();
  pH->name="p_H";
  
  /* insert pH in the initial marking */
  nc_create_arc(&u.net.t0->post, &pH->pre, u.net.t0, pH); 
  
  /* iterate the list of transitions for transitions h and l
     such that  h -/-> l and there is a place in pre(l) \cap h+
     add a new transition ch and a place ph
     pre(ch) = pre(h)+pH
     post(ch)= post(h)+ph
     for each l insert new clh with
	 pre(clh) = pre(l) + ph
     post(clh) = post(l)
  */
  
  /*DEBUG printf(">>>Causal reduction initiated\n");*/

  /* iterate over all transitions */
  for (transitions1 = orig_transitions; transitions1; transitions1 = transitions1->next) {
    h = (struct trans *) transitions1;
    /*DEBUG printf(">>Considering transition %s as h\n", h->name); */
    /*DEBUG printf(">Causal test\n"); */
    //first we check for candidate causal places. This is not needed for the reduction,
    //but it is a useful efficiency optimization
    for(i = h->post.deg-1; i >=0; --i){
    	p = h->post.adj[i];
    	for(j = p->post.deg-1; j >=0; --j){
    		l = p->post.adj[j];
        	/*DEBUG printf(">Considering transition %s as l\n", l->name);*/
    		if(! FLOW(h->level, l->level) && isInPostPositive(p,h))places = 1;
    	}
    }
    //now we look for candidate conflict places and if we find them we apply the reduction
	/*DEBUG printf(">Conflict test and causal reduction\n");*/
    ph = NULL;
    ch = NULL;
    for(transitions2 = orig_transitions; transitions2; transitions2 = transitions2->next){
    	l = (struct trans *) transitions2;
    	/*DEBUG printf(">Considering transition %s as l\n", l->name);*/
    	if(FLOW(h->level,l->level)) continue;	//nothing to do if h can speak to l
    	//DEBUG printf(">%s cannot flow to %s\n", policy->levelNames[h->level], policy->levelNames[l->level]);
    	//DEBUG printf(">Looking for places in the intersection\n");
       	//if h cannot speak to l we must check for the existence of a place in pre(l) \cap h+
    	for(i = l->pre.deg - 1; i >= 0; --i){	//places in pre(l)
    		p = (struct place *) l->pre.adj[i];
    		if (isInPreNegative(p,h)) {
    			places = 1;	//found a candidate conflict place
    			//DEBUG printf("Place %s is in pre(%s) cap negative(%s)\n", p->name, l->name, h->name);
    			break;  /* exit if an intersection is found */
    		}
    		//DEBUG printf("Place %s is not in pre(%s) cap negative(%s)\n", p->name, l->name, h->name);
    	}
    	if(i>=0){ 	//candidate conflict place between h and l, causal construction needed
    		//DEBUG printf(">Conflict place found, proceeding with construction.\n");
    		if(!ph){	//ph and ch are built only once, even if multiple l are in conflict with h
    			//DEBUG printf(">Building ch and ph...\n");
    			ch = nc_create_transition(); // create a copy of h
    			ch->name = (char *) malloc( sizeof(char)*(2+strlen(h->name)+1));
    			sprintf(ch->name, "c-%s", h->name);
    			ch->level = h->level;
    			ph = nc_create_place (); // create the corresponding place ph
    		    ph->name=(char *) malloc(sizeof(char)*(2+strlen(h->name)+1));
    		    sprintf(ph->name,"p-%s", h->name);
    		    // pre(ch) = pre(h) + pH
    		    // post(ch) = pre(h) + ph
    		    for (i = h->pre.deg - 1; i >= 0; --i){
    		    	p = (struct place *) h->pre.adj[i];
    		    	nc_create_arc(&p->post, &ch->pre, p, ch); 	/* add p to pre of ch */
    		    	nc_create_arc(&ch->post, &p->pre, ch, p); 	/* add p to post of ch */
    		     }
    		     nc_create_arc(&pH->post, &ch->pre, pH, ch); 	/* add pH to pre  of ch */
    		     nc_create_arc(&ch->post, &ph->pre, ch, ph); 	/* add ph to post of ch */
    		     //DEBUG printf(">Done.\n");
    		}//if(!ph)
    		//create a copy of l for h
    		//DEBUG printf(">Building clh...\n");
    	    clh = nc_create_transition ();
    	    clh->name = (char *) malloc(sizeof(char)*(strlen(l->name) + 1 + strlen(h->name) +2 +1));
    	    sprintf(clh->name, "c-%s-%s", l->name, h->name);
    	    clh->level = l->level;
    	    // pre(clh) = pre(l) + ph
    	    for (i = l->pre.deg - 1; i >= 0; --i){
    	    	p = (struct place *) l->pre.adj[i];
    	    	nc_create_arc(&p->post, &clh->pre, p, clh);
    	    }
    	    nc_create_arc(&ph->post, &clh->pre, ph, clh);
    	    // post(clh) = post(l)
    	    for (i = l->post.deg - 1; i >= 0; --i){
    	    	p = (struct place *) l->post.adj[i];
    	    	nc_create_arc(&clh->post, &p->pre, clh, p);
    	    }
    	    //DEBUG printf(">Done.\n");
    	    // FRANCESCO
    	    // cont(tLc) = cont(tL)
    	    // for (i = tL->cont.deg - 1; i >= 0; i--) {
    	    //  p = (struct place *) tL->cont.adj[i];
    	    //  nc_create_arc(&tLc->cont, &p->cont, tLc, p);
    	    // }
    	    // \FRANCESCO
    	}//if(i>=0)
    }//for(transitions2 = orig_transitions; transitions2; transitions2 = transitions2->next)
  }//for(transitions1 = orig_transitions; transitions1; transitions1 = transitions1->next)
  /*DEBUG printf(">>>Causal reduction completed\n"); */
  return places;
}

