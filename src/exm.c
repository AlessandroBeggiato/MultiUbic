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

#include "exm.h"
int exm_compartor(void * node1, void * node2) {
  // downcast of the pointers
  struct exm * mark1 = node1;
  struct exm * mark2 = node2;
  return mark1->high_index == mark2->high_index &&
      ( (mark1->downgraded && mark2->downgraded) ||
          (! mark1->downgraded && mark2->downgraded) ) &&
       mark1->place == mark2->place;
}
