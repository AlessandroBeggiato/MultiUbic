
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

/* test and debug */
#undef CONFIG_DEBUG

/* unfold order */
#undef CONFIG_MCMILLAN
#define CONFIG_ERV
#undef CONFIG_ERV_MOLE
#undef CONFIG_SIZELEX

/* see src/nodelist.c */
#define CONFIG_NODELIST_STEP 1024

/* experimental */
#undef CONFIG_PMASK

//ALESSANDRO: more fine tuned debug macros to trace the executions
// higher DLVL defined => more verbose output

//#define DLVL1
#define DLVL2
//#define DLVL3
//#define DLVL4

#ifdef DLVL4
	#define D4(x) x
	#define DLVL3
#else
	#define D4(x)
#endif

#ifdef DLVL3
	#define D3(x) x
	#define DLVL2
#else
	#define D3(x)
#endif

#ifdef DLVL2
	#define D2(x) x
	#define DLVL1
#else
	#define D2(x)
#endif

#ifdef DLVL1
	#define D1(x) x
#else
	#define D1(x)
#endif

