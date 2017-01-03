/*
 * Copyright (C) 2015 Alessandro Beggiato <alessandro.beggiato@imtlucca.it>
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

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "msd.h"

/*****************************************************************************/

/*
 * Maps is an nxn matrix, where n is the number of levels in the policy.
 * Each column j tells how level i (on the rows) must be mapped (h,l,d)
 * when checking for j-BNDC or j-BINI
 */
char * maps;
FILE * ifs;
FILE * ofsPolicy;
FILE ** ofs;

void clean(int n){
	int i;
	fclose(ifs);
	for(i = 0; i < n; ++i)fclose(ofs[i]);
	if(ofsPolicy)fclose(ofsPolicy);
}

void usage (void)
{
	fprintf(stderr,
"\n"
"The Partitioner tool -- a program to convert an MSD net system in a set of two (or three) level net systems\n"
"Copyright (C) 2015 Alessandro Beggiato\n"
"\n"
"This program comes with ABSOLUTELY NO WARRANTY.  This is free software, and you\n"
"are welcome to redistribute it under certain conditions.  You should have\n"
"received a copy of the GNU General Public License along with this program.  If\n"
"not, see <http://www.gnu.org/licenses/>.\n"
"\n"
"\n"
"Usage: partition [OPTIONS] NETFILE POLICYFILE\n"
"\n"
"Argument NETFILE is a path to the .ll_net input file.\n"
"Argument POLICYFILE is a path to the .msd policy file.\n"
"Available options: \n"
" -m \t produces output for MultiUBIC (default is for Ubic2)\n"
);
printmsd_Format();
fprintf(stderr,
"Branch eager v0.1, compiled %s\n", __DATE__);

	exit (EXIT_FAILURE);
}

void computeMaps(){
	int i, j, L;
	L = policy->levelNumber;
	maps = 0;
	maps = malloc(sizeof(char) * L * L);
	for(i = 0; i < L; ++i)
		for(j = 0; j < L; ++j){
			if(!FLOW(j,i)){
				maps[(i * L) + j] = 'l';
			}
			else{
				if(i==j){
					maps[(i * L) + j] = 'h';
				}
				else{
					if(policy->isTransitive)
						maps[(i * L) + j] = 'h';
					else
						maps[(i * L) + j] = 'd';
				}
			}
		}
}

void debugMaps(){
	int i, j;
	printf("    L=\t");
	for(i = 0; i < policy->levelNumber; ++i)
		printf("%s \t", policy->levelNames[i]);
	printf("\n");
	for(i = 0; i < policy->levelNumber; ++i){
		printf("m(%s)\t", policy->levelNames[i]);
		for(j = 0; j < policy->levelNumber; ++j){
			printf("%c \t", maps[(i * policy->levelNumber) + j]);
		}
		printf("\n");
	}
}

void writePolicy(FILE * f, int isTransitive){
	fprintf(f, "MSD\n");
	if(isTransitive)
		fprintf(f, "TRANSITIVE\n");
	else
		fprintf(f, "INTRANSITIVE\n");
	fprintf(f, "LVL 3\n");
	fprintf(f,"0 l\n");
	fprintf(f,"1 h\n");
	fprintf(f,"2 d\n");
	fprintf(f, "POLICY\n");
	fprintf(f, "0 TO 1\n");
	if(!isTransitive){
		fprintf(f, "1 TO 2\n");
		fprintf(f, "2 TO 0\n");
	}
}

