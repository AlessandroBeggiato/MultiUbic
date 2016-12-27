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

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h> //ALESSANDRO

#include "glue.h"
#include "debug.h"
#include "global.h"
#include "output.h"
#include "netconv.h"
#include "readpep.h"
#include "unfold.h"

/* awk */
#define TOOL_AWK "\"mawk\""

/* grep */
#define TOOL_GREP "\"/bin/grep\""

/* Name of package */
#define PACKAGE "anica"

/* all gloabal data is stored in this structure */
struct u u;

/*****************************************************************************/

void usage (void)
{
	fprintf(stderr,
"\n"
"The MultiUbic tool -- an unfolding based intransitive interference checker for multilevel domains\n"
"Copyright (C) 2015 Alessandro Beggiato\n"
"based on the Ubic2 project\n"
"Copyright (C) 2014 Francesco Burato\n"
"based on the Ubic project\n"
"Copyright (C) 2013 Paolo Baldan, Alberto Carraro\n"
"based on the Cunf project\n"
"Copyright (C) 2010-2012  Cesar Rodriguez <cesar.rodriguez@lsv.ens-cachan.fr>\n"
"Laboratoire de Specification et Verification (LSV), ENS Cachan, France\n"
"\n"
"This program comes with ABSOLUTELY NO WARRANTY.  This is free software, and you\n"
"are welcome to redistribute it under certain conditions.  You should have\n"
"received a copy of the GNU General Public License along with this program.  If\n"
"not, see <http://www.gnu.org/licenses/>.\n"
"\n"
"\n"
"Usage: MultiUbic [OPTIONS] NETFILE POLICYFILE\n"
"\n"
"Argument NETFILE is a path to the .ll_net input file.\n"
"Argument POLICYFILE is a path to the .msd policy file.");
printmsd_Format();
fprintf(stderr,
"Allowed OPTIONS are:\n"
" -s [on|off]  Stop once the first weak causal place is found and report it.\n"
"              (off by default, hence a prefix complete for interferences is\n"
"               generated)\n"
" -l NUMBER    Stop when NUMBER histories have been added\n"
" -t NAME      Stop when transition NAME is inserted\n"
" -d DEPTH     Unfold up to given DEPTH\n"
" -o FILE      Output file to store the unfolding in.  If not provided,\n"
"              defaults to NETFILE with the last 7 characters removed\n"
"              (extension '.ll_net') plus a suffix depending option the -O\n"
" -f FORMAT    Write unfolding in format FORMAT. Available formats: 'cuf',\n"
"              'dot', 'fancy'.  Default is 'cuf'. In 'fancy' format weak\n"
"              causal place representatives are depicted in red\n"
"\n"
"For more information, see http://www.math.unipd.it/~baldan/UBIC\n"
"Branch eager v.0.3, compiled %s\n", __DATE__);

	exit (EXIT_FAILURE);
}

void rusage (void)
{
	struct rusage r;
	char buff[128];
	int fd, ret;

	u.unf.usrtime = 0;
	u.unf.vmsize = 0;
	ret = getrusage (RUSAGE_SELF, &r);
	if (ret >= 0) {/*if no error occurs*/
		/* in linux this is 0; in mac os this is the maxrss in kb */
		u.unf.vmsize = r.ru_maxrss; /*in kilobyte*/
		u.unf.usrtime = r.ru_utime.tv_sec * 1000 +
				r.ru_utime.tv_usec / 1000;
	}

	/* this will only work in linux, in macos u.unf.vmsize is set to maxrss
	 * in kb */
	fd = open ("/proc/self/statm", O_RDONLY);
	if (fd < 0) return;
	ret = read (fd, buff, 128);
	close (fd);
	buff[127] = 0;
        u.unf.vmsize = strtoul (buff, 0, 10) * sysconf (_SC_PAGESIZE) >> 10;
        //int mem_usage = system("ps -o rss -o comm | grep -r ubic");
}

char * peakmem (void)
{
	static char b[16];
	char buff[4096];
	char *s;
	int fd, ret;

	fd = open ("/proc/self/status", O_RDONLY);
	if (fd < 0) return "?";

	ret = read (fd, buff, 4096);
	close (fd);
	if (ret >= 4096) {
		PRINT ("Bug in peakmem!!\n");
		exit (1);
	}
	buff[ret] = 0;
	s = strstr (buff, "VmPeak:\t");
	if (! s) return "?";
	s[16] = 0;
	sprintf (b, "%u", atoi (s + 8));
	return b;
}

