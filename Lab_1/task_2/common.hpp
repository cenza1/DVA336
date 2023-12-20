/* DO NOT CHANGE THIS FILE */

#pragma once 

#include <iostream>
#include <iomanip>
#include <cassert>

// d-dimensional point representation
template <int d> struct point
{
	float v[d];
};

// overloading the << operator for 'point'
template <int d> std::ostream &operator<<(std::ostream &os, const point<d> &p)
{
	os << "(" << p.v[0];
	for (int i = 1; i < d; ++i)
		os << ',' << p.v[i];
	os << ")";
	return os;
}

// d-dimensional ball representation
template <int d> struct ball
{
	point<d> center;
	float radius;
};

// overloading the << operator for 'ball'
template <int d> std::ostream &operator<<(std::ostream &os, const ball<d> &b)
{
	const auto old = std::cout.precision();
	os << "center=" << b.center << " radius=" << std::setprecision(10) << b.radius << std::setprecision(old);
	return os;
}
