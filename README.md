CFRPlusMatrix
=============

Solves matrix games with uniform random [-1, 1] distribution using Fictitious play, Counterfactual Regret Minimization
and CFR+.

Building:

make

Examples:

./CFRPlusMatrix -a 0 -s 10 -e 0.001

Use algorithm 0 (Fictitious play) to find an epsilon-equilibrium with epsilon of 0.001 in a 10x10 game.

./CFRPlusMatrix -a 1 -s 100 -e 0.0001

Use algorithm 1 (CFR) to find an epsilon-equilibrium with epsilon of 0.0001 in a 100x100 game.

./CFRPlusMatrix -a 2 -s 1000 -e 0.00001

Use algorithm 2 (CFR+) to find an epsilon-equilibrium with epsilon of 0.00001 in a 1000x1000 game.

