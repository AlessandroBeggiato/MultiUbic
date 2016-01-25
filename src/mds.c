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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "readlib.h"
#include "mds.h"
#include "glue.h"

void printMDS_Format(){
	fprintf(stderr,
	"  The template for MDS_FORMAT is:\n\n"
	"  MDS\n"
	"  TRANSITIVE/INTRANSITIVE\n"
	"  LVL #levels\n"
	"  0 lvl_name\n"
	"  1 lvl_name\n"
	"  ...\n"
	"  #-1 lvl_name\n"
	"  POLICY\n"
	"  i TO j\n\n");
}

/*
 * ALESSANDRO: copy and paste of ReadCmdToken that sets sbuf to NULL instead of aborting
 * is something goes wrong. I know this is a terrible practice but I need this done the day
 * before yesterday =)
 */
void ReadToken (FILE *file){
	register int  len = 0;
	char *string = sbuf;

	if (!string) sbuf = string = gl_malloc (sballoc = 512);

	if (!isalnum((int)(*string++ = ReadCharComment(file)))){
		sbuf=NULL;
		return;
	}

	len++;
	while (isalnum((int)(*string = getc(file))) || *string == '_')
	{
		string++;
		len++;
		if (len >= sballoc)
		{
			sbuf = gl_realloc (sbuf,sballoc += 512);
			string = sbuf + len;
		}
	}
	ungetc(*string, file);
	*string = '\0';
}

void readMDS (char *mdsFileName){
	FILE *infile;
	int i,j;
	int transitive = 0;
	int counter = 0;
	/* Open the file, read the header. */
	if (!(infile = fopen(mdsFileName, "r"))){
		fprintf(stderr,"'%s': cannot open for reading\n", mdsFileName);
		exit(EXIT_FAILURE);
	}

	ReadToken(infile);
	if (strcmp(sbuf, "MDS")) {
		fprintf(stderr,"Syntax error in the policy file: MDS expected\n");
		exit(EXIT_FAILURE);
	}
	ReadNewline(infile);
	ReadToken(infile);
	if (!strcmp(sbuf, "TRANSITIVE"))transitive = 1;
	else if (strcmp(sbuf, "INTRANSITIVE")){
		fprintf(stderr,"Syntax error in the policy file: TRANSITIVE or INTRANSITIVE expected\n");
		exit(EXIT_FAILURE);
	}
	ReadNewline(infile);
	ReadToken(infile);
	if (strcmp(sbuf, "LVL")) {
		fprintf(stderr,"Syntax error in the policy file: LVL expected\n");
		exit(EXIT_FAILURE);
	}
	ReadToken(infile);
	if(!sbuf){
		fprintf(stderr,"Syntax error in the policy file: integer value expected\n");
		exit(EXIT_FAILURE);
	}
	//header read, I can now allocate the policy
	initializeMDS(atoi(sbuf));
	policy->isTransitive=transitive;
	while(counter < policy->levelNumber){
		ReadNewline(infile);
		ReadToken(infile);
		if(!sbuf || atoi(sbuf)!=counter){
			fprintf(stderr,"Syntax error in the policy file: %d expected at line %d\n",counter+1, counter+4);
			exit(EXIT_FAILURE);
		}
		ReadToken(infile);
		if(!sbuf){
			fprintf(stderr,"Syntax error in the policy file: levelName expected at line %d \n",counter+4);
			exit(EXIT_FAILURE);
		}
		policy->levelNames[counter]=malloc(strlen(sbuf));
		sprintf(policy->levelNames[counter],"%s",sbuf);
		++counter;
	}
	ReadNewline(infile);
	ReadToken(infile);
	if (strcmp(sbuf, "POLICY")) {
		fprintf(stderr,"Syntax error in the policy file: POLICY expected\n");
		exit(EXIT_FAILURE);
	}
	ReadNewline(infile);
	ReadToken(infile);
	while(sbuf){
		if(!sbuf){
			fprintf(stderr,"Syntax error in the policy file: integer value expected at line %d\n", counter+5);
			exit(EXIT_FAILURE);
		}
		i = atoi(sbuf);
		ReadToken(infile);
		if (!sbuf || strcmp(sbuf, "TO")) {
			fprintf(stderr,"Syntax error in the policy file: TO expected at line %d\n", counter+5);
			exit(EXIT_FAILURE);
		}
		ReadToken(infile);
		if(!sbuf){
			fprintf(stderr,"Syntax error in the policy file: integer value expected at line %d\n", counter+5);
			exit(EXIT_FAILURE);
		}
		j=atoi(sbuf);
		FLOW(i,j) = 1;
		++counter;
		ReadNewline(infile);
		ReadToken(infile);
	}
}

void initializeMDS(int levels){
	//printf("Initialize MDS with %d levels\n", levels);
	int i,j;
	policy = malloc(sizeof(struct mds));
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

void debugMDS(){
	printf(policy->isTransitive?"Transitive":"Intransitive");
	printf(" policy:\n");
	int i, j;
	for(i=0; i < policy->levelNumber;++i)
		for(j=0; j < policy->levelNumber;++j)
			printf(FLOW(i,j)?"%s->%s\n":"",policy->levelNames[i],policy->levelNames[j]);
}
