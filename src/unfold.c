
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

#include "netconv.h"
#include "marking.h"
#include "global.h"
#include "ls/ls.h"
#include "al/al.h"
#include "da/da.h"
#include "debug.h"
#include "glue.h"
#include "pe.h"
#include "ec.h"

#include "unfold.h"

static struct cond * _unfold_new_cond (struct event *e, struct place *p)
{
	D3(printf("_unfold_new_cond (%s(%d), %s)\n", e->ft->name, e->id, p->name);)
	struct cond *c;
	struct h* h;		//ALESSANDRO
	struct nl* aux;

	/* mallocate a new structure */
	c = gl_malloc (sizeof (struct cond));

	/* initialize structure's fields */
	ls_insert (&u.unf.conds, &c->nod);
	c->pre = e;
	al_init (&c->post);
	al_init (&c->cont);
	c->fp = p;
	ls_insert (&p->conds, &c->pnod);
	ls_init (&c->ecl);
	c->m = 0;
	//ALESSANDRO: moved is_weak_causal to struct place
	//c-> is_weak_causal = 0; /* PAOLO: initially not a weak causal place */
	c->absorbs = 0;	/* ALESSANDRO: initially absorbs nothing */
	/* also the condition number */
	c->id = u.unf.numcond++;

	/* finally, update the preset of c, the postset of e and append the
	 * condition to the list p->conds only if e is not a cutoff */
	c->pre = e;
	al_add (&e->post, c);

	/*ALESSANDRO: now I update the list of conditions absorbed by c */
	h = e->hist.adj[0];		//fetch the history of e FIXME if we move to contextual nets this has to be changed
	aux = h->frontier;
	while(aux){
		if(absorbs(c, aux->node)){
			nl_insert(&(c->absorbs), aux->node);
		}
		aux = aux->next;
	}
	return c;
}

static void _unfold_postset (struct event *e)
{
	D3(printf("_unfold_postset (%s(%d))\n", e->ft->name, e->id);)
	struct trans *t;
	struct place *p;
	int i;

	ASSERT (e);
	ASSERT (e->ft);

	/* if the output degree of the post node in e is the same as in the
	 * post node in e->origin, postset conditions are already unfolded;
	 * otherwise we create a new condition for each place */

	t = e->ft;
	if (e->post.deg == t->post.deg) return;

	for (i = t->post.deg - 1; i >= 0; i--) {
		p = (struct place *) t->post.adj[i];
		_unfold_new_cond (e, p);
	}
}

static void _unfold_combine (register struct ec *r)
{
	register struct ec *rp;
	register int i, j;
	static struct da l;
	static int init = 1;

	ASSERT (r);
	ASSERT (r->c);

	/* compute those rp from rco(r) which must be combined with r */
	if (init) {
		init = 0;
		da_init (&l, struct ec *);
	}
	j = 0;
	for (i = r->rco.deg - 1; i >= 0; i--) {
		rp = (struct ec *) r->rco.adj[i];
		if (! ec_included (r, rp)) da_push (&l, j, rp, struct ec *);
	}

	for (j--; j >= 0; j--) {
		u.unf.numcomp++;
		rp = ec_alloc2 (r, da_i (&l, j, struct ec *));
		ec_conc (rp);
		pe_update_read (rp);
	}

	/* !!! */
	al_term (&r->rco);
}

static void _unfold_enriched (struct h *h)
{
	struct event *e;
	struct cond *c;
	struct ec *r;
	int i;

	ASSERT (h);
	ASSERT (h->corr == 0);
	ASSERT (h->e);
	ASSERT (h->e->ft);
	ASSERT (h->e->iscutoff == 0);
	ASSERT (h->e->post.deg == h->e->ft->post.deg);

	/* if the prefix must not go beyond some depth and h already has that
	 * depth, then no history generated using h need to be inserted */
	if (u.depth && h->depth >= u.depth) return;

	/* append a new enriched condition r for each c in post(e), compute the
	 * concurrency relation for r and use r to update pe with new possible
	 * extensions */
	e = h->e;
	for (i = e->post.deg - 1; i >= 0; i--) {
		c = (struct cond *) e->post.adj[i];
		if (c->fp->post.deg + c->fp->cont.deg == 0) continue;

		r = ec_alloc (c, h);
		u.unf.numgen++;
		ec_conc (r);
		pe_update_gen (r);
	}

	/* then, do the same with cont(e) :) */
	for (i = e->cont.deg - 1; i >= 0; i--) {
		c = (struct cond *) e->cont.adj[i];
		if (c->fp->post.deg == 0) continue;
		r = ec_alloc (c, h);
		u.unf.numread++;
		ec_conc (r);
		pe_update_read (r);
		_unfold_combine (r);
	}
}

static void _unfold_init (void)
{
	struct event *e0;
	struct h *h0;

	/* allocate and initialize initial event */
	e0 = gl_malloc (sizeof (struct event));
	ls_insert (&u.unf.events, &e0->nod);
	al_init (&e0->pre);
	al_init (&e0->post);
	al_init (&e0->cont);
	al_init (&e0->ac);
	al_init (&e0->hist);
	e0->ft = u.net.t0;
	ls_insert (&u.net.t0->events, &e0->tnod);
	e0->id = u.unf.numev++;
	e0->iscutoff = 0;
	e0->m = 0;
	ASSERT (e0->id == 0);
	DPRINT ("+ Event e0 !!\n");
	/* e0 has only one history h0, consisting only on the event e0 */
	h0 = h_alloc (e0);
	ASSERT (h0->id == 0);

	/* insert the initial marking in the marking table */
	h_marking (h0);
	marking_add (h0);
	ASSERT (h0->corr == 0);

	/* set up the initial event in the unfolding */
	u.unf.e0 = e0;

	_unfold_postset (e0);
	_unfold_enriched (h0);
}

