plus(var X, z, var X) :- ;
plus(var X, s(var Y), s(var Z)) :- plus(X, Y, Z) ;
