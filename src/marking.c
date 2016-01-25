/* 
 * Copyright (C) 2014 Francesco Burato
 * based on the Ubic project
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

#include "nodelist.h"
#include "marking.h"
#include "global.h"
#include "ls/ls.h"
#include "debug.h"
#include "glue.h"
#include "h.h"
#include "stack.h"

// FRANCESCO
#include "exm.h"

/*
 * Given a condition c used to fire the event h->e
 * determines whether c->ft is a weak causal place or not
 * and sets c->is_weak_causal accordingly
 */
/*
 * ALESSANDRO: old function for three level contextual nets
 *
 void _find_weak_causal(struct h * h, struct cond * c) {
  int cont, i;
  struct h * min_hist_he, *min_hist_condition, *hp1,
      *hp2;
  struct stack * s= 0;
  // find the minimal history of c->pre which has
  // min_hist_condition->e \equiv c->pre
  min_hist_condition = h_find_minimal_history(c->pre);
  ASSERT(min_hist_condition != 0);
  //
   // if the projection of c is present in high_list then
   // c->pre is an high transition with positive contribution to c->pre
   // and since h->e is low, c->fp is a weak causal place unless there is a
   // downgrading transition d' present in [l'] s.t. c->pre is in asymmetric conflict
   // with d' i.e. h' is present in the rest of the history of d'
   //
  if (nl_test(min_hist_condition->high_list, c->fp)) {
    // find the minimal history associated to h->e which is the low transition
    min_hist_he = h_find_minimal_history(h->e);
    // min_hist_he = [l'], c->pre = h'
    ASSERT(min_hist_he != 0);
    cont = 1;
    // for each event in the min_hist_he look for downgrading transition
    // and find out if they are in min_hist_condition and caused
    // by c->pre \equiv min_hist_condition->e
    s = stack_alloc();
    push(s,min_hist_he);
    while(!isEmpty(s) && cont) {
      // extract history from stack
      hp1 = pop(s);
      if(hp1->e->ft->name[0] == 'd' && find_event_in_history(c->pre,hp1)) {
 	 	 //
         // if hp1->e is a downgrading transition in [l']
         // and h'=c->pre is in the history of hp1->e then
         // the place has been downgraded and search can stop
         //
        cont = 0;
      } else
        //
        // otherwise continue with the search for downgrading transitions
        // in the rest of [l']
        //
        // put rest of the history in the stack
        for(i = 0 ; i < hp1->nod.deg; ++i) {
          hp2 = (struct h *) hp1->nod.adj[i];
          push(s,hp2);
        }
    }
    //clean up the stack
    stack_destroy(s);
    if (cont)
      // if cont == 1 there is not a downgrading transition between condition->pre
      // and he->e so a weak causal place has been found
      // c is a weak causal place
      c->is_weak_causal = 1;
  }
}*/

char _compare_dextended_markings(struct nl * de1, struct nl * de2) {
  struct nl * node;
  // look for all the elements in the first list in the second list
  node = de1;
  while(node && nl_find_custom(de2,node->node,exm_compartor))
    node = node->next;
  if(node)
    // if node != 0 then there is a difference -> return 1
    return 1;
  // here all nodes of the first list are present in the second
  // it is necessary to check the opposite
  node = de2;
  while(node && nl_find_custom(de1,node->node,&exm_compartor))
    node = node->next;
  if(node)
    // if node != then there is a difference -> return -1
    return -1;
  // the downgrading markings are the same -> return 0
  return 0;
}
// \FRANCESCO

struct hash_entry {
  struct ls nod;
  struct h *h;
};

struct hash {
  struct ls *tab;
  int size;
};

struct hash hash;

int marking_hash(const struct nl *l) {
  unsigned int val, i;

  /* compute the hash value of a marking */
  val = 0;
  i = 1;
  for (; l; l = l->next) {
    val += ((const struct place *) (l->node))->id * i;
    i++;
  }
  return val % hash.size;
}

void marking_init(void) {
  int i;

  /* initialize the hash table; h.size is the size of the array h.tab;
   * h.tab is a vector of lists; h.tab[i] is a singly-linked list storing
   * all the pairs (marking,history) with the same marking */

  /* FIXME use prime numbers! */
  hash.size = 1 + u.net.numpl * 800;
  hash.tab = gl_malloc(hash.size * sizeof(struct ls));
  for (i = hash.size - 1; i >= 0; i--)
    ls_init(hash.tab + i);
}

int marking_find(const struct h *h) {
  struct hash_entry *he;
  struct ls *n;
  int ret;
  int ret_h; /* PAOLO */

  /* if the marking h->marking is in the hash table, it is somewhere in
   * the list h.tab[hash(h->marking)]; return the logical condition of
   * h->marking to be in the table
   */

  ASSERT (marking_hash (h->marking) == h->hash);
  n = hash.tab + h->hash;
  for (n = n->next; n; n = n->next) {
    he = ls_i (struct hash_entry, n, nod);
    ret = nl_compare(he->h->marking, h->marking);
    if(ret == 0)
    	ret_h = nl_compare2(he->h->Lambda, h->Lambda, cmpExtPlace2);
    // FRANCESCO
    if ((ret == 0) && (ret_h == 0) && (_compare_dextended_markings(he->h->downgrading_marking,h->downgrading_marking) == 0))
      return 1;
    //\FRANCESCO
  }

  return 0;
}

