/*
 * CAPMath.h
 *
 *  Created on: Feb 20, 2009
 *      Author: jchu014
 */

#ifndef CAPMATH_H_
#define CAPMATH_H_

#include <ostream>

struct Point3D
{
	float x,y,z;
	Point3D(float x_, float y_, float z_)
	: x(x_),y(y_),z(z_)
	{};
	Point3D()
	{};
	friend std::ostream& operator<<(std::ostream &_os, const Point3D &val);
};

inline std::ostream& operator<<(std::ostream &os, const Point3D &val)
{
  os << "( " << val.x << ", " << val.y << ", " << val.z << ") ";
  return os;
};

template <typename T>
inline void FIND_VECTOR(T& ans,const T& from,const T& to)
{
	ans.x = to.x - from.x;
	ans.y = to.y - from.y;
	ans.z = to.z - from.z;
}

//#define DOT(a,b) ( (a).x*(b).x + (a).y*(b).y + (a).z*(b).z )
template <typename T>
inline float DOT(const T& a, const T& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

#define CROSS_PRODUCT(ans, vec1, vec2) \
(ans).x = (vec1).y*(vec2).z - (vec1).z*(vec2).y; \
(ans).y = (vec1).z*(vec2).x - (vec1).x*(vec2).z; \
(ans).z = (vec1).x*(vec2).y - (vec1).y*(vec2).x

#define NORMALISE(vec) \
{ \
  double length = sqrt(DOT((vec), (vec))); \
  (vec).x = (vec).x / length; \
  (vec).y = (vec).y / length; \
  (vec).z = (vec).z / length; \
}

#endif /* CAPMATH_H_ */