/*
 * ALESSANDRO: goes through the transitions of the net,
 * reads the level from their name and assigns it to the additional
 * field in the transition structure. Since we are at it, we also
 * syntax check the transitions, aborting if some of them lacks information
 * about its level.
 * The dummy transition t0 is assigned a special level -1
 */
void addLevelToTransitions() {
  struct trans * t;
  struct ls * n;
  char* strPtr;
  int i = 0;
  n = u.net.trans.next;//playing dirty to skip the dummy _t0_
  u.net.t0->level = -1;
  for (n = n->next; n; n = n->next) {
    // get a transition, find '_' in its name
    t = ls_i (struct trans, n, nod);
    strPtr = t->name;
    while(strPtr && *strPtr != '_' && *strPtr != '\0')++strPtr;
    if(!strPtr || *strPtr == '\0'){
    	fprintf(stderr, "Syntax error in transition %s: %s_level expected\n", t->name, t->name);
    	exit(EXIT_FAILURE);
    }
    ++strPtr;//Now I'm pointing at the first character of the level (could as well be \0)
    //Naive lookup in the level table.
    for(i = 0; i < policy->levelNumber; ++i)
    	if(!strcmp(strPtr, policy->levelNames[i])){
    		t->level=i;
    		i = policy->levelNumber;
    	}
    if(i != policy->levelNumber + 1){
    	fprintf(stderr, "Error: transition %s has unrecognized level %s\n", t->name, strPtr);
    	exit(EXIT_FAILURE);
    }
  }
}


