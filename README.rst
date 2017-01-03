====================
The MultiUbic Tool v.0.3
====================

This document is organised as follows:
1. INTRODUCTION: a succint description of MultiUbic
2. INSTALLATION: how to compile MultiUbic
3. USAGE: how to use MultiUbic
4. ADDITIONAL CONTENT: what comes together with MultiUbic
  4.1 Tools
  4.2 Scripts
5. PREVIOUS TOOLS: on what MultiUbic is built upon


1. INTRODUCTION
------------

This is a prototypical version of the tool MultiUbic (Multilevel Unfolding-Based
Interference Checker) for checking BNDC (Bisimulation based Non Deducibility
on Composition ) and BINI (Bisimulation based
Intransitive Non Interference) on safe Petri nets. Both properties are
 non interference property aimed at characterising the absence of
undesired information flows, and are checked with repsect to a security
domain with arbitrary many security levels.

The tool is based on Ubic2, a tool set for carrying out unfolding-based
verification of Petri nets of the BINI property on a three level security domain, 
developed by Paolo Baldan and Francesco Burato.

The MultiUbic Tool is developed and maintained by

Alessandro Beggiato
Sysma Research Unit
IMT School for Advanced Studies Lucca

e-mail : alessandro.beggiato@imtlucca.it


2. INSTALLATION
------------

In order to install MultiUbic move to the folder containing this file and type:

  make all

The installation puts the binaries to the "MultiUbic/bin" folder from where
you may copy them to suitable locations in your machine. Be warned that
the auxiliary scripts provided with this release expects the binaries to be
in "MultiUbic/bin". You can easily edit them to your needs.

If you want to compile only MultiUbic or only the auxiliary tools provided
with it, you can respectively type

 make MultiUbic

or

 make tools


For input and output file formats, command options we refer to the
cunf documentation (see below)

3. USAGE 
-----

The input net must be expressed in the PEP ll_net format, the output
can be produced in the internal 'cuf' format or as a dot file (simple
or fancy). 

The tool can generate an e-complete prefix or stop once the first weak
causal place is found. When the fancy dot output format is chosen,
weak causal places are depicted in red.

For details on the file formats we refer to the CUNF
documentation. 

IMPORTANT NOTICE: when defining the a net to be
checked (in the PEP ll_net format) the confidentiality level of
transitions is determined by the transition name: all transitions
must be named according to a NAME_LEVEL scheme, with
LEVEL defined in the policy file. The format of the policy file
is given below.

Executing MultiUbic without arguments, you'll get an overview of the
command line options.

Usage:
 
  MultiUbic [OPTIONS] NETFILE POLICYFILE

 Argument NETFILE is a path to the .ll_net input file.
 Argument POLICYFILE is a path to the .msd policy file.

 Allowed OPTIONS are:
 -s [on|off]  Stop once the first weak causal place is found and report it.
              (off by default, hence a prefix complete for interferences is
               generated)
 -l NUMBER    Stop when NUMBER histories have been added
 -t NAME      Stop when transition NAME is inserted
 -d DEPTH     Unfold up to given DEPTH
 -o FILE      Output file to store the unfolding in.  If not provided,
              defaults to NETFILE with the last 7 characters removed
              (extension '.ll_net') plus a suffix depending option the -O
 -f FORMAT    Write unfolding in format FORMAT. Available formats: 'cuf'
              'dot', 'fancy'.  Default is 'cuf'. In 'fancy' format weak
              causal place representatives are depicted in red

 The template of the MSD_FORMAT for the policy file is:
	MSD
	TRANSITIVE/INTRANSITIVE
	LVL #levels
	0 lvl_name
	1 lvl_name
	 ...
	#levels-1 lvl_name
	POLICY
	i TO j

The admitted flows are specified by the constraints i TO j, where i and j are the
numerical ids of some security levels. For example, the standard high-low two
level transitive policy is specified by:

	MSD
	TRANSITIVE
	LVL 2
	0 Low
	1 High
	POLICY
	0 TO 1

