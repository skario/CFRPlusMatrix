CFRPlusMatrix
=============

Solves random zero-sum matrix games using Fictitious play, Counterfactual Regret Minimization and CFR+.

#####Building

make

#####Parameters

* `-h` show help
* `-a <id>` algorithm
    + `-a 0` Fictitious play
    + `-a 1` CFR
    + `-a 2` CFR+ (default)
* `-s <n>` matrix dimensions (n-by-n) (default 1000)
* `-distribution <id>` random number distribution
    + `-distribution 0` uniform \[-1, 1\] (default)
    + `-distribution 1` normal (sigma 0.5)
    + `-distribution 2` cauchy (gamma 0.02)
* `-e <epsilon>` epsilon (default 0.0001)
* `-n <n>` number of times to run
* `-all` run all algorithms (used together with -n)
* `-dump` print payoffs and strategies when done
* `-delay <n>` CFR+ averaging delay in number of iterations (default 0)
* `-w <id>` CFR+ weighting mode
    + `-w 0` constant (like in CFR)
    + `-w 1` linear (default)
    + `-w 2` quadratic (often faster than linear but can get stuck in small games)

#####Examples

`./CFRPlusMatrix -a 0 -s 10 -e 0.001`

Use algorithm 0 (Fictitious play) to find an epsilon-equilibrium with epsilon of 0.001 in a 10x10 game.

`./CFRPlusMatrix -a 1 -s 100 -e 0.0001`

Use algorithm 1 (CFR) to find an epsilon-equilibrium with epsilon of 0.0001 in a 100x100 game.

`./CFRPlusMatrix -a 2 -s 1000 -e 0.00001`

Use algorithm 2 (CFR+) to find an epsilon-equilibrium with epsilon of 0.00001 in a 1000x1000 game.

`./CFRPlusMatrix -s 10 -e 0.0001 -n 50 -all`

Compare algorithms. It will output something like this:

    Matrix size: 10x10
    Epsilon: 0.000100
    N: 50
    Fictitious play  | min 9500 | max 1690341 | avg 597456.7
    CFR              | min 3483 | max 49694  | avg 17850.9
    CFR+             | min 127  | max 3657   | avg 968.6

See http://jeskola.net/cfr/ for more stuff.