int main (int argc, char **argv)
{
	int opt, l, secure;
	char *stoptr, *inpath, *outpath, *outformat;

	/* initialize global parameters */
	u.mark = 2;

	/* PAOLO: additional global parameter: generate full unfolding? Default is yes. */
	u.complete=1;

	char *policyPath;	/* ALESSANDRO: additional parameter: file containing the policy specification */
	int possiblyNotSecure = 0;		/* ALESSANDRO: auxiliary flag to implement fast static check (avoiding to unfold surely secure nets*/

	/* parse command line */
	stoptr = 0;
	inpath = 0;
	outpath = 0;
	outformat = 0;
	u.stoptr = 0;
	u.depth = 0;
	u.historyLimit = 0;//ALESSANDRO
	while (1) {
		opt = getopt (argc, argv, "s:o:t:d:f:l:");
		if (opt == -1) break;
		switch (opt) {
		//ALESSANDRO
		case 'l':
			u.historyLimit = atoi (optarg);
			break;
		/* PAOLO: option s (stop at first causal place) added */
		case 's' :
  		       if (strcmp (optarg, "on") == 0)
    		          u.complete=0;
		        break;		
		case 'o' :
			outpath = optarg;
			break;
		case 't' :
			stoptr = optarg;
			break;
		case 'd' :
			u.depth = atoi (optarg);
			break;
		case 'f' :
			outformat = optarg;
			break;
		default :
			usage ();
		}
	}
	/* ALESSANDRO: input file and policy file expected at this point*/
	/* ALESSANDRO: old code
	 * if (optind != argc - 1) usage ();
	 * policyPath = argv[argc - 1];
	 */
	if (optind != argc - 2) usage ();
	inpath = argv[argc - 2];
	policyPath = argv[argc - 1];
	printf("MultiUbic %s %s\n", inpath, policyPath);
	/* ALESSANDRO: end of new code*/

	/* validate the output format string */
	if (! outformat) outformat = "cuf";
	if (strcmp (outformat, "cuf") && strcmp (outformat, "dot") &&
			strcmp (outformat, "fancy")) usage ();

	/* set default file name for the output */
	if (! outpath) {
		l = strlen (inpath);
		outpath = gl_malloc (l + 16);
		strcpy (outpath, inpath);
		strcpy (outpath + (l > 7 ? l - 7 : l), ".unf.");
		strcpy (outpath + (l > 7 ? l - 2 : l + 5), 
				! strcmp (outformat, "fancy") ? "dot" :
				outformat);
	}

	/* ALESSANDRO: read the policy file, compute transitive closure it if it is transitive */
	readmsd(policyPath);
	if(policy->isTransitive)applyTransitiveClosure();
	D1(debugmsd();)
	/* ALESSANDRO: end of new code */

	/* load the input net */
	DPRINT ("  Reading net from '%s'\n", inpath);
	D1 (printf("  Reading net from '%s'\n", inpath);)
	read_pep_net (inpath);
	DPRINT ("  It is a %s net\n", u.net.isplain ? "plain" : "contextual");
	nc_static_checks (stoptr);

	/*
	 * ALESSANDRO: instead of tampering with read_pep_net, I go through
	 * the transitions after the net is loaded and I label them with
	 * the security level. We are assuming transition's names are in
	 * the form name_level so I retrieve the information I need from the name.
	 * The syntax of names is checked within this function, and if an error
	 * is found we abort and warn the user in detail.
	 */
	addLevelToTransitions();

	/* PAOLO: causal reduction applied to the net read from input  */
	/* ALESSANDRO: updated for the multilevel scenario, added static checks to
	 * quickly decide that a net is safe */
	possiblyNotSecure = causal_reduction();
	//reduction = clock();
	//possiblyNotSecure=1;//FIXME added this to run comparison tests, remove it for the release
	if(possiblyNotSecure){	//ALESSANDRO: added this if to optimize on surely secure nets
		secure = unfold();
		//end = clock();//ALESSANDRO
		/* write the output net */
		DPRINT ("  Writing unfolding to '%s'\n", outpath);
		if (! strcmp (outformat, "cuf")) {
			write_cuf (outpath);
		} else if (! strcmp (outformat, "dot")) {
			PRINT ("warn\tsome statics are not well reported under the selected output format\n");
			write_dot (outpath);
		} else {
			PRINT ("warn\tsome statics are not well reported under the selected output format\n");
			write_dot_fancy (outpath);
		}
	}
	else{
		printf("The net is %s\n", policy->isTransitive?"BNDC":"BINI");
	}

#ifdef CONFIG_DEBUG
	db_mem ();
#endif
	rusage ();
	PRINT ("time\t%.3f\n"
		"mem\t%lu\n"

		"hist\t%d\n"
		"events\t%d\n"
		"cond\t%d\n"

		"gen\t%d\n"
		"read\t%d\n"
		"comp\t%d\n"

		"r(h)\t%.2f\n"
		"s(h)\t%.2f\n"
		"co(r)\t%.2f\n"
		"rco(r)\t%.2f\n"
		"mrk(h)\t%.2f\n"

		"pre(e)\t%.2f\n"
		"ctx(e)\t%.2f\n"
		"pst(e)\t%.2f\n"

		"cutoffs\t%d\n"
		"ewhite\t%llu\n"
		"egray\t%llu\n"
		"eblack\t%llu\n"
		"net\t%s\n",

		u.unf.usrtime / 1000.0,
		u.unf.vmsize,

		u.unf.numh - 1,
		u.unf.numev - 1,
		u.unf.numcond,

		u.unf.numgen,
		u.unf.numread,
		u.unf.numcomp,

		u.unf.numr / (float) (u.unf.numh - 1),
		u.unf.nums / (float) (u.unf.numh - 1),
		u.unf.numco / 
			(float) (u.unf.numgen + u.unf.numread + u.unf.numcomp),
		u.unf.numrco / 
			(float) (u.unf.numgen + u.unf.numread + u.unf.numcomp),
		u.unf.nummrk / (float) (u.unf.numh - 1),

		u.unf.numepre / (float) (u.unf.numev - 1),
		u.unf.numecont / (float) (u.unf.numev - 1),
		u.unf.numepost / (float) (u.unf.numev - 1),


		u.unf.numcutoffs,
		u.unf.numewhite,
		u.unf.numegray,
		u.unf.numeblack,
		inpath);
	/*
	 * ALESSANDRO: FIXME adding a little feedback for the tester, comment for release
	 */
	/*FILE * answer;
	answer = fopen("answer.tmp", "w");
	if(answer){//else don't bother
		fprintf(answer, "%d,%.3f,%lu,%d\n", secure, (double)u.unf.usrtime / 1000.0, u.unf.vmsize, u.unf.numh);
		printf("%d,%.3f,%lu,%d\n", secure, (double)u.unf.usrtime / 1000.0, u.unf.vmsize, u.unf.numh);
		fclose(answer);
	}*/
	return EXIT_SUCCESS;
}

