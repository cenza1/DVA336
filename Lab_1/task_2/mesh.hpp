#pragma once

#include "common.hpp"

#include <cassert>
#include <cstdlib>
#include <cmath>

// return square of x
static inline float sq(const float &x)
{
	return x * x;
}

// return square distance between two d-dimensional points
template <int d> static inline float sqdst(const point<d> &a, const point<d> &b)
{
	float sum = 0.0f;
	for (int i = 0; i < d; ++i)
		sum += sq(a.v[i] - b.v[i]);
	return sum;
}

// mesh of n d-dimensional points stored according to the AoS layout
template <const int d> struct mesh
{
	// constructor
	mesh(const int n) : n(n)
	{
		assert(n > 0 && n % 16 == 0);
		data = (point<d> *)malloc(sizeof(point<d>) * n);
		assert(data);
	}
	// destructor
	~mesh()
	{
		free(data);
	}
	// set a point of the mesh
	void set(const point<d> p, int i)
	{
		assert(i >= 0 && i < n);
		data[i] = p;
	}
	// calculate center and radius of the ball enclosing the points
	ball<d> calc_ball()
	{
		ball<d> b;
		// calculate the center
		point<d> min = data[0];
		point<d> max = data[0];
		for (int i = 1; i < n; ++i)
			for (int j = 0; j < d; ++j)
				min.v[j] = data[i].v[j] < min.v[j] ? data[i].v[j] : min.v[j],
				max.v[j] = data[i].v[j] > max.v[j] ? data[i].v[j] : max.v[j];
		for (int i = 0; i < d; ++i)
			b.center.v[i] = (max.v[i] - min.v[i]) * 0.5f + min.v[i];
		// calculate the radius
		float tmp, maxsqdst = 0.0f;
		for (int i = 0; i < n; ++i)
		{
			tmp = sqdst(data[i], b.center);
			maxsqdst = maxsqdst > tmp ? maxsqdst : tmp;
		}
		b.radius = sqrtf(maxsqdst);
		// return the enclosing ball
		return b;
	}
	// return the index of the farthest point from the given point p (OPTIONAL)
	int farthest(point<d> p)
	{
		int argmax = 0;
		float maxsqdst = sqdst(data[0], p);
		for (int i = 1; i < n; ++i)
		{
			float sqdsti = sqdst(data[i], p);
			if (sqdsti > maxsqdst)
			{
				maxsqdst = sqdsti;
				argmax = i;
			}
		}
		return argmax;
	}

private:
	const int n = 0;
	point<d> *data = nullptr;
};
