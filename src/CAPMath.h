/*
 * CAPMath.h
 *
 *  Created on: Feb 20, 2009
 *      Author: jchu014
 */

#ifndef CAPMATH_H_
#define CAPMATH_H_

#include <iostream>
#include <cmath>

typedef float gtMatrix[4][4];
#ifdef FE_VALUE_IS_DOUBLE
typedef double Real;
#else
typedef float Real;
#endif //FE_VALUE_IS_DOUBLE

namespace cap
{

class Vector3D
{
public:
	Vector3D() :
		x(0), y(0), z(0) {
	}

	Vector3D(Real x_, Real y_, Real z_) :
		x(x_), y(y_), z(z_) {
	}

	Real Length() const {
		return sqrtf(x * x + y * y + z * z);
	}

	void Normalise() {
		Real length = Length();
		x /= length;
		y /= length;
		z /= length;
	}

	void CrossProduct(const Vector3D& vec1, const Vector3D& vec2) {
		x = (vec1).y * (vec2).z - (vec1).z * (vec2).y;
		y = (vec1).z * (vec2).x - (vec1).x * (vec2).z;
		z = (vec1).x * (vec2).y - (vec1).y * (vec2).x;
	}

	// arithmetic operations
	inline Vector3D operator +(const Vector3D& rkVector) const {
		return Vector3D(x + rkVector.x, y + rkVector.y, z + rkVector.z);
	}

	Vector3D operator-(const Vector3D& rhs) const {
		return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	inline Vector3D operator *(const Real fScalar) const {
		return Vector3D(x * fScalar, y * fScalar, z * fScalar);
	}

	inline Real operator *(const Vector3D& rhs) const {
		return x * rhs.x + y * rhs.y + z * rhs.z;
	}

	/** Returns true if the vector's scalar components are all greater
	 that the ones of the vector it is compared against.
	 */
	inline bool operator <(const Vector3D& rhs) const {
		if (x < rhs.x && y < rhs.y && z < rhs.z)
			return true;
		return false;
	}

	/** Returns true if the vector's scalar components are all smaller
	 that the ones of the vector it is compared against.
	 */
	inline bool operator>(const Vector3D& rhs) const {
		if (x > rhs.x && y > rhs.y && z > rhs.z)
			return true;
		return false;
	}

	inline bool operator ==(const Vector3D& rhs) const {
		if (x == rhs.x && y == rhs.y && z == rhs.z)
			return true;
		return false;
	}

	Real x, y, z;
};

inline std::ostream& operator<<(std::ostream &os, const Vector3D &val)
{
	os << "( " << val.x << ", " << val.y << ", " << val.z << ") ";
	return os;
};

class Point3D
{
public:
	Real x,y,z;
	Point3D(Real x_, Real y_, Real z_)
	: x(x_),y(y_),z(z_)
	{};
	Point3D()
	: x(0),y(0),z(0)
	{};
	explicit Point3D(Real p[]) //for compatibility with Cmgui
	: x(p[0]),y(p[1]),z(p[2])
	{};
	
	//default cpoy ctor will do
//	Point3D(const Point3D& other)
//	:x(other.x), y(other.y), z(other.z)
//	{};
	
	// Can only add vectors to points
	Point3D operator+(const Vector3D& rhs) const
	{
		return Point3D(x+rhs.x, y+rhs.y, z+rhs.z);
	}
	
	//For convenience
	Point3D operator-(const Vector3D& rhs) const
	{
		return Point3D(x-rhs.x, y-rhs.y, z-rhs.z);
	}
	
	Vector3D operator-(const Point3D& rhs) const 
	{
		return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z);
	}
	
	Point3D operator/(const Real divider) const
	{
			return Point3D(x/divider, y/divider, z/divider);
	}
	
	//default assignment will do
//	Point3D& operator=(const Point3D& rhs) 
//	{
//		if (this != &rhs)
//		{
//			x = rhs.x;
//			y = rhs.y;
//			z = rhs.z;
//		}
//		return *this;
//	}
	
