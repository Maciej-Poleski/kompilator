// number of edges outgoing from vertex
int r = edges(+int vertex) extern "C"

// edge outgoing from vertex, 0 <= index < edges(vertex)
int r = edge(+int vertex, +int index) extern "C"

// check if path from v to u exists
path(int v, int v) :- .
path(int v, int u) :- edge(v, u).

// try...failed syntax doesn't exist in language
// but can be simulated as shown below
// path(+int v, +int u) {
//   +bool succeeded = false;
//   for(+int i=0, i<edges(v); i=i+1) {
//     try {
//       path(edge(v, i), y);
//       succeeded = true;
//       break;
//     } failed {
//       continue;
//     }
//   }
//   if(!succeeded) {
//     fail();
//   }
// }

path(int v, int u) :- iterate(v, u, 0).

iterate(int x, int y, int i) :- lt(i, $edges(x)), path($edge(x, i), y).
iterate(int x, int y, int i) :- lt(i, $edges(x)), iterate(x, y, $inc(i)).

lt(+int a, +int b) {
  if(a>=b) {
    fail();
  }
}

int r = inc(+int a) {
  r === a+1;
}
