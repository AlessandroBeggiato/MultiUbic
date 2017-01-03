/*
 * msd.c
 *
 *  Created on: Dec 15, 2015
 *      Author: alessandro
 */

#include <stdio.h>
#include <ctype.h>
#include <msd.h>
#include <string.h>
#include <stdlib.h>


void printmsd_Format(){
	fprintf(stderr,
	"  The template for MSD_FORMAT is:\n\n"
	"  MSD\n"
	"  TRANSITIVE/INTRANSITIVE\n"
	"  LVL #levels\n"
	"  0 lvl_name\n"
	"  1 lvl_name\n"
	"  ...\n"
	"  #-1 lvl_name\n"
	"  POLICY\n"
	"  i TO j\n\n");
}

int readln(FILE * f){
	if(line)free(line);
	line = 0;
	n = 0;
	if(getline(&line, &n, f)<0){
		free(line);
		line = 0;
		return 0;
	}
	return 1;
}

void readmsd (char *msdFileName){
	FILE *infile;
	int aux, aux2, i,j;
	int transitive = 0;
	int counter = 0;
	/* Open the file, read the header. */
	if (!(infile = fopen(msdFileName, "r"))){
		fprintf(stderr,"'%s': cannot open for reading\n", msdFileName);
		exit(EXIT_FAILURE);
	}
	line = 0;
	n = 0;
	if (!readln(infile) || strcmp(line, "MSD\n")) {
		fprintf(stderr,"Syntax error in the policy file: msd expected\n");
		exit(EXIT_FAILURE);
	}
	aux = readln(infile);
	if (aux && !strcmp(line, "TRANSITIVE\n"))transitive = 1;
	else if (!aux || strcmp(line, "INTRANSITIVE\n")){
		fprintf(stderr,"Syntax error in the policy file: TRANSITIVE or INTRANSITIVE expected\n");
		exit(EXIT_FAILURE);
	}
	if (!readln(infile) || strncmp(line, "LVL ", 4)) {
		fprintf(stderr,"Syntax error in the policy file: LVL # expected\n");
		exit(EXIT_FAILURE);
	}

	if(strlen(line)<6){
		fprintf(stderr,"Syntax error in the policy file: integer value expected\n");
		exit(EXIT_FAILURE);
	}
	//header read, I can now allocate the policy
	initializemsd(atoi(line+4));
	policy->isTransitive=transitive;
	while(counter < policy->levelNumber){
		if(!readln(infile)){
			fprintf(stderr,"Syntax error in the policy file: %d LevelName expected at line %d\n",counter+1, counter+4);
			exit(EXIT_FAILURE);
		}

		for(aux=0; line[aux] && line[aux]!=' '; ++aux){}
		if(line[aux]!=' ' || !(line[aux+1])){
			fprintf(stderr,"Syntax error in the policy file: %d LevelName expected at line %d\n",counter+1, counter+4);
			exit(EXIT_FAILURE);
		}
		++aux;
		policy->levelNames[counter]=malloc(strlen(line+aux)-1);
		memcpy(policy->levelNames[counter], line+aux, strlen(line+aux)-1);
		policy->levelNames[counter][strlen(line+aux)-1]='\0';
		++counter;
	}

	if (!readln(infile) || strcmp(line, "POLICY\n")) {
		fprintf(stderr,"Syntax error in the policy file: POLICY expected\n");
		exit(EXIT_FAILURE);
	}

	while(readln(infile)){
		if(strstr(line, " TO ") >= line+strlen(line)-4){
			fprintf(stderr,"Syntax error in the policy file: i TO j expected at line %d\n", counter+5);
			exit(EXIT_FAILURE);
		}
		for(aux = 0; line[aux] != ' '; ++aux){}
		line[aux]='\0';
		i = atoi(line);
		for(aux2 = aux+4; line[aux2] != '\n' && line[aux2] != '\0'; ++aux2){}
		line[aux2]='\0';
		j=atoi(line+aux+4);
		FLOW(i,j) = 1;
		++counter;
	}
	fclose(infile);
}

void initializemsd(int levels){
	//printf("Initialize msd with %d levels\n", levels);
	int i,j;
	policy = malloc(sizeof(struct msd));
	policy->isTransitive=0;
	policy->levelNumber=levels;
	policy->levelNames = malloc(levels * sizeof(char*));
	policy->flow = malloc(levels * levels * sizeof(int));
	for(i = 0; i < levels; ++i){
		policy->levelNames[i] = NULL;
		for(j=0; j < levels; ++j)
			if(i==j)FLOW(i,j) = 1;
			else FLOW(i,j) = 0;
	}
}

/*
 * ALESSANDRO: this is computed in the dumbest possible way
 */
void applyTransitiveClosure(){
	int i,j,k;
	for(i = 0; i < policy->levelNumber; ++i){
		for(j = 0; j < policy->levelNumber; ++j)
			for(k = 0; k < policy->levelNumber; ++k)
				if(FLOW(i,j)&&FLOW(j,k))FLOW(i,k)=1;
	}
}

void debugmsd(){
	printf(policy->isTransitive?"Transitive":"Intransitive");
	printf(" policy:\n");
	int i, j;
	for(i=0; i < policy->levelNumber;++i)
		for(j=0; j < policy->levelNumber;++j)
			printf(FLOW(i,j)?"%s->%s\n":"",policy->levelNames[i],policy->levelNames[j]);
}

int getLevelId(char* levelName){
	int i;
	for(i = policy->levelNumber-1; i >=0; --i){
		if(strcmp(policy->levelNames[i], levelName)==0)
			return i;
	}
	return -1;
}
