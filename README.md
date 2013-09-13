CFRPlusMatrix
=============

Solves matrix games with uniform random [-1, 1] distribution payoffs using Fictitious play, Counterfactual Regret Minimization
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

./CFRPlusMatrix -s 10 -e 0.0001 -n 50 -all

Compare algorithms. It will output something like this:

Matrix size: 10x10<br />
Epsilon: 0.000100<br />
N: 50<br />
Fictitious play  | min 9500 | max 1690341 | avg 597456.7<br />
CFR              | min 3483 | max 49694  | avg 17850.9<br />
CFR+             | min 127  | max 3657   | avg 968.6<br />