While the standard three-level intransitive policy is:

	MSD
	TRANSITIVE
	LVL 3
	0 Low
	1 High
	2 Downgrading
	POLICY
	0 TO 1
	0 TO 3
	1 TO 3
	3 TO 0

4. ADDITIONAL CONTENT 
-----

This release of MultiUbic comes equipped with a tool, the partitioner, that maps a
multilevel security domain verification problem to a set of two or three levels 
security domain verification problems, depending on whether the policy
of the original problem is transitive or intransitive.
We also provide a tool to generate the test set we used in the paper.


4.1 Tools
----
To compile both the partitioner and the test set generator, simply run 

  make tools

If you wish you can compile them separately with

  make partitioner

and 

  make testNetGen

They are also compiled together with MultiUbic if you run

  make all

and in every way you compile them, you will find both binaries in the
MultiUbic/bin folder.

Executing them without arguments, you'll get an overview of the
command line options.
The usage for the partitioner is:

  partitioner [OPTIONS] NETFILE POLICYFILE

it expects a .ll_net net file and a .msd policy file and produces a set of .ll_net
file for Ubic2, in a folder at the same path and with the same name of the input net.
If the parameter -m is specified, it will also produce a set of .msd file, as in that
case the output would be suitable for MultiUbic.

The usage of the test set generator is
  
  testNetGen [OPTIONS] OUT_DIR

It generates a single net system at the path specified by OUT_DIR.
The parameters are described in the long version of the paper.
The naming scheme for the generated net system is TestNet-N-L-E.ll_net
where N, L, and E are the specified parameters.
A useful script is provided to generate a big test set at once (see below).
You can of course easily edit it to your needs.

4.2 Scripts

We provide a couple of useful scripts with this release of MultiUibc:

genetateTestSet.sh : this script produces the test set we used in the paper to compare Ubic2
and MultiUbic. It generates the net systems T(n, l, e) with n in [100,300], l in [1,3] and e in [0,6]
(see paper for details). Then it partiotion those net systems into equivalent sets of problems
for Ubic2. It expects to find both the partitioner tool and the testNetGen tool in the
MultiUbic/bin folder, and the default output folder in MultiUbic/TestSystems/T/MultiUbic
for the multilevel domain problems, and MultiUbic/TestSystems/T/UBIC2 for the partitioned
problems. You can edit all these paths in the header of the script.

tester.sh: this is a naive script to compare the running time of MultiUbic versus Ubic2.
By commenting out one of the definitions of the variable netType, you can specify if you 
want to run it on the T systems generated by testNetGen, or on the Dijkstra system 
provided with this release of MultiUbic.
The scripts expects to find a file called netList.txt in the root MultiUbic folder, in which
you specify onto which net the comparison must run. We included an example file
for the first Dijkstra-type systems.
The output of the tester is a simple csv file, placed by default in the folder
containing the net systems you are using for the test.
Once again, you can easily edit all the paths by changing the header of the script.
NOTICE: the tester script expects both Ubic2 and MultiUbic to be located in
the MultiUbic/bin folder. If you want to run the comparison, download Ubic2,
compile it and install it to that folder. 

5. PREVIOUS TOOLS 
-----

--- About the Ubic2 Tool ---

====================
The Ubic2 Tool v.0.1
====================
This is a prototypical version of the tool UBIC2 (Unfolding-Based
Interference Checker 2) for checking BINI (Bisimulation based
Intransitive Non Interference) on safe contextual Petri nets. The property BINI is
a non interference property aimed at characterising the absence of
undesired information flows.

The underlying idea is simple: a system is viewes as consisting of
components at three different levels of confidentiality, an high part
H, which should be secret, a low part, which is public, and a downgrading
part, which represents a declassification event. The absence of a flow
information from H to L is captured by asking that the activity of H
does not determine visible effects, according to some selected
observational semantics, at the low level. Nontheless flow from H
to L is allowed when a declassification or dowgrading event take place,
which could be imagined as the encryption of the information.
In the BINI formalization, a process S is deemed free of interference
whenever S alone, seen from the low level, is behaviourally equivalent
to S interacting with any parallel high level process when S start from
its initial state or any state reachable after a dowgrading event.
In the setting of contextual Petri nets, transitions are split into three
sets: high transitions H, downgrading transitions D and low transitions L.
The BINI property can be characterized in terms of undesired interactions
between high and low transition. The tool exploits a characterisation of
such interaction expressed in terms of the unfolding semantics, as 
direct causalities high level to a low level and direct conflicts between
high and low level transition without downgrading event taking place.

