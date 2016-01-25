====================
The MultiUbic Tool v.0.1
====================
This is a prototypical version of the tool MultiUbic (Multilevel Unfolding-Based
Interference Checker) for checking BNDC (Bisimulation based Non Deducibility on
Composition) and BINI (Bisimulation based Intransitive Non Interference) on safe
Petri nets, in a mutlilevel security domain. 
Both properties are non interference property aimed at characterising the absence
of undesired information flows.

The underlying idea is simple: a system is viewed as consisting of
components at different levels of confidentiality, an a security policy describes
which information flows among them are legal. The absence of a flow of
information from a level H to another level L is captured by asking that the activity 
of H does not determine visible effects, according to some selected
observational semantics, at the low level. Nontheless flow from H
to L is allowed when it is mediated by some other level D toward whom
H is allowed to interfere.

In the BNDC formalization, a process S in deemed free of interference
whenever, for each level L of the domain, S alone, seen from the levels
to whom L cannot interfere, is behaviourally equivalent
to S interacting with any parallel system comprising only levels to whom
L may interfere.
In the BINI formalization, a process S is deemed free of interference
whenever, for each level L in the domain, S from which we removed any other
level toward whom L can legally interfere (S\D), seen from the levels toward whow
L cannot interfere, is behaviourally equivalent to S\D interacting with any parallel
system with only level L, both starting from any reachable state of S.

In the setting of safe Petri nets, transitions are split as many sets as there
are levels.
Both the BNDC and the BINI properties can be characterized in terms of undesired interactions
between transitions of levels not permitted to interfere by the security policy. The tool
exploits a characterisation of such interactions expressed in terms of the unfolding semantics, as 
direct causalities and direct conflicts between those transitions.

The tool is based on Ubic2, a tool set for carrying out unfolding-based
verification of contextual Petri nets of the BINI property in a three level
setting, developed Francesco Burato.

The MultiUbic Tool is developed and maintained by

Alessandro Beggiato
Sysma Research Unit 
IMT School for Advanced Studies Lucca

e-mail : alessandro.beggiato@imtlucca.it


INSTALLATION
------------

In order to install MultiUbic move to the "MultiUbic/Release" folder and type the
following commands:

  make all

The installation puts the binaries to the "MultiUbic/Release" folder from where
you may copy them to suitable locations in your machine.

The MultiUbic folder also includes a folder

  * examples     the examples referred to in the paper 

For input and output file formats, command options we refer to the
cunf documentation (see below)

USAGE 
-----

The input net must be expressed in the PEP ll_net format, and the input
security policy must be specified in MDS format. The output
can be produced in the internal 'cuf' format or as a dot file (simple
or fancy). 

The tool can generate an i-complete prefix or stop once the first (intranitive) weak
causal place is found. When the fancy dot output format is chosen,
weak causal places are depicted in red.

For details on the file formats we refer to the CUNF
documentation. We only mention that when defining the a net to be
checked (in the PEP ll_net format) the confidentiality level of
transitions is determined by the transition name: names are supposed
to be in the format name_level.

Executing MultiUbic without arguments, you'll get an overview of the
command line options.

Usage: ubic [OPTIONS] NETFILE POLICYFILE

Argument NETFILE is a path to the .ll_net input file.
Argument POLICYFILE is a path to the .mds policy file.

The template for MDS_FORMAT is:
MDS
TRANSITIVE/INTRANSITIVE
LVL #levels
0 lvl_name
1 lvl_name
...
#-1 lvl_name
POLICY
i TO j
...

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
-f FORMAT    Write unfolding in format FORMAT. Available formats: 'cuf',
             'dot', 'fancy'.  Default is 'cuf'. In 'fancy' format weak
             causal place representatives are depicted in red



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

