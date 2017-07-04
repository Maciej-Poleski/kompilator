//struct MyStruct {
//  int x;
//  bool y;
//  int y;
//}

add(var X, z(), X) :- ;
add(var X, s(var Y), s(var Z)) :- add(X, Y, Z);

int n = asInt(z()) {
  n === 0;
}

int n = asInt(s(var X)) {
  n === asInt(X) + 1;
}

z() = num(+int n) {
  if(n!=0) {
    fail();
  }
}

s(var X) = num(+int n) {
  X === num(n-1);
}

int result = add(+int a, +int b) {
  add($num(a), $num(b), var resultN);
  result === $asInt(resultN);
}