The tool is based on Ubic, a tool set for carrying out unfolding-based
verification of Petri nets of the BNDC property, developed by Paolo
Baldan and Alberto Carraro.

The UBIC2 Tool is developed and maintained by

Francesco Burato
Department of Mathematics
University of Padova

e-mail : francesco.burato.1@studenti.unipd.it


INSTALLATION
------------

In order to install UBIC2 move to the "ubic/Release" folder and type the
following commands:

  make all

The installation puts the binaries to the "ubic/Release" folder from where
you may copy them to suitable locations in your machine.

The ubic folder also includes a folder

  * examples     the examples referred to in the paper 

For input and output file formats, command options we refer to the
cunf documentation (see below)

USAGE 
-----

The input net must be expressed in the PEP ll_net format, the output
can be produced in the internal 'cuf' format or as a dot file (simple
or fancy). 

The tool can generate an e-complete prefix or stop once the first weak
causal place is found. When the fancy dot output format is chosen,
weak causal places are depicted in red.

For details on the file formats we refer to the CUNF
documentation. We only mention that when defining the a net to be
checked (in the PEP ll_net format) the confidentiality level of
transitions is determined by the transition name: names starting by h
correspond to high level transitions, all the others to low level
transitions.

Executing ubic without arguments, you'll get an overview of the
command line options.

Usage:
 
  ubic2 [OPTIONS] NETFILE

Argument NETFILE is a path to the .ll_net input file.  Allowed OPTIONS are:

 -s [on|off]  Stop once the first weak causal place is found and report it.
              (off by default, hence a prefix complete for interferences is
               generated)
 -d DEPTH     Unfold up to given DEPTH
 -o FILE      Output file to store the unfolding in.  If not provided,
              defaults to NETFILE with the last 7 characters removed
              (extension '.ll_net') plus a suffix depending option the -O
 -f FORMAT    Write unfolding in format FORMAT. Available formats: 'cuf',
              'dot', 'fancy'.  Default is 'cuf'. In 'fancy' format weak
              causal place representatives are depicted in red





--- About the Ubic Tool ---

===================
The Ubic Tool v.0.1
===================

This is a prototypical version of the tool UBIC (Unfolding-Based
Interference Checker) for checking BNDC (Bisimulation based Non
Deducibility on Composition) on safe Petri nets. The property BNDC is
a non interference property aimed at characterising the absence of
undesired information flows.

The underlying idea is simple: a system is viewed as consisting of
components at different levels of confidentiality, in the simplest
case a high part H, which intuitively should be secret, and a low part
L, which is public. The absence of a flow of information from H to L
is captured by asking that the activity of H does not determine
visible effects, according to some selected observational semantics,
at low level L. In the BNDC formalisation, a process S is deemed free
of interferences whenever S alone, seen from the low level, is
behaviourally equivalent to S interacting with any parallel high level
process. In the setting of Petri nets, transitions are split into two
sets: high transitions H and low transitions L. The BNDC property can
be characterized in terms of undesired interactions between high and
low transitions. The tool exploit a characterisation of such
interactions expressed in terms of the unfolding semantics, as direct
causalities high level to a low level transitions and direct conflicts
between high and low level transitions.

The tool is based on Cunf a tool set for carrying out unfolding-based
verification of Petri nets extended with read arcs, also called
contextual nets, or c-nets, developed by Cesar Rodriguez.

The UBIC Tool is developed and maintained by

Paolo Baldan
Department of Mathematics
University of Padova

WWW    : http://www.math.unipd.it/~baldan/
e-mail : baldan@math.unipd.it

