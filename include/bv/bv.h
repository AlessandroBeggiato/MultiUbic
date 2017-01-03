
/*
 * Bit Array -- interface and implementation :)
 * 
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

#ifndef _BV_BV_H_
#define _BV_BV_H_

#include <string.h>
#include "glue.h"

struct bv {
	int len;
	unsigned char * tab;
};

#define bv_init(b,l) \
	do { \
		(b)->len = 1 + (l) / 8; \
		(b)->tab = (unsigned char *) gl_realloc (0, (b)->len); \
		memset ((b)->tab, 0, (b)->len); \
	} while (0)

#define bv_term(b) \
		gl_free ((b)->tab)

#define bv_get(b,i) \
		(((b)->tab[(i) >> 3] & (1 << ((i) & 7))) ? 1 : 0)
#define bv_clear(b,i) \
		(b)->tab[(i) >> 3] &= ~(1 << ((i) & 7))
#define bv_set(b,i) \
		(b)->tab[(i) >> 3] |= 1 << ((i) & 7)
#define bv_setv(b,i,v) \
		(b)->tab[(i) >> 3] = \
			((b)->tab[(i) >> 3] & ~(1 << ((i) & 7))) | \
			(((v) & 1) << ((i) & 7))
// FRANCESCO
#define bv_or(b1 , b2 , b3) \
  if((b1)->len == (b2)-> len && (b3)->len == (b1)->len) { \
    int i, len; \
    len = (b1)->len; \
    for(i = 0; i < len; ++i){\
      (b3)->tab[i]=(b1)->tab[i] | (b2)->tab[i]; \
    }\
  }
#define bv_zeros(b) \
    do{\
    int i;\
    for(i = 0; i < (b)->len; i++){\
      (b)->tab[i] = 0;\
    }\
    }while(0)
// \FRANCESCO
#endif