/*
 * @param h a pointer to a possible extension
 *
 * searches for another history in the hash table with the same marking
 * if it finds one, let it be h'; it then calls h_cmp to compare h and h'
 * if the history is not a cutoff, it is inserted in the table
 *
 */
void marking_add(struct h *h) {
	D3(printf("marking_add(%s(%d))\n", h->e->ft->name, h->e->id);)

  struct hash_entry *he, *nhe;
  struct ls *buckl, *n;
  int ret;

  ASSERT (h);
  ASSERT (h->marking);
  ASSERT (h->corr != 0); /* histories always initialized as cutoffs */

  /* add the marking h->marking to the hash table if there is no
   * other history h' such that h->marking = h'->marking; if such h'
   * exists, then h is a cutoff
   */

  // determine if the marking h->marking is in the hash table
  ASSERT (marking_hash (h->marking) == h->hash);
  buckl = hash.tab + h->hash;
  D4(printf("cutoff?\n");)
  for (n = buckl->next; n; n = n->next) {	//for each entry with the same hash of h
	  he = ls_i (struct hash_entry, n, nod);
	  D4(printf("same marking?\n");)
	  ret = nl_compare(he->h->marking, h->marking);	//test if they have the same marking
	  // ALESSANDRO: new code
	  // ALESSANDRO: if they do, test if they also have the same extended marking (i.e. same Lambda)
	  if(ret==0){
		  D4(printf("same lambda?\n");)
		  ret = nl_compare2(he->h->Lambda, h->Lambda, cmpExtPlace2);
	  }
	  //ALESSANDRO: if they do and we are checking BINI, test also if they have the same delta and kappa
	  if (ret == 0 &&  !policy->isTransitive){
		  D4(printf("same delta?\n");)
		  ret = nl_compare2(he->h->delta, h->delta, cmpRelation2);
		  if(ret ==0){
			  D4(printf("same kappa?\n");)
			  ret = nl_compare2(he->h->kappa, h->kappa, cmpRelation2);
		  }
	  }
	  if(ret == 0)  {
		  D3(printf("cutoff because of %s(%d)\n", he->h->e->ft->name, he->h->e->id);)
		  break;
	  }
  }

#ifdef CONFIG_MCMILLAN
  /* if it is the case, and the corresponding history is smaller,
   * according to the McMillan's order, then it is a cutoff */
  if (n && h_cmp (he->h, h) < 0) {
#else
  /* if present in the hash table, then we know h is a cutoff, as we are
   * sure that the corresponding history is smaller (the ERV order is
   * total) */
  if (n) {
    // FIXME -- this doesn't work!!! ASSERT (h_cmp (he->h, h) < 0);
    ASSERT (h_cmp (he->h, h) <= 0);
#endif

    /* deallocate the marking and set history h as a cutoff, whose
     * corresponding history is he->h */
    ASSERT (he);
    D4(printf("cleaning up marking\n");)
    nl_delete(h->marking);
    D4(printf("cleaning up Lambda\n");)
    nl_delete(h->Lambda); /* PAOLO */
    /* ALESSANDRO */
    //D4(printf("cleaning up frontier\n");)
    //nl_delete(h->frontier);
    //TODO FIXME deallocate in a clean way kappa and delta, frontier is still needed
    // FRANCESCO
    D4(printf("cleaning up downgrading_marking\n");)
    nl_freememory(h->downgrading_marking);
    // \FRANCESCO
    h->marking = 0;
    h->corr = he->h;
    u.unf.numcutoffs++;
    D4(printf("not adding it\n");)
    return;
  }
  D3(printf("not a cutoff, adding it to the table\n");)
  /* otherwise, insert h into the table */
  h->corr = 0;
  nhe = gl_malloc(sizeof(struct hash_entry));
  nhe->h = h;
  ls_insert(buckl, &nhe->nod);

}

static void _marking_print(const struct nl *l) {
  if (!l)
    return;

  PRINT(" %s", ((const struct place* ) (l->node))->name);
  _marking_print(l->next);
}

void marking_print(const struct h *h) {
  _marking_print(h->marking);
}

#if 0
void __debug (void)
{
  struct ls *n;
  struct hash_entry *he;
  int i;

  for (i = 0; i < hash.size; i++) {
    for (n = hash.tab[i].next; n; n = n->next) {
      he = ls_i (struct hash_entry, n, nod);
      marking_print (he->h);
      PRINT (" h%d/e%d:%s size %d depth %d\n", he->h->id, he->h->e->id,
          he->h->e->ft->name, he->h->size, he->h->depth);
    }
  }
}
#endif

