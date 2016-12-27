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

#ifndef INCLUDE_msd_H_
#define INCLUDE_msd_H_

/* ALESSANDRO
 * Struct to store the security policy and a macro to poll it.
 *
 * */
struct msd {
	int isTransitive; //1 mean yes, and we must transitively close flow
	int levelNumber; //how many level are there
	char** levelNames; //human readable names for security levels
	int* flow; //The adjacency matrix is stored in a single array
};

struct msd* policy;

#define FLOW(X,Y) (policy->flow)[(policy->levelNumber * X) + Y]

void readmsd (char *msdFileName);
void applyTransitiveClosure();
void initializemsd(int levels);
void debugmsd();
void printmsd_Format();

#endif /* INCLUDE_msd_H_ */
