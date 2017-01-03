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

#ifndef _NODELIST_H_
#define _NODELIST_H_

struct nl {
	void * node;
	struct nl * next;
};

struct nl * nl_push (struct nl **, void *);
struct nl * nl_insert (struct nl **, void*);
struct nl* nl_insert2 (struct nl **list, void *node,
		int (* cmp) (const void *n1, const void *n2));
void nl_delete (struct nl *);
int nl_compare (const struct nl *list1, const struct nl *list2);
//ALESSANDRO: compares two lists using the custom comparator cmp on their elements
int nl_compare2 (const struct nl *list1, const struct nl *list2, int (* cmp) (const void *n1, const void * n2));
int nl_test (const struct nl *list, void *node); /*PAOLO*/
int nl_test_ord (const struct nl *list, void *node); /*ALESSANDRO*/

// FRANCESCO
/*
 * Find an element in a list using the comparator cmp passed as argument.
 */
char nl_find_custom(struct nl * list, void * element,
    int (* cmp) (void *n1, void * n2));
void nl_freememory(struct nl * list);
// \FRANCESCO
#endif
