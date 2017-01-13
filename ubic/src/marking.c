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
 * Given a condition c used to fire the low event h->e
 * determines whether c->ft is a weak causal place or not
 */
void _find_weak_causal(struct h * h, struct cond * c) {
  int cont, i;
  struct h * min_hist_he, *min_hist_condition, *hp1,
      *hp2;
  struct stack * s= 0;
  // find the minimal history of c->pre which has
  // min_hist_condition->e \equiv c->pre
  min_hist_condition = h_find_minimal_history(c->pre);
  ASSERT(min_hist_condition != 0);
  /*
   * if the projection of c is present in high_list then
   * c->pre is an high transition with positive contribution to c->pre
   * and since h->e is low, c->fp is a weak causal place unless there is a
   * downgrading transition d' present in [l'] s.t. c->pre is in asymmetric conflict
   * with d' i.e. h' is present in the rest of the history of d'
   */
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
        /*
         * if hp1->e is a downgrading transition in [l']
         * and h'=c->pre is in the history of hp1->e then
         * the place has been downgraded and search can stop
         */
        cont = 0;
      } else
        /*
         * otherwise continue with the search for downgrading transitions
         * in the rest of [l']
         */
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
      /* c is a weak causal place */
      c->is_weak_causal = 1;
  }
}

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

  /* if the marking h->marking is in the hash table, it is somewere in
   * the list h.tab[hash(h->marking)]; return the logical condition of
   * h->marking to be in the table
   */

  ASSERT (marking_hash (h->marking) == h->hash);
  n = hash.tab + h->hash;
  for (n = n->next; n; n = n->next) {
    he = ls_i (struct hash_entry, n, nod);
    ret = nl_compare(he->h->marking, h->marking);
    /* PAOLO: compare also high list */
    ret_h = nl_compare(he->h->high_list, h->high_list);
    // FRANCESCO
    if ((ret == 0) && (ret_h == 0) && (_compare_dextended_markings(he->h->downgrading_marking,h->downgrading_marking) == 0))
      return 1;
    //\FRANCESCO
  }

  return 0;
}

/* PAOLO: changed to return int 1 if the net is found to be unsecure */
int marking_add(struct h *h) {
  struct hash_entry *he, *nhe;
  struct ls *buckl, *n;
  int ret;
  /* PAOLO */
  int found_weak_causal_pre, found_weak_causal_cont; /* found any weak causal place? */
  int ret_h; /* return value for high marking comparison */
  // FRANCESCO
  int i;
  //struct h *h1;
  // \FRANCESCO
  struct cond *c;

  found_weak_causal_pre = found_weak_causal_cont = 0;
  /* PAOLO */

  ASSERT (h);
  ASSERT (h->marking);
  ASSERT (h->corr != 0); /* histories always initialized as cutoffs */

  /* h->corr = 0;
   return; */

  /* add the marking h->marking to the hash table if there is no
   * other history h' such that h->marking = h'->marking; if such h'
   * exists, then h is a cutoff; the function returns 1 iff h is a cutoff
   */

  /* determine if the marking h->marking is in the hash table */
  ASSERT (marking_hash (h->marking) == h->hash);
  buckl = hash.tab + h->hash;
  for (n = buckl->next; n; n = n->next) {
    he = ls_i (struct hash_entry, n, nod);
    ret = nl_compare(he->h->marking, h->marking);
    // ### IMPORTANT: HERE EXTENDED MARKING IS COMPUTED
    /* PAOLO: compare also high list */
    ret_h = nl_compare(he->h->high_list, h->high_list);
    // FRANCESCO
    // compare the downgrading extended markings
    if ((ret == 0) && (ret_h == 0) && (_compare_dextended_markings(he->h->downgrading_marking,h->downgrading_marking) == 0))
      break;
    //\FRANCESCO
    // ### /IMPORTANT
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
    // FIXME -- this deosn't work!!! ASSERT (h_cmp (he->h, h) < 0);
    ASSERT (h_cmp (he->h, h) <= 0);
#endif

    /* deallocate the marking and set history h as a cutoff, whose
     * corresponding history is he->h */
    ASSERT (he);
    nl_delete(h->marking);
    nl_delete(h->high_list); /* PAOLO */
    // FRANCSCO
    nl_freememory(h->downgrading_marking);
    // \FRANCESCO
    h->marking = 0;
    h->corr = he->h;
    u.unf.numcutoffs++;
    return 0; /* PAOLO: added return value */
  }

  /* otherwise, insert h into the table */
  h->corr = 0;
  nhe = gl_malloc(sizeof(struct hash_entry));
  nhe->h = h;
  ls_insert(buckl, &nhe->nod);

  // HERE IT IS TESTED PRESENCE OF WEAK CAUSAL PLACES
  /* PAOLO: check if causal interference and, if the prefix is
   not required to be complete, exit immediately */
  // FRANCESCO
  if (h->e->ft->name[0] != 'h' && h->e->ft->name[0] != 'd') {
    for (i = h->e->pre.deg - 1; (!found_weak_causal_pre || !u.complete ) && i >= 0; i--) {
      c = (struct cond *) h->e->pre.adj[i];
      _find_weak_causal(h, c);
      found_weak_causal_pre = found_weak_causal_pre || c->is_weak_causal;
      // \FRANCESCO
      // PAOLO: if the net is found to be unsecure and a
      // complete prefix is not required, return directly
      if (!u.complete && found_weak_causal_pre) {
        printf("Found a weak causal place: %s:c%d\n", c->fp->name, c->id);
        return 1;
      }
    }
    for (i = h->e->cont.deg - 1; (!found_weak_causal_cont || !u.complete ) && i >= 0; i--) {
      c = (struct cond *) h->e->cont.adj[i];
      _find_weak_causal(h, c);
      found_weak_causal_cont = found_weak_causal_cont || c->is_weak_causal;
      // PAOLO: if the net is found to be unsecure and a
      // complete prefix is not required, return directly
      if (!u.complete && found_weak_causal_cont) {
        printf("Found a weak causal place: %s:c%d\n", c->fp->name, c->id);
        return 1;
      }
    }
    // \FRANCESCO
  }

  /* PAOLO: return value */
  return found_weak_causal_pre || found_weak_causal_cont;
}

static void _marking_print(const struct nl *l) {
  if (!l)
    return;
  _marking_print(l->next);

  PRINT(" %s", ((const struct place* ) (l->node))->name);
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

