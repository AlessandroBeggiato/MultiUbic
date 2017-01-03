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

#ifndef _NETCONV_H_
#define _NETCONV_H_

#include "global.h"
#include "al/al.h"

void nc_create_net (void);
void nc_create_unfolding (void);
struct place * nc_create_place (void);
struct trans * nc_create_transition (void);
int nc_create_arc (struct al * src_post, struct al * dst_pre,
		void * src, void * dst);
void nc_static_checks (const char * stoptr);
int causal_reduction(void); /* PAOLO: causal reduction. ALESSANDRO: updated to MDS and added fast static checks */
#endif
