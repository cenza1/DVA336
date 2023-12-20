#pragma once

#include <cassert>
#include <cstdlib>
#include <cmath>

static inline double sq(double x) 
{
	return x*x;
}

struct vector
{
	// constructor
	vector(const int n) : n(n)
	{
		assert(n > 0 && n % 8 == 0);
		data = (double*)malloc(sizeof(double) * n);
		assert(data);
	}
	// destructor
	~vector()
	{
		free(data);
	}
	// set a point of the mesh
	void set(double x, int i)
	{
		assert(i >= 0 && i < n);
		data[i] = x;
	}
	static double cosine_similarity(const vector &a,const vector &b) {
		assert(a.n==b.n);
		const int n = a.n;
		double ab = 0.0f;
		double aa = 0.0f;
		double bb = 0.0f;
		for(int i=0; i<n; ++i)
		{
			aa += sq(a.data[i]);
			bb += sq(b.data[i]);
			ab += a.data[i]*b.data[i];
		}
		return ab/(sqrtf(aa)*sqrtf(bb));
   }

private:
	const int n = 0;
	double *data = nullptr;
};
