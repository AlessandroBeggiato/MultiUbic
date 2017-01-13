/* 
 * Copyright (C) 2013 Paolo Baldan, Alberto Carraro
 * based on the Cunf project
 * Copyright (C) 2010, 2011 Cesar Rodriguez <cesar.rodriguez@lsv.ens-cachan.fr>
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
/*  PAOLO: causal reduction (apply causal reduction to the net read from input   */
/*********************************************************************************/
void causal_reduction() {
  struct ls * places;	        /* for iterating the list of places */
  struct ls * transitions1, * transitions2;	/* for iterating lists of transitions */
  
  struct ls * orig_places;	/* start of the original list of places */
  struct ls * orig_transitions;	/* start of the original list of transitions */


  struct place * pH, * ph, * p;
  struct trans * tH, * tHc;
  struct trans * tL, * tLc;
  
  int i;

  /* initialize the starts of the original lists of transitions and
     places: these are used to iterate over the places and transitions
     of the original net. It os safe as new elements are added at the
     beginning of the list ... a bit tricky */
  orig_places = u.net.places.next;
  orig_transitions = u.net.trans.next;

  /* create pH */
  pH = nc_create_place ();
  pH->name="p_H";
  
  /* insert pH in the initial marking */
  nc_create_arc(&u.net.t0->post, &pH->pre, u.net.t0, pH); 
  
  /* iterate the list of transitions, and if high h
     add a new high transition hc and a place ph
     pre(hc) = pre(h)
     post(hc)= post(h)+ph
     for each transition l
     if there is a place in pre(l) \cap h+
             insert new lh with
	            pre(lh) = pre(l) + ph
                    post(ph) = post(l)
  */
  
  for (transitions1 = orig_transitions; transitions1; transitions1 = transitions1->next) {
    /* for all transitions, if high ... */
    tH = (struct trans *) transitions1;
    if (tH->name[0]=='h') {
      /* create a copy ... hc (this should be done only if the search
	 below of a tL such that pre{tL} cap negative(tH) <> 0 is
	 successful -> to be optimized ) */
      // DEBUG printf("Examining %s\n", tH->name);
      tHc=nc_create_transition ();
      tHc->name=(char *) malloc(sizeof(char)*(2+strlen(tH->name)+1));
      sprintf(tHc->name, "%s-c", tH->name);
      // DEBUG printf("Created %s\n", tHc->name);
      ph=nc_create_place (); /* create the corresponding place ph */
      ph->name=(char *) malloc(sizeof(char)*(2+strlen(tH->name)+1));
      sprintf(ph->name,"p-%s", tH->name);

      /* pre(tHc) = pre(tH) + pH */ 
      /* post(tHc) = pre(tH) + ph */ 
      for (i = tH->pre.deg - 1; i >= 0; i--) {
	p = (struct place *) tH->pre.adj[i];
	nc_create_arc(&p->post, &tHc->pre, p, tHc); /* add p to pre  of tHc */
	nc_create_arc(&tHc->post, &p->pre, tHc, p); /* add p to post of tHc */
      }

      // FRANCESCO
      // add context to pH
      for (i = tH->cont.deg - 1; i >= 0; i--) {
        p = (struct place *) tH->cont.adj[i];
        nc_create_arc(&tHc->cont,&p->cont,tHc,p); /* add p to the context of tHc */
      }
      // \FRANCESCO

      nc_create_arc(&pH->post, &tHc->pre, pH, tHc); /* add pH to pre  of tHc */
      nc_create_arc(&tHc->post, &ph->pre, tHc, ph); /* add ph to post of tHc */
      

      /* look for low transitions tL such that pre{t2} \cap \negative{t1} */
      for (transitions2 = orig_transitions; transitions2; transitions2 = transitions2->next) {
	tL = (struct trans *) transitions2;
	/* excludes t0 */
	if ((tL->name[0] != 'h') && (tL->name[0] != 'd') && (tL != u.net.t0)) {
	  // DEBUG printf("Examining %s vs %s\n", tH->name, tL->name);
	  /* check if pre{tL} \cap \negative{tH} =! 0 */
	  for (places = orig_places; places; places = places->next) {
	    /* for all places p check ig they belong to  pre{tL} \cap \negative{tH} (maybe a local check is better) */
	    p = (struct place *) places;
	    if (al_test(&tL->pre,p) 
		&& al_test(&tH->pre,p) 
		&& ! al_test(&tH->post,p)) {
	      // DEBUG printf("Place %s is in pre(%s) cap negative(%s)\n", p->name, tL->name, tH->name);
	      break;  /* exit if an intersection is found */
	    }
	    // DEBUG else 
	    // DEBUG printf("Place %s is not in pre(%s) cap negative(%s)\n", p->name, tL->name, tH->name );
	  }	  
	  if (places) {
	    /* if the intersection is not empty add a copy of tL */
	    tLc=nc_create_transition ();
	    tLc->name=(char *) malloc(sizeof(char)*(strlen(tL->name) + 1 + strlen(tH->name) +2+1));
	    sprintf(tLc->name, "%s-%s-c", tL->name, tH->name);
	    
	    //printf("Examining %s vs %s: success creating %s\n", tH->name, tL->name, tLc->name);
	    /* pre(tLc) = pre(tL) + ph */ 
	    for (i = tL->pre.deg - 1; i >= 0; i--) {
	      p = (struct place *) tL->pre.adj[i];
	      nc_create_arc(&p->post, &tLc->pre, p, tLc);
	    }
	    nc_create_arc(&ph->post, &tLc->pre, ph, tLc);

	    /* post(tLc) = post(tL) */ 
	    for (i = tL->post.deg - 1; i >= 0; i--) {
	      p = (struct place *) tL->post.adj[i];
	      nc_create_arc(&tLc->post, &p->pre, tLc, p);
	    }

	    // FRANCESCO
	    // cont(tLc) = cont(tL)
	    for (i = tL->cont.deg - 1; i >= 0; i--) {
	      p = (struct place *) tL->cont.adj[i];
	      nc_create_arc(&tLc->cont, &p->cont, tLc, p);
	    }
	    // \FRANCESCO
	  }
	}
      }
    }
  }
}