static void _unfold_progress (struct h *h)
{
	static int i = 0;
	i++;

	DPRINT ("\n- h%d/e%d:%s; size %d; is ",
			h->id,
			h->e->id,
			h->e->ft->name,
			h->size);
	if (h->corr != 0) {
		DPRINT ("a cut-off! (corr. h%d/e%d:%s)\n",
				h->corr->id,
				h->corr->e->id,
				h->corr->e->ft->name);
	} else {
		DPRINT ("new!\n");
	}

	if ((i & 0xfff) == 0) {
		PRINT ("  At size %6d, %d histories\n", h->size, i);
	}
}

//ALESSANDRO: debug print
void _printAbsorbList(struct nl * conditionList, struct cond * x){
	if(conditionList == 0){
		printf("\n");
		return;
	}
	struct cond * c = conditionList->node;
	if(c != x)
		printf("%s(%d)[%p] ", c->fp->name, c->id, c);
	else
		printf(">>>%s(%d)[%p]<<< ", c->fp->name, c->id, c);
	_printAbsorbList(conditionList->next, x);
}

/*
 * ALESSANDRO: determines if condition c in pre(l) is the image of weak causal place.
 * If the answer is yes, sets c->is_weak_causal to 1 and returns 1
 */
 int is_weak_causal(struct h * history_l, struct cond * c) {
  D3(printf("is_weak_causal(%s(%d), %s)\n", history_l->e->ft->name, history_l->e->id, c->fp->name);)
  int lambda_l, lambda_h;
  int i;
  struct event * h, * l;
  struct cond * d;
  struct place * fc;
  l = history_l->e;
  h = c->pre;
  lambda_l = l->ft->level;
  lambda_h = h->ft->level;
  fc = c->fp;
  if(fc->causal)return 1;//ALESSANDRO: let us optimize
  if(lambda_h != -1 && !FLOW(lambda_h, lambda_l)){ // only the dummy event t0 has level == -1
	  fc->causal = isInPostPositive(c->fp, h->ft);
	  if(fc->causal && !policy->isTransitive){
		  D1(printf("Found a weak causal place %s -> %s -> %s ...\n", h->ft->name, c->fp->name, l->ft->name);)
		  D1(printf(
				  "Because of %s(%d) -> %s(%d)[%p] -> %s(%d) ...\n",
				  h->ft->name, h->id,
				  c->fp->name, c->id, c,
				  l->ft->name, l->id );)

		  // we are looking for an intransitive causal, so we must also check that
		  // c in not absorbed by other conditions in pre(l)
		  for(i = l->pre.deg -1; i >= 0 && fc->causal; --i){
			  d = l->pre.adj[i];
			  D2(printf("checking absorbs list of %s(%d):\n", d->fp->name, d->id);)
			  D2(_printAbsorbList(d->absorbs, c);)
			  fc->causal = !nl_test_ord(d->absorbs, c);
			  if(! fc->causal){
				  D1(printf("...but %s downgrades it\n", ((struct cond *)l->pre.adj[i])->fp->name);)
		  	  }
		  }
	  }
	  D1(if(fc->causal)printf("Found a%s causal place %s -> %s -> %s\n",  policy->isTransitive?" weak":"n intransitive", h->ft->name, c->fp->name, l->ft->name);)
  }
  return fc->causal;
}

int unfold ()
{
	struct h *h;
	/* PAOLO */
	int i;
	int secure; /* is the net BNDC/BINI ? */
	secure = 1;   /* unless something will go wrong, the net is BNDC */

#ifdef CONFIG_MCMILLAN
	DPRINT ("  Using McMillan order\n");
#else
#ifdef CONFIG_ERV
	DPRINT ("  Using Esparza-Romer-Vogler order\n");
#else
	DPRINT ("  Using Mole ERV order\n");
#endif
#endif

#ifdef CONFIG_PMASK
	ec_pmask_init ();
#endif
	nc_create_unfolding ();
	marking_init ();
	pe_init ();
	h_init ();
	_unfold_init ();
	while (!u.historyLimit || u.unf.numh < u.historyLimit) { //ALESSANDRO
		h = pe_pop ();
		/* PAOLO: get return value of marking add: if 1 means
		   that the net is found unsecure: stop! */
		if (h == 0) {
			D2(printf("there are no possible extensions left\n");)
		  break;
		}
		marking_add (h);


		// HERE IT IS TESTED PRESENCE OF WEAK CAUSAL PLACES
		for (i = h->e->pre.deg - 1; (secure || u.complete ) && i >= 0; --i) {
		    secure = !is_weak_causal(h, h->e->pre.adj[i]) && secure; //ALESSANDRO: the order is fundamental
		 }

        /* if a weak causal place has been found and it is not
	   required to generate a complete prefix stop! */
		if (!secure && u.complete == 0) {
		  break;
		}
#ifdef CONFIG_DEBUG
		else {
		  printf("Checked %s, still secure\n", h->e->ft->name);
		}
#endif
		/* PAOLO end */
		_unfold_progress (h);     	//print debug info
		_unfold_postset (h->e);		//here post(e) is created, unless h is not the first history of e

		if (h->corr == 0) {
			h->e->iscutoff = 0;
			_unfold_enriched (h);	//here pe is updated, new histories are added to pe without their post
		}
	}

	h_term ();
	pe_term ();

	//printf("--------------------------------------------------------\n");
	//if (secure)
	//  printf("The net is %s.\n", policy->isTransitive?"BNDC":"BINI");
	//else
	//  printf("The net is not %s.\n", policy->isTransitive?"BNDC":"BINI");

	//printf("--------------------------------------------------------\n\n");
	return secure;
}

