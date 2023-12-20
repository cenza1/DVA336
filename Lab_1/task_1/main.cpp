/* DO NOT CHANGE THIS FILE */

#include "vector_vectorized.hpp"

#include <ctime>
#include <iostream>

int main(int argc, char *argv[]) {
  srand(1234);
  const int n = atoi(argv[1]);
  vector a(n);
  vector b(n);
  for (int i = 0; i < n; ++i)
    a.set(1.0f * rand() / RAND_MAX, i), b.set(1.0f * rand() / RAND_MAX, i);
  clock_t t = clock();
  float x = vector::cosine_similarity(a, b);
  t = clock() - t;
  std::cout << "n = " << n << "; cosine_similarity = " << x
            << "; elapsed time = " << 1000.0 * t / CLOCKS_PER_SEC << "ms"
            << std::endl;
  return 0;
}
