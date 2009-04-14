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

class Vector3D
{
public:
	Vector3D() :
		x(0), y(0), z(0) {
	}

	Vector3D(float x_, float y_, float z_) :
		x(x_), y(y_), z(z_) {
	}

	float Length() const {
		return sqrtf(x * x + y * y + z * z);
	}

	void Normalise() {
		float length = Length();
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

	inline Vector3D operator *(const float fScalar) const {
		return Vector3D(x * fScalar, y * fScalar, z * fScalar);
	}

	inline float operator *(const Vector3D& rhs) const {
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

	float x, y, z;
};

inline std::ostream& operator<<(std::ostream &os, const Vector3D &val)
{
	os << "( " << val.x << ", " << val.y << ", " << val.z << ") ";
	return os;
};

class Point3D
{
public:
	float x,y,z;
	Point3D(float x_, float y_, float z_)
	: x(x_),y(y_),z(z_)
	{};
	Point3D()
	: x(0),y(0),z(0)
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
	
	Vector3D operator-(const Point3D& rhs) const {
		return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z);
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

inline Point3D operator*(const gtMatrix& m, const Point3D& v) // includes translation
{
	Point3D r;

	float fInvW = 1.0 / ( m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] );

	r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] ) * fInvW; // this is 1st row not column
	r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] ) * fInvW;
	r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] ) * fInvW;

	return r;
}

//inline Point3D operator*(float scalar, const Point3D& rhs)
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

inline Vector3D operator*(float scalar, const Vector3D& rhs)
{
	return Vector3D(scalar*rhs.x, scalar*rhs.y, scalar*rhs.z);
}

inline Vector3D operator*(const gtMatrix& m, const Vector3D& v) // no translation
{
	Vector3D r;

	r.x = ( m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] );
	r.y = ( m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] );
	r.z = ( m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] );

	return r;
}

typedef float Real;

inline std::ostream& operator<<(std::ostream &os, const gtMatrix& m)
{
	os << "( " << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << ") " << std::endl;
	os << "( " << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << ") " << std::endl;
	os << "( " << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << ") " << std::endl;
	os << "( " << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << ") " << std::endl;
	return os;
};


inline void inverseMatrix(const float m[4][4], float mInv[4][4])
{
	Real m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3]; 
	Real m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
	Real m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
	Real m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

	Real v0 = m20 * m31 - m21 * m30;
	Real v1 = m20 * m32 - m22 * m30;
	Real v2 = m20 * m33 - m23 * m30;
	Real v3 = m21 * m32 - m22 * m31;
	Real v4 = m21 * m33 - m23 * m31;
	Real v5 = m22 * m33 - m23 * m32;

	Real t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
	Real t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
	Real t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
	Real t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

	Real invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

	Real d00 = t00 * invDet;
	Real d10 = t10 * invDet;
	Real d20 = t20 * invDet;
	Real d30 = t30 * invDet;

	Real d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	Real d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	Real d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	Real d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m10 * m31 - m11 * m30;
	v1 = m10 * m32 - m12 * m30;
	v2 = m10 * m33 - m13 * m30;
	v3 = m11 * m32 - m12 * m31;
	v4 = m11 * m33 - m13 * m31;
	v5 = m12 * m33 - m13 * m32;

	Real d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	Real d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	Real d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	Real d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

	v0 = m21 * m10 - m20 * m11;
	v1 = m22 * m10 - m20 * m12;
	v2 = m23 * m10 - m20 * m13;
	v3 = m22 * m11 - m21 * m12;
	v4 = m23 * m11 - m21 * m13;
	v5 = m23 * m12 - m22 * m13;

	Real d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
	Real d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
	Real d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
	Real d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

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
inline float DotProduct(const T& a, const T& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

template <typename V>
inline void CrossProduct(V& ans, const V& vec1, const V& vec2)
{
	(ans).x = (vec1).y*(vec2).z - (vec1).z*(vec2).y; 
	(ans).y = (vec1).z*(vec2).x - (vec1).x*(vec2).z; 
	(ans).z = (vec1).x*(vec2).y - (vec1).y*(vec2).x;
}

//template <typename V>
//inline void Normalise(V& vec) 
//{ 
//	double length = sqrt(DotProduct((vec), (vec)));
//	(vec).x = (vec).x / length;
//	(vec).y = (vec).y / length;
//	(vec).z = (vec).z / length;
//}

#endif /* CAPMATH_H_ */