Alberto Carraro
DAIS University Ca' Foscari Venice - 
Laboratoire PPS, Universite Paris Diderot, Paris Sorbonne Cite

WWW    : http://www.dsi.unive.it/~acarraro/
e-mail : acarraro@dsi.unive.it


INSTALLATION
------------

In order to install UBIC move to the ubic-0.1 folder and type the
following commands:

  make all
  make dist

The installation puts the binaries to the "dist/bin" folder from where
you may copy them to suitable locations in your machine.

The ubic-0.1 folder also includes a folder

* examples     the examples referred to in the paper 
* binaries     pre-compiled binaries for linux and mac osx


For input and output file formats, command options we refer to the
cunf documentation (see below)

USAGE 
-----

The input net must be expressed in the PEP ll_net format, the output
can be produced in the internal 'cuf' format or as a dot file (simple
or fancy). 

The tool can generate an e-complete prefix or stop once the first weak
causal place is found. When the fancy dot output format is chosen,
weak causal places are depicted in red.

For details on the file formats we refer to the CUNF
documentation. We only mention that when defining the a net to be
checked (in the PEP ll_net format) the confidentiality level of
transitions is determined by the transition name: names starting by h
correspond to high level transitions, all the others to low level
transitions.

Executing ubic without arguments, you'll get an overview of the
command line options.

Usage: ubic [OPTIONS] NETFILE

Argument NETFILE is a path to the .ll_net input file.  Allowed OPTIONS are:
 -s [on|off]  Stop once the first weak causal place is found and report it.
              (off by default, hence a prefix complete for interferences is
               generated)
 -d DEPTH     Unfold up to given DEPTH
 -o FILE      Output file to store the unfolding in.  If not provided,
              defaults to NETFILE with the last 7 characters removed
              (extension '.ll_net') plus a suffix depending option the -O
 -f FORMAT    Write unfolding in format FORMAT. Available formats: 'cuf',
              'dot', 'fancy'.  Default is 'cuf'. In 'fancy' format weak
              causal place representatives are depicted in red





--- About the Cunf Tool ---
===================
The Cunf Tool v.1.6
===================

The Cunf Tool is actually a set of tools for carrying out unfolding-based
verification of Petri nets extended with read arcs, also called contextual
nets, or c-nets.  The package specifically contains the tools:

 - cunf: constructs the unfolding of a c-net;
 - cna: performs reachability and deadlock analysis using unfoldings
   constructed by cunf;
 - Scripts such as pep2dot or grml2pep to do format conversion between
   various Petri net formats, unfolding formats, etc.

Cunf is written in C, the sources are in src/ and /include. Cna is
written in python, and depends on the "ptnet" module; both are located
in the tools/ folder.

Cna requires the Minisat solver to be in the $PATH.  For your
convenience, the source code of Minisat v.2.2.0 is present in the
minisat/ folder, and the main Makefile will compile it for you.

Authors and Contact
-------------------

The Cunf Tool is developed and maintained by

César Rodríguez
LSV, CNRS & ENS de Cachan
61, avenue du Président Wilson
94235 CACHAN Cedex, France

WWW    : http://www.lsv.ens-cachan.fr/~rodriguez/
e-mail : cesar.rodriguez@lsv.ens-cachan.fr

Quick Installation
------------------

Full details about the installation are given in section 3 of the manual:

https://cunf.googlecode.com/files/user-manual-v1.6.pdf

The installation puts all binaries and libraries into the "dist/"
folder, from where you may copy them to suitable locations in your
machine.

Type the following commands::

  make all
  make dist

After that, make available to Python the module

dist/lib/ptnet,

by copying it to any folder pointed by your installation-dependent
default module search path, or any folder pointed by the environment
variable PYTHONPATH.  Cna and other Python scripts won't work without this
step.

Documentation
-------------

See https://cunf.googlecode.com/files/user-manual-v1.6.pdf

Development
-----------

The Cunf Tool is hosted at https://code.google.com/p/cunf/.
You can get the latest source code typing in your terminal::

  git clone https://code.google.com/p/cunf/ 
