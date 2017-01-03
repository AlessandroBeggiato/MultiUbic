/*
 * main.cpp
 *
 *  Created on: Jan 11, 2016
 *      Author: alessandro
 */

#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

using namespace std;

void usage (void)
{
	fprintf(stderr,
"\n"
"TestNetGenerator -- an generator of net systems\n"
"Copyright (C) 2015 Alessandro Beggiato\n"
"\n"
"This program comes with ABSOLUTELY NO WARRANTY.  This is free software, and you\n"
"are welcome to redistribute it under certain conditions.  You should have\n"
"received a copy of the GNU General Public License along with this program.  If\n"
"not, see <http://www.gnu.org/licenses/>.\n"
"\n"
"\n"
"Usage: TestNetGen [OPTIONS] OUT_DIR\n"
"\n"
"Generates a test net and its security policy.\n"
"Saves them in OUT_DIR\n");
fprintf(stderr,
" -n N  Number of blocks in the net. Each block consists of an input place and an output place and three conflicting transitions.\n"
"       (default is 1)\n"
" -l L  Number of different security levels in a block, range is 1-3.\n"
"       (default is 1)\n"
" -e E  Number of optional arcs in the policy, among security levels of transitions in each block, range is 0-6.\n"
"       (default is 0)\n"
"\n"
"Branch eager v0.1, compiled %s\n", __DATE__);
	exit (EXIT_FAILURE);
}

char levels[] = {'A','A','A'};
char buffer[200];
int N = 1;
int L = 1;
int E = 0;

void writeMSD(FILE * f){
	fprintf(f, "MSD\n");
	fprintf(f, "TRANSITIVE\n");
	fprintf(f, "LVL %d\n", N*L);
	int i, j;
	for(j=0; j<L;  ++j){
		for(i=0; i<N; ++i){
			fprintf(f,"%d %c%d\n", (j*N)+i, levels[j], i);
		}
	}
	fprintf(f, "POLICY\n");
	for(i=0; i<N-1; ++i){
		fprintf(f, "%d TO %d\n", i, i+1);
		if(L>1){
			fprintf(f, "%d TO %d\n", N+i, N+i+1);
			if(E>0)fprintf(f, "%d TO %d\n", i, N+i);
			if(E>1)fprintf(f, "%d TO %d\n", N+i, i);
		}
		if(L>2){
			fprintf(f, "%d TO %d\n", (2*N)+i, (2*N)+i+1);
			if(E>3)fprintf(f, "%d TO %d\n", i, (2*N)+i);
			if(E>4)fprintf(f, "%d TO %d\n", (2*N)+i, i);
			if(E>5)fprintf(f, "%d TO %d\n", (2*N)+i, N+i);
			if(E>6)fprintf(f, "%d TO %d\n", N+i, (2*N)+i);
		}
	}
	//policy in the last block
	if(L>1){
		if(E>0)fprintf(f, "%d TO %d\n", N-1, (2*N)-1);
		if(E>1)fprintf(f, "%d TO %d\n", (2*N)-1, N-1);
	}
	if(L>2){
		if(E>3)fprintf(f, "%d TO %d\n", N-1, (2*N)-1);
		if(E>4)fprintf(f, "%d TO %d\n", (2*N)-1, N-1);
		if(E>5)fprintf(f, "%d TO %d\n", (3*N)-1, (2*N)-1);
		if(E>6)fprintf(f, "%d TO %d\n", (2*N)-1, (3*N)-1);
	}
}

void writeNet(FILE* f){
	int i, j, lastid;

	vector<string> tp, pt;

	char transitions[] = {'a','b','c'};

	fprintf(f,"PEP\n");
	fprintf(f,"PetriBox\n");
	fprintf(f,"FORMAT_N2\n");

	fprintf(f,"PL\n");
	fprintf(f,"1 \"p0\"9@9M1\n");
	for(i=1; i <= N; ++i){
		fprintf(f,"%d \"p%d\"9@9\n",i+1,i);
	}
	lastid = N+1;
	fprintf(f,"TR\n");
	for(i=0; i<N; ++i){
		for(j=0; j<3;++j){
			fprintf(f,"%d \"%c%d_%c%d\"9@9\n",++lastid,transitions[j], i, levels[j],i);
			sprintf(buffer, "%d > %d", i+1 , lastid);
			pt.push_back(string(buffer));
			sprintf(buffer, "%d < %d", lastid, i+2);
			tp.push_back(string(buffer));
		}
	}

	fprintf(f,"TP\n");//<
	while(tp.size()>0){
		fprintf(f,"%s\n",tp.back().c_str());
		tp.pop_back();
	}
	fprintf(f,"PT\n");
	while(pt.size()>0){
		fprintf(f,"%s\n",pt.back().c_str());
		pt.pop_back();
	}
}

int main(int argc, char* const argv[]){
	FILE* netFile;
	FILE* policyFile;
	const char * outDir;
	string netPath;
	string policyPath;
	int opt;
	while (1) {
		opt = getopt (argc, argv, "n:e:l:");
		if (opt == -1) break;
		switch (opt) {
		case 'n' :
  		    N = atoi(optarg);
			break;
		case 'e' :
			E = atoi(optarg);
			if(E<0 || E>6){
				fprintf(stderr, "ERROR: -e option out of range");
				usage();
			}
			break;
		case 'l' :
			L = atoi(optarg);
			if(L<1 || L>3){
				fprintf(stderr, "ERROR: -l option out of range");
				usage();
			}
			break;
		default :
			usage ();
		}
	}
	if(optind!= argc-1)usage();
	outDir = argv[optind];
	printf("Components: %d Levels: %d Extra edges: %d\n",N,L,E);
	sprintf(buffer , "%s/TestNet-%.3d-%d-%d",outDir,N,L,E);
	netPath = string(buffer) + ".ll_net";
	policyPath = string(buffer) + ".msd";
	netFile = fopen(netPath.c_str(), "w");
	if(!netFile){
		printf("%s: cannot open for write\n", netPath.c_str());
		exit(EXIT_FAILURE);
	}
	policyFile = fopen(policyPath.c_str(), "w");
	if(!policyFile){
		printf("%s: cannot open for write\n", policyPath.c_str());
		fclose(netFile);
		exit(EXIT_FAILURE);
	}
	if(L>1)levels[1] = 'B';
	if(L>2)levels[2] = 'C';
	writeMSD(policyFile);
	fclose(policyFile);
	writeNet(netFile);
	fclose(netFile);
	exit(EXIT_SUCCESS);
}
