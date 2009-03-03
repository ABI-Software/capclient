/*
 * CAPMath.h
 *
 *  Created on: Feb 20, 2009
 *      Author: jchu014
 */

#ifndef CAPMATH_H_
#define CAPMATH_H_

#include <ostream>

class Point3D
{
public:
	float x,y,z;
	Point3D(float x_, float y_, float z_)
	: x(x_),y(y_),z(z_)
	{};
	Point3D()
	{};
	Point3D(const Point3D& other)
	:x(other.x), y(other.y), z(other.z)
	{};
	
	Point3D operator+(const Point3D& rhs) const
	{
		return Point3D(x+rhs.x, y+rhs.y, z+rhs.z);
	}
	
	Point3D& operator=(const Point3D& rhs)
	{
		if (this != &rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
		}
		return *this;
	}
	
	friend std::ostream& operator<<(std::ostream &_os, const Point3D &val);
};

inline Point3D operator*(float scalar, const Point3D& rhs)
{
	return Point3D(scalar*rhs.x, scalar*rhs.y, scalar*rhs.z);
}

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
