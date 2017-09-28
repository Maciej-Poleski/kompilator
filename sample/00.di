f(a()) :- ;
f(b()) :- ;

f(var X) :- f(a()), f(b()) ;
