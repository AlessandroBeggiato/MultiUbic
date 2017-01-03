/*
 * msd.h
 *
 *  Created on: Dec 15, 2015
 *      Author: alessandro
 */


#ifndef INCLUDE_MSD_H_
#define INCLUDE_MSD_H_

#include <stdio.h>

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
char* line;
size_t n;

#define FLOW(X,Y) (policy->flow)[(policy->levelNumber * X) + Y]

void readmsd (char *msdFileName);
void applyTransitiveClosure();
void initializemsd(int levels);
int getLevelId(char* levelName);
void debugmsd();
void printmsd_Format();
int readln(FILE * f);
#endif /* INCLUDE_MSD_H_ */
