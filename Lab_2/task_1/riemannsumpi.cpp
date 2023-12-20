#include <assert.h>
#include <emmintrin.h>
#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

constexpr double correctpi = 3.14159265358979323846264338327950288419716939937510;

inline double f(const double x) {
    return sqrt(1 - x * x);
}
inline __m128d f2(const __m128d Dx, unsigned long i) {
    // vectorized version of function f
    __m128d x = _mm_mul_pd(_mm_set_pd(i, i + 1), Dx);
    return _mm_sqrt_pd(_mm_sub_pd(_mm_set1_pd(1.0d), _mm_mul_pd(x, x)));
}

double riemann_seq(const unsigned long n) {
    const double Dx = 1.0 / n;
    double sum = 0.0;
    for (unsigned long i = 0; i < n; ++i)
        sum += Dx * f(i * Dx);
    return sum;
}

double omp_riemann_seq(const unsigned long n) {
    const double Dx = 1.0 / n;
    double sum = 0.0;
#pragma omp parallel
    {
        double partialSum = 0;

#pragma omp for nowait
        for (unsigned long i = 0; i < n; ++i)
            partialSum += Dx * f(i * Dx);

#pragma omp atomic
        sum += partialSum;
    }

    return sum;
}

double omp_vec_riemann_seq(const unsigned long n) {
    const __m128d Dx = _mm_set_pd1(1.0d / n);
    __m128d sum = _mm_setzero_pd();

#pragma omp parallel
    {
        __m128d partialSum = _mm_setzero_pd();

#pragma omp for nowait
        // n will always be divisible by 2 due to the assert in main
        for (unsigned long i = 0; i < n; i += 2) {
            partialSum = _mm_add_pd(partialSum, _mm_mul_pd(Dx, f2(Dx, i)));
        }

#pragma omp critical
        { sum = _mm_add_pd(sum, partialSum); }
    }

    double result[2] = {0.0d};
    _mm_storeu_pd(result, sum);
    return (result[0] + result[1]);
}

void test_unchanged(unsigned long n) {
    printf("\nUnchanged version: ");
    double pi;

    double ts = omp_get_wtime();
    pi = 4.0 * riemann_seq(n);
    ts = omp_get_wtime() - ts;
    printf("seq, elapsed time = %.3f seconds, err = %.15f\n", ts, abs(correctpi - pi));
}

void test_omped(unsigned long n) {
    printf("\nOpenMP version: ");
    double pi;

    double ts = omp_get_wtime();
    pi = 4.0 * omp_riemann_seq(n);
    ts = omp_get_wtime() - ts;
    printf("seq, elapsed time = %.3f seconds, err = %.15f\n", ts, abs(correctpi - pi));
}

void test_omp_veced(unsigned long n) {
    printf("\nOpenMP + Vec version: ");
    double pi;

    double ts = omp_get_wtime();
    pi = 4.0 * omp_vec_riemann_seq(n);
    ts = omp_get_wtime() - ts;
    printf("seq, elapsed time = %.3f seconds, err = %.15f\n", ts, abs(correctpi - pi));
}

int main(int argc, char *argv[]) {

    const unsigned long n = strtoul(argv[1], nullptr, 0);
    const int nproc = omp_get_max_threads();
    assert(n > 0 && n % 2 == 0);

    printf("n = %lu\n", n);
    printf("nproc = %d\n", nproc);

    test_unchanged(n);
    test_omped(n);
    test_omp_veced(n);

    return EXIT_SUCCESS;
}
