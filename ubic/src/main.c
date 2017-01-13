/*
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

// FRANCESCO
struct high_hash * high_hash = NULL;
// \FRANCESCO

/*****************************************************************************/

void usage (void)
{
	fprintf(stderr,
"\n"
"The ubic2 tool -- an unfolding based intransitive interference checker\n"
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
"Usage: ubic [OPTIONS] NETFILE\n"
"\n"
"Argument NETFILE is a path to the .ll_net input file.  Allowed OPTIONS are:\n"
" -s [on|off]  Stop once the first weak causal place is found and report it.\n"
"              (off by default, hence a prefix complete for interferences is\n"
"               generated)\n"
//" -t NAME    Stop when transition NAME is inserted\n"
" -d DEPTH     Unfold up to given DEPTH\n"
" -o FILE      Output file to store the unfolding in.  If not provided,\n"
"              defaults to NETFILE with the last 7 characters removed\n"
"              (extension '.ll_net') plus a suffix depending option the -O\n"
" -f FORMAT    Write unfolding in format FORMAT. Available formats: 'cuf',\n"
"              'dot', 'fancy'.  Default is 'cuf'. In 'fancy' format weak\n"
"              causal place representatives are depicted in red\n"
"\n"
"For more information, see http://www.math.unipd.it/~baldan/UBIC\n"
"Branch eager v0.1, compiled %s\n", __DATE__);

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

// FRANCESCO
/*
 * Compute number of high transitions present in the net
 * and setup the table for accessing the element in it
 */
void manage_high(void) {
  struct trans * t;
  struct ls * n;
  struct high_hash * hashentry;
  // initialize counter to 0
  u.numhigh = 0;
  for (n = u.net.trans.next; n; n = n->next) {
    // get the transition and test the first character
    t = ls_i (struct trans, n, nod);
    if(t->name[0]=='h') {
      // setting the index of the hash_entry
      hashentry = (struct high_hash *) gl_malloc(sizeof(struct high_hash));
      hashentry->hname = t->name;
      hashentry->index = u.numhigh;
      HASH_ADD_KEYPTR(hh , high_hash , hashentry->hname , strlen(hashentry->hname), hashentry);
      // increment counter of high transitions
      u.numhigh++;
    }
  }
}
// \FRANCESCO

int main (int argc, char **argv)
{
	int opt, l;
	char *stoptr, *inpath, *outpath, *outformat;
	

	/* initialize global parameters */
	u.mark = 2;

	/* PAOLO: additional global parameter: generate full unfolding?
	   Default is yes. */
	u.complete=1;

	/* parse command line */
	stoptr = 0;
	inpath = 0;
	outpath = 0;
	outformat = 0;
	u.stoptr = 0;
	u.depth = 0;
	while (1) {
		opt = getopt (argc, argv, "s:o:t:d:f:");
		if (opt == -1) break;
		switch (opt) {
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
	if (optind != argc - 1) usage ();
	inpath = argv[argc - 1];

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

	/* load the input net */
	DPRINT ("  Reading net from '%s'\n", inpath);
	read_pep_net (inpath);
	DPRINT ("  It is a %s net\n", u.net.isplain ? "plain" : "contextual");
	nc_static_checks (stoptr);

	/* PAOLO: causal reduction applied to the net read from input  */
	causal_reduction();

	/* FRANCESCO */
	// Computing the number of high transitions
	manage_high();
	/*/FRANCESCO */
	/* unfold */
	unfold ();

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

	return EXIT_SUCCESS;
}