	friend std::ostream& operator<<(std::ostream &_os, const Point3D &val);
};

template <class Type3D>
bool operator==(Type3D const& a, Type3D const& b)
{
	if (a.x == b.x && a.y == b.y && a.z == b.z)
		return true;
	return false;
}

inline Point3D operator*(const gtMatrix& m, const Point3D& v) // includes translation
{
	Point3D r;

	Real fInvW = 1.0 / ( m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] );

	r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] ) * fInvW; // this is 1st row not column
	r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] ) * fInvW;
	r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] ) * fInvW;

	return r;
}

//inline Point3D operator*(Real scalar, const Point3D& rhs)
//{
//	return Point3D(scalar*rhs.x, scalar*rhs.y, scalar*rhs.z);
//}

inline std::ostream& operator<<(std::ostream &os, const Point3D &val)
{
	os << "( " << val.x << ", " << val.y << ", " << val.z << ") ";
	return os;
};

inline std::istream& operator>>(std::istream& in, Point3D &val)
{
	std::string temp;
	in >> temp; // name of the vector
	in >> val.x;
	in >> temp; // trailing character = i
	in >> val.y;
	in >> temp; // trailing character = j
	in >> val.z;
	in >> temp; // trailing character = k
	
	return in; 
}

inline Vector3D operator*(Real scalar, const Vector3D& rhs)
{
	return Vector3D(scalar*rhs.x, scalar*rhs.y, scalar*rhs.z);
}

inline Vector3D operator*(const gtMatrix& m, const Vector3D& v) // no translation
{
	Vector3D r;

	r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z);// + m[0][3] );
	r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z);// + m[1][3] );
	r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);// + m[2][3] );

	return r;
}

inline std::ostream& operator<<(std::ostream &os, const gtMatrix& m)
{
	os << "( " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << ") " << std::endl;
	os << "( " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << ") " << std::endl;
	os << "( " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << ") " << std::endl;
	os << "( " << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << ") " << std::endl;
	return os;
};


inline void inverseMatrix(const gtMatrix m, gtMatrix mInv)
{
	float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3]; 
	float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
	float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	float v0 = m20 * m31 - m21 * m30;
	float v1 = m20 * m32 - m22 * m30;
	float v2 = m20 * m33 - m23 * m30;
	float v3 = m21 * m32 - m22 * m31;
	float v4 = m21 * m33 - m23 * m31;
	float v5 = m22 * m33 - m23 * m32;

	float t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
	float t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
	float t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
	float t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

	float invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	float d00 = t00 * invDet;
	float d10 = t10 * invDet;
	float d20 = t20 * invDet;
	float d30 = t30 * invDet;

	float d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	float d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	float d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	float d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	float d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	float d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	float d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	float d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	float d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	float d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	float d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	float d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	mInv[0][0] = d00, mInv[0][1] = d01, mInv[0][2] = d02, mInv[0][3] = d03;
	mInv[1][0] = d10, mInv[1][1] = d11, mInv[1][2] = d12, mInv[1][3] = d13;
	mInv[2][0] = d20, mInv[2][1] = d21, mInv[2][2] = d22, mInv[2][3] = d23;
	mInv[3][0] = d30, mInv[3][1] = d31, mInv[3][2] = d32, mInv[3][3] = d33;
}

inline void transposeMatrix(gtMatrix m)
{
	for (int i = 0;i<4;i++)
	{
		for (int j = i+1; j<4;j++ )
		{
			float temp = m[i][j];
			m[i][j] = m[j][i];
			m[j][i] = temp;
		}
	}
}

//template <typename T>
//inline void FIND_VECTOR(T& ans,const T& from,const T& to)
//{
//	ans.x = to.x - from.x;
//	ans.y = to.y - from.y;
//	ans.z = to.z - from.z;
//}

