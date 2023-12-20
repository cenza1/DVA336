#pragma once

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <emmintrin.h>

static inline double sq(double x) { return x * x; }

struct vector {
  // constructor
  vector(const int n) : n(n) {
    assert(n > 0 && n % 8 == 0);
    data = (double *)malloc(sizeof(double) * n);
    assert(data);
  }

  // destructor
  ~vector() { free(data); }

  // set a point of the mesh
  void set(double x, int i) {
    assert(i >= 0 && i < n);
    data[i] = x;
  }

  static double cosine_similarity(const vector &a, const vector &b) {
    assert(a.n == b.n);
    const int n = a.n;

    // Set the vectors to be filled with zero-values as the multiplication
    // results will accumulate in the vectors.
    __m128d ab = _mm_setzero_pd();
    __m128d aa = _mm_setzero_pd();
    __m128d bb = _mm_setzero_pd();

    // All elements should be processed even if i is incrimented in strides of 2
    // since n has to divisible by 8 on vector construction.
    for (int i = 0; i < n; i += 2) {
      __m128d a_vec = _mm_set_pd(a.data[i], a.data[i + 1]);
      aa = _mm_add_pd(aa, _mm_mul_pd(a_vec, a_vec));

      __m128d b_vec = _mm_set_pd(b.data[i], b.data[i + 1]);
      bb = _mm_add_pd(bb, _mm_mul_pd(b_vec, b_vec));

      ab = _mm_add_pd(ab, _mm_mul_pd(a_vec, b_vec));
    }

    __m128d vec_result =
        _mm_div_pd(ab, _mm_mul_pd(_mm_sqrt_pd(aa), _mm_sqrt_pd(bb)));

    // The calculated result needs to be stored in an array of 2 elements since
    // two dubles fit in a 128-bit vector.
    double result[2] = {0.0};
    _mm_storeu_pd(result, vec_result);
    return (result[0] + result[1]) / 2;
  }

private:
  const int n = 0;
  double *data = nullptr;
};
