/* DO NOT CHANGE THIS FILE */

#include "mesh_vectorized.hpp"

#include <ctime>

template <const int d> inline point<d> randompoint() {
  point<d> p;
  for (int j = 0; j < d; ++j)
    p.v[j] = 2.0f * rand() / RAND_MAX;
  return p;
}

// D is a parameter arbitrarily set at compile time in the command line
#ifndef D
#define D 3 // default value (can be changed)
#endif

int main(int argc, char *argv[]) {
  srand(12345);
  const int n = atoi(argv[1]);
  mesh<D> m(n);
  for (int i = 0; i < n; ++i)
    m.set(randompoint<D>(), i);
  clock_t t = clock();
  ball<D> b = m.calc_ball();
  t = clock() - t;
  std::cout << "The enclosing ball is " << b
            << "; elapsed time=" << 1000.0 * t / CLOCKS_PER_SEC << "ms"
            << std::endl;
  t = clock();
  int i = m.farthest(b.center);
  t = clock() - t;
  std::cout << "The farthest point from the ball center is #" << i
            << "; elapsed time=" << 1000.0 * t / CLOCKS_PER_SEC << "ms"
            << std::endl;
  return 0;
}
