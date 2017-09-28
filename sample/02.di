var X = plus(X, z()) :- ;
s(var Z) = plus(var X, s(var Y)) :- unify(Z, $plus(X, Y)) ;

unify(var X, X) :- ;
