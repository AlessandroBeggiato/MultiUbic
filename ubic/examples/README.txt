------------------
Some examples nets
------------------

Some example nets are provided in 'll_net' format, the input format of
UBIC and in 'ifn' format (derived from the lola format), the input
format of ANICA.

* netDijkstra-n
Dijkstra mutual exclusion algorthm, with n processes

* netLine-n
A sequential net with n+2 transitions in a chain
  l1 ... ln h_{n+1} l_{n+2}
It has an interference, no concurrency hence unfolding should not
provide great advantages
 

* netR-n-m
A variant of the mutual exclusion net in the paper, with n high and m
low processes.
