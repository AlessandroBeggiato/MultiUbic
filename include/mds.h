/*
 * mds.h
 *
 *  Created on: Dec 15, 2015
 *      Author: alessandro
 */


#ifndef INCLUDE_MDS_H_
#define INCLUDE_MDS_H_

/* ALESSANDRO
 * Struct to store the security policy and a macro to poll it.
 *
 * */
struct mds {
	int isTransitive; //1 mean yes, and we must transitively close flow
	int levelNumber; //how many level are there
	char** levelNames; //human readable names for security levels
	int* flow; //The adjacency matrix is stored in a single array
};

struct mds* policy;

#define FLOW(X,Y) (policy->flow)[(policy->levelNumber * X) + Y]

void readMDS (char *mdsFileName);
void applyTransitiveClosure();
void initializeMDS(int levels);
void debugMDS();
void printMDS_Format();

#endif /* INCLUDE_MDS_H_ */
