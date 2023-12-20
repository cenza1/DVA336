#pragma once

#include "common.hpp"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <xmmintrin.h>

template <int d> struct pointsSOA {
  float *v[d];
  int size;
};

// return square of x
static inline float sq(const float &x) { return x * x; }

// return max square distance in an array of d-dimensional points, compared to a
// specific point b.
template <int d>
static inline float max_sqdst(const pointsSOA<d> &a, const point<d> &b) {
  __m128 max_vector = _mm_setzero_ps();

  // The loop does four calculations at a time since 128-bit vectors fit four
  // floats. j will always be divisible by 4 since the n in the mesh constructor
  // has to be divisible by 16.
  for (int j = 0; j < a.size; j += 4) {
    __m128 sum = _mm_setzero_ps();

    for (int i = 0; i < d; i++) {
      __m128 b_vec = _mm_set1_ps(b.v[i]);
      __m128 a_vec = _mm_loadu_ps(&a.v[i][j]);
      __m128 subtosq = _mm_sub_ps(a_vec, b_vec);
      sum = _mm_add_ps(sum, _mm_mul_ps(subtosq, subtosq));
    }

    // Save the four largest values of all iterations.
    max_vector = _mm_max_ps(sum, max_vector);
  }

  // Check which element in max_vector that is the largest.
  float result[4] = {0.0f};
  _mm_storeu_ps(result, max_vector);
  float max = result[0];
  for (int i = 1; i < 4; i++) {
    if (result[i] > max) {
      max = result[i];
    }
  }

  return max;
}

// Return the index of a point in a that is the furthest away from b
template <int d>
static inline int index_of_furthest(const pointsSOA<d> &a, const point<d> &b) {
  int maxIndex = 0;
  float maxValue = 0.0f;

  for (int j = 0; j < a.size; j += 4) {
    __m128 sum = _mm_setzero_ps();

    for (int i = 0; i < d; i++) {
      __m128 b_vec = _mm_set1_ps(b.v[i]);
      __m128 a_vec = _mm_loadu_ps(&a.v[i][j]);
      __m128 subtosq = _mm_sub_ps(a_vec, b_vec);
      sum = _mm_add_ps(sum, _mm_mul_ps(subtosq, subtosq));
    }

    // Results have to be stored and checked every loop to know which index
    // has the highest value.
    float result[4] = {0.0};
    _mm_storeu_ps(result, sum);
    for (int k = 0; k < 4; k++) {
      if (result[k] > maxValue) {
        maxIndex = j + k;
        maxValue = result[k];
      }
    }
  }

  return maxIndex;
}

// return square distance between two d-dimensional points
template <int d>
static inline float sqdst(const point<d> &a, const point<d> &b) {
  float sum = 0.0f;
  for (int i = 0; i < d; ++i)
    sum += sq(a.v[i] - b.v[i]);
  return sum;
}

// mesh of n d-dimensional points stored according to the AoS layout
template <const int d> struct mesh {
  // constructor
  mesh(const int n) : n(n) {
    assert(n > 0 && n % 16 == 0);
    for (int i = 0; i < d; i++) {
      data.v[i] = (float *)malloc(sizeof(float) * n);
      assert(data.v[i]);
    }
    data.size = n;
  }

  // destructor
  ~mesh() {
    for (int i = 0; i < d; i++) {
      free(data.v[i]);
    }
  }

  // set a point of the mesh
  void set(const point<d> p, int i) {
    assert(i >= 0 && i < n);
    for (int j = 0; j < d; j++) {
      data.v[j][i] = p.v[j];
    }
  }

  // calculate center and radius of the ball enclosing the points
  ball<d> calc_ball() {
    ball<d> b;
    // calculate the center
    float min[d] = {0.0f};
    float max[d] = {0.0f};

    for (int i = 0; i < d; i++) {
      min[i] = data.v[i][0];
      max[i] = data.v[i][0];
    }

    for (int i = 0; i < d; i++) {
      __m128 max_vector = _mm_setzero_ps();
      __m128 min_vector = _mm_setzero_ps();

      for (int j = 0; j < n; j += 4) {
        __m128 loaded_values = _mm_loadu_ps(&data.v[i][j]);

        // Save the 4 largest and 4 smallest values per dimension
        max_vector = _mm_max_ps(loaded_values, max_vector);
        min_vector = _mm_min_ps(loaded_values, min_vector);
      }

      // Check which value is the smalles for current dimension
      float maxResult[4] = {0.0f};
      float minResult[4] = {0.0f};
      _mm_storeu_ps(maxResult, max_vector);
      _mm_storeu_ps(minResult, min_vector);

      for (int k = 0; k < 4; k++) {
        min[i] = minResult[k] < min[i] ? minResult[k] : min[i];
        max[i] = maxResult[k] > max[i] ? maxResult[k] : max[i];
      }
    }

    int i = 0;
    __m128 half_float = _mm_set1_ps(0.5);

    // Use efficient calculations for the center point when the amount
    // of dimensions as long as there are more than 3 dimensions to work with.
    for (; i < d - 3; i += 4) {
      __m128 minVec = _mm_loadu_ps(&min[i]);
      __m128 maxVec = _mm_loadu_ps(&max[i]);
      __m128 resultVec = _mm_mul_ps(half_float, _mm_sub_ps(maxVec, minVec));
      resultVec = _mm_add_ps(resultVec, minVec);
      _mm_storeu_ps(&b.center.v[i], resultVec);
    }
    for (; i < d; ++i)
      b.center.v[i] = (max[i] - min[i]) * 0.5f + min[i];

    // calculate the radius
    float maxsqdst = max_sqdst(data, b.center);
    b.radius = sqrtf(maxsqdst);

    // return the enclosing ball
    return b;
  }

  // return the index of the farthest point from the given point p (OPTIONAL)
  int farthest(point<d> p) { return index_of_furthest(data, p); }

private:
  const int n = 0;
  pointsSOA<d> data;
};