template <typename T>
inline double DotProduct(const T& a, const T& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

//template <typename V>
//inline void CrossProduct(V& ans, const V& vec1, const V& vec2)
//{
//	(ans).x = (vec1).y*(vec2).z - (vec1).z*(vec2).y; 
//	(ans).y = (vec1).z*(vec2).x - (vec1).x*(vec2).z; 
//	(ans).z = (vec1).x*(vec2).y - (vec1).y*(vec2).x;
//}

template <typename V>
inline V CrossProduct(const V& vec1, const V& vec2)
{
	return V((vec1).y*(vec2).z - (vec1).z*(vec2).y,
				    (vec1).z*(vec2).x - (vec1).x*(vec2).z,
				    (vec1).x*(vec2).y - (vec1).y*(vec2).x);
}

//template <typename V>
//inline void Normalise(V& vec) 
//{ 
//	double length = sqrt(DotProduct((vec), (vec)));
//	(vec).x = (vec).x / length;
//	(vec).y = (vec).y / length;
//	(vec).z = (vec).z / length;
//}

//inline
//double ComputeVolumeOfTetrahedron(double ax, double ay, double az, double bx, double by, double bz,
//          double cx, double cy, double cz, double dx, double dy, double dz)
//
//{
//	double bxdx, bydy, bzdz, cxdx, cydy, czdz;
//	double vol=0.0;
//
//	bxdx=bx-dx;
//	bydy=by-dy;
//	bzdz=bz-dz;
//	cxdx=cx-dx;
//	cydy=cy-dy;
//	czdz=cz-dz;
//	vol = (az-dz)*(bxdx*cydy-bydy*cxdx) +
//		(ay-dy)*(bzdz*cxdx-bxdx*czdz) +
//		(ax-dx)*(bydy*czdz-bzdz*cydy);
//	return fabs(vol);
//}

template <typename V>
inline
double ComputeVolumeOfTetrahedron(const V& a, const V& b, const V& c, const V& d)

{
	double vol = DotProduct((a - d), CrossProduct((b - d), (c - d)));
	return fabs(vol);
}

template <typename V>
inline
V& operator*=(V& v, double number)
{
	v.x *= number;
	v.y *= number;
	v.z *= number;
	return v;
}

template <typename V>
inline
V& operator/=(V& v, double number)
{
	v.x /= number;
	v.y /= number;
	v.z /= number;
	return v;
}

template <typename V>
inline
V& operator+=(V& v, const V& rhs)
{
	v.x += rhs.x;
	v.y += rhs.y;
	v.z += rhs.z;
	return v;
}

struct Plane
{
	Vector3D normal;
	Point3D position;
};

inline
double SolveASinXPlusBCosXIsEqualToC(double a, double b, double c)
{
	// This solves a sin(x) + b cos(x) = c for 0 < x < 180;
	
	// use the fact the eqn is equivalent to
	// C cos(x - y) = c
	// where
	double C = std::sqrt(a*a + b*b);
//	std::cout << "C = " << C << std::endl;
	
	double y = std::atan(a/b);
//	std::cout << "y = " << y << std::endl;
	
	double xMinusY = std::acos(c / C);
//	std::cout << "xMinusY = " << xMinusY << std::endl;
	
	double x = xMinusY + y;
	while (x < 0)
	{
		x += 2.0 * M_PI;
	}
	while (x > 2.0 * M_PI)
	{
		x -= 2.0 * M_PI;
	}
	
	if ( x < M_PI )
	{
		return x;
	}

	// try the other solution
	x = (-xMinusY) + y;

	while (x < 0)
	{
		x += 2.0 * M_PI;
	}
	while (x > 2.0 * M_PI)
	{
		x -= 2.0 * M_PI;
	}
	return x;
}

class ImagePlane
{
// This class defines a rectangular object in 3D space
	
public:
	Point3D tlc;            /**< top left corner */
	Point3D trc;            /**< top right corner */
	Point3D blc;            /**< bottom left corner */
	Point3D brc;            /**< bottom right corner */
	Vector3D normal;         /**< normal of the plane */
	
	Vector3D xside;          /**< vector from blc to brc */
	Vector3D yside;          /**< vector from blc to tlc */
	
//	int            imageSize;      /**< also stored in StudyInfo - image square */
//	double          fieldOfView;    /**< should match length of xside */
//	double          sliceThickness; /**< may vary between series */
//	double          sliceGap;       /**< non-zero for short axis only */
//	double          pixelSizeX_;     /**< in mm, should be square */
//	double          pixelSizeY_;     /**< in mm, should be square */
	double d; // the constant of the plane equation ax + by + cz = d (a,b & c are the 3 components of this->normal)
};

} // end namespace cap
#endif /* CAPMATH_H_ */
