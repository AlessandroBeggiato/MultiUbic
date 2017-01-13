/*
 * Copyright (C) 2014 Francesco Burato
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

/*
 * Data structure for storing the extended markings for
 * complete prefixes. It is a single linked list that keep
 * tracks of the places in the frontier of the configuration
 * which are caused by high transition in the frontier and
 * of the places in the frontier of the configuration which are
 * caused by high transition in the frontier and are downgraded.
 *
 * The projection of the high transition is memorized as index
 * of the hash table while the markings are saved as pointer
 * to nodelist which will be NULL if the markings are empty.
 */
#ifndef EXM_H_
#define EXM_H_
#include "nodelist.h"
#include "global.h"

struct exm {
  int high_index; // index of high transition in hash table
  struct place * place; // pointer to the place caused by high condition
  int downgraded; // != 0 iff there is a downgrading transition after high condition
};
int exm_compartor(void * node1, void * node2);

#endif /* EXM_H_ */