int main (int argc, char **argv)
{
	char *inpath;
	char *netname;
	char *outdir;
	char *ofname;
	char *policyPath;
	char outPolicyPath[300];
	char *auxstr, *auxstr2;
	int i, L, verbatim, levelId;
	int multi = 0;

	inpath = 0;


	int opt;
	while (1) {
		opt = getopt (argc, argv, "m");
		if (opt == -1) break;
		switch (opt) {
		case 'm' :
	  	    multi = 1;
	  	   	break;
		default :
			usage ();
		}
	}

	if(optind != argc-2)usage();
	inpath = argv[argc - 2];
	policyPath = argv[argc - 1];
	ofsPolicy = 0;
	readmsd(policyPath);
	if(policy->isTransitive)applyTransitiveClosure();
	//debugmsd();//comment this
	computeMaps();
	//debugMaps();//comment this
	ofs = malloc(sizeof(FILE *) * policy->levelNumber);

	if (!(ifs = fopen(inpath, "r"))){
		fprintf(stderr,"'%s': cannot open for reading\n", inpath);
		exit(EXIT_FAILURE);
	}

	//compute out folder
	auxstr = rindex(inpath, '.');
	*auxstr = '\0';
	outdir = malloc(sizeof(char) * strlen(inpath));
	strcpy(outdir, inpath);
	*auxstr = '.';

	//extract net name
	auxstr2 = strrchr(inpath, '/');
	if(!auxstr2)auxstr2=inpath;
	else auxstr2 = auxstr2+1;
	netname = malloc(sizeof(char) * (strlen(auxstr2) - strlen(auxstr) + 1));
	strncpy(netname, auxstr2, strlen(auxstr2) - strlen(auxstr));
	netname[strlen(auxstr2) - strlen(auxstr)] = '\0';

	if(mkdir(outdir, 0777) != 0 && errno!=EEXIST){
		fprintf(stderr,"'%s': cannot create\n", inpath);
		fclose(ifs);
		exit(EXIT_FAILURE);
	}

	//printf("Partitioner ready\n");
	//printf("Processing net system: \t %s \n", netname);
	//printf("From file            : \t %s \n", inpath);
	//printf("With security policy : \t %s \n", policyPath);
	//printf("Placing outputs in   : \t %s/ \n", outdir);
	if(multi){
		sprintf(outPolicyPath, "%s/%s.msd",outdir, netname);
		//printf("Output policy file   : \t %s \n", outPolicyPath);
		ofsPolicy = fopen(outPolicyPath,"w");
		if(!ofsPolicy){
			fprintf(stderr,"'%s': cannot open for write\n", outPolicyPath);
			exit(EXIT_FAILURE);
		}
		writePolicy(ofsPolicy, policy->isTransitive);
	}

	L = policy->levelNumber;
	for(i = 0; i < L; ++i){
		ofname = malloc(sizeof(char) * strlen(outdir) + strlen(netname) + strlen(policy->levelNames[i]) + 10);
		memset(ofname, 0, sizeof(char) * strlen(outdir) + strlen(netname) + strlen(policy->levelNames[i]) + 10);
		sprintf(ofname, "%s/%s_%s.ll_net", outdir, netname, policy->levelNames[i]);
		//printf("creating output file : \t %s\n", ofname);
		ofs[i] = fopen(ofname, "w");
		if(!ofs[i]){
			fprintf(stderr,"'%s': cannot open for write\n", ofname);
			clean(i);
		}
		free(ofname);
		ofname = 0;
	}

	//copy everything up to TR included
	//after TR modify each transition name according to maps
	//if you read TP or PT start copying everything again
	verbatim = 1;
	while(readln(ifs)){
		//printf("read: %s", line);//comment this
		if(!strcmp(line, "TP\n") || !strcmp(line, "PT\n")){
			//printf("----- \t Verbatim ON \t -----\n");//comment this
			verbatim = 1;
		}

		if(!verbatim){
			//then I am dealing with a transition line and I need to recognize and store its level
			//transitions are in this format: id"name_level"9@9
			//if will set pointers to the '_' character and to the last '\"' character
			auxstr = rindex(line, '_');
			if(!auxstr){
				fprintf(stderr,"'%s': transition_level expected\n", line);
				clean(L);
				exit(EXIT_FAILURE);
			}
			//I terporarily close the string after the level, so I can easily read it using the pointer to '_'
			auxstr2=rindex(line, '\"');
			*auxstr2='\0';
			levelId = getLevelId(auxstr+1);
			*auxstr2='\"';
			if(levelId < 0){
				fprintf(stderr,"'%s': unknow level\n", line);
				clean(L);
				exit(EXIT_FAILURE);
			}
		}

		if(verbatim){
			//here I copy a line which does not need any rewriting
			for(i = 0; i < L; ++i)
				fprintf(ofs[i], "%s", line);
		}
		else{
			//here I rewrite a line containing a transition, changing its level according to the maps
			//the format in the original net is id"name_level"9@9
			//for Ubic I write id"L-name_level"9@9
			//for MultiUBIC i write id"name-level_L"9@9
			//with L belonging to {l,d,h} in both cases
			if(multi)//store the insertion point
				auxstr = index(line, '_')-1;
			else
				auxstr = index(line, '\"');
			auxstr2 = line;
			while(*auxstr2!='\0'){
				for(i = 0; i < L; ++i)
					fprintf(ofs[i], "%c", *auxstr2);
				if(auxstr2 == auxstr){//then this is where I insert the level
					if(multi){
						for(i = 0; i < L; ++i){
							fprintf(ofs[i], "-");
						}
						//printf("%c",*auxstr2);//comment this
						++auxstr2;
						//printf("%c",*auxstr2);//comment this
						++auxstr2;//skip the '_'
						//printf("%c\n",*auxstr2);//comment this
						auxstr = rindex(line, '\"');
						*auxstr = '\0';
						for(i = 0; i < L; ++i){
							//printf("%s_%c\"9@9\n",auxstr2,maps[(levelId*L) + i]);//comment this
							fprintf(ofs[i], "%s_%c\"9@9\n",auxstr2,maps[(levelId*L) + i]);
						}
						auxstr2=auxstr-1;//so we will exit the while after the increment
					}
					else{//Ubic
						for(i = 0; i < L; ++i){
							fprintf(ofs[i], "%c-", maps[(levelId*L) + i]);
						}
					}
				}
				++auxstr2;
			}
		}

		if(!strcmp(line, "TR\n")){
			verbatim = 0;
			//printf("----- \t Verbatim OFF \t -----\n");//comment this
		}
	}

	clean(L);
	//printf("\nDone. \n");//comment this
	return EXIT_SUCCESS;
}
