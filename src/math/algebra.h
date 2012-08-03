/*
 * CAPMath.h
 *
 *  Created on: Feb 20, 2009
 *      Author: jchu014
 */

#ifndef CAPMATH_H_
#define CAPMATH_H_

#include <iostream>
#include <string>
#include <limits>

#include <cmath>
#ifndef M_PI
# define M_PI	3.14159265358979323846
#endif
#ifndef M_PI_2
# define M_PI_2	1.57079632679489661923
#endif

extern "C"
{
#include <zn/cmgui_configure.h>
}

#include "utils/debug.h"
#include "utils/misc.h"

typedef float gtMatrix[4][4];
#ifdef FE_VALUE_IS_DOUBLE
typedef double Real;
const double determinatTolerance = 1e-10;
#else
typedef float Real;
const double determinatTolerance = 1e-6;
#endif //FE_VALUE_IS_DOUBLE

namespace cap
{

/**
 * Type3D.
 */
class Type3D
{
protected:
    Type3D() : x(0), y(0), z(0) {}
    Type3D(Real x_, Real y_, Real z_) : x(x_), y(y_), z(z_) {}

public:
    /**
     * Returns true if the 3D type's scalar components are all greater
     * than the ones of the 3D type it is compared against.
     */
    inline bool operator <(const Type3D& rhs) const
    {
        if (x < rhs.x && y < rhs.y && z < rhs.z)
            return true;
        return false;
    }

    /**
     * Returns true if the 3D type's scalar components are all less
     * than the ones of the 3D type it is compared against.
     */
    inline bool operator>(const Type3D& rhs) const
    {
        if (x > rhs.x && y > rhs.y && z > rhs.z)
            return true;
        return false;
    }

    /**
     * Returns true if the 3D type's scalar components are all equal
     * to the ones of the 3D type it is compared against.
     */
    inline bool operator ==(const Type3D& rhs) const
    {
        if (x == rhs.x && y == rhs.y && z == rhs.z)
            return true;
        return false;
    }

    friend std::ostream& operator<<(std::ostream &_os, const Type3D &val);

    Real x, y, z;
};

inline std::ostream& operator<<(std::ostream &os, const Type3D &val)
{
    os << "( " << val.x << ", " << val.y << ", " << val.z << ") ";
    return os;
}

/**
 * Vector3D.
 */
class Vector3D : public Type3D
{
public:
    Vector3D()
        : Type3D()
    {
    }

    Vector3D(Real x_, Real y_, Real z_)
        : Type3D(x_, y_, z_)
    {
    }

    Real Length() const {
        return sqrt(x * x + y * y + z * z);
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

    Real operator()(const int r) const
    {
        if (r == 1)
            return x;
        else if (r == 2)
            return y;
        else if (r == 3)
            return z;

        return std::numeric_limits<Real>::max();
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

    Vector3D& operator*=(double number)
    {
        x *= number;
        y *= number;
        z *= number;
        return *this;
    }

};

/**
 * Point3D.
 */
class Point3D : public Type3D
{
public:
    Point3D() : Type3D() {}
    Point3D(Real x_, Real y_, Real z_) : Type3D(x_, y_, z_) {}
    Point3D(const Vector3D& vec) : Type3D(vec.x, vec.y, vec.z) {}

    explicit Point3D(Real p[]) //for compatibility with Cmgui
        : Type3D(p[0], p[1], p[2]) {}

    /**
     * Convert the point to an array.  The array returned
     * must be deleted by the receiver.
     *
     * \returns a pointer to an array of 3 Reals.
     */
    Real* ToArray()
    {
        Real *array = new Real[3];
        array[0] = x;
        array[1] = y;
        array[2] = z;

        return array;
    }

    Vector3D ToVector3D() const
    {
        return Vector3D(x, y, z);
    }

    /**
     * Addition operator.
     *
     * @param	rhs	The right hand side.
     *
     * @return	The result of the operation.
     */
    Point3D operator+(const Vector3D& rhs) const
    {
        return Point3D(x+rhs.x, y+rhs.y, z+rhs.z);
    }

    Point3D operator+(const Point3D& rhs) const
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

    Point3D& operator*=(double number)
    {
        x *= number;
        y *= number;
        z *= number;
        return *this;
    }

    Point3D& operator/=(double number)
    {
        x /= number;
        y /= number;
        z /= number;
        return *this;
    }

    Point3D& operator+=(double number)
    {
        x += number;
        y += number;
        z += number;
        return *this;
    }

    Point3D& operator+=(const Point3D& oth)
    {
        x += oth.x;
        y += oth.y;
        z += oth.z;
        return *this;
    }
};

class Matrix3x3
{
//    Vector3D col1, col2, col3;
    Real m[9];

public:
    Matrix3x3() {Identity();}
    Matrix3x3(const Real xx, const Real yx, const Real zx,
              const Real xy, const Real yy, const Real zy,
              const Real xz, const Real yz, const Real zz)
//        : col1(xx, xy, xz)
//        , col2(yx, yy, yz)
//        , col3(zx, zy, zz)
    {
        m[0] = xx, m[1] = yx, m[2] = zx;
        m[3] = xy, m[4] = yy, m[5] = zy;
        m[6] = xz, m[7] = yz, m[8] = zz;
    }

    Matrix3x3(const Vector3D& column1, const Vector3D& column2, const Vector3D& column3)
    {
        m[0] = column1.x, m[3] = column1.y, m[6] = column1.z;
        m[1] = column2.x, m[4] = column2.y, m[7] = column2.z;
        m[2] = column3.x, m[5] = column3.y, m[8] = column3.z;
    }

    Matrix3x3& Identity()
    {
        //Matrix3x3 mx(Vector3D(1.0, 0.0, 0.0), Vector3D(0.0, 1.0, 0.0), Vector3D(0.0, 0.0, 1.0));
        m[0] = m[4] = m[8] = 1.0;
        m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0.0;
        return *this;
    }

    Matrix3x3& Invert()
    {
        Real tmp[9];
        tmp[0] = m[4] * m[8] - m[5] * m[7];
        tmp[1] = m[2] * m[7] - m[1] * m[8];
        tmp[2] = m[1] * m[5] - m[2] * m[4];
        tmp[3] = m[5] * m[6] - m[3] * m[8];
        tmp[4] = m[0] * m[8] - m[2] * m[6];
        tmp[5] = m[2] * m[3] - m[0] * m[5];
        tmp[6] = m[3] * m[7] - m[4] * m[6];
        tmp[7] = m[1] * m[6] - m[0] * m[7];
        tmp[8] = m[0] * m[4] - m[1] * m[3];

        Real determinant = m[0] * tmp[0] + m[1] * tmp[3] + m[2] * tmp[6];
        if (fabs(determinant) <= determinatTolerance)
        {
            return Identity();
        }

        Real invDeterminant = 1.0 / determinant;
        m[0] = invDeterminant * tmp[0];
        m[1] = invDeterminant * tmp[1];
        m[2] = invDeterminant * tmp[2];
        m[3] = invDeterminant * tmp[3];
        m[4] = invDeterminant * tmp[4];
        m[5] = invDeterminant * tmp[5];
        m[6] = invDeterminant * tmp[6];
        m[7] = invDeterminant * tmp[7];
        m[8] = invDeterminant * tmp[8];

        return *this;
    }

    void Transpose()
    {
        std::swap(m[1], m[3]);
        std::swap(m[2], m[6]);
        std::swap(m[5], m[7]);
//        Real tmp = col1.y; col1.y = col2.x; col2.x = tmp;
//        tmp  = col1.z; col1.z = col3.x; col3.x = tmp;
//        tmp = col2.z; col2.z = col3.y; col3.y = tmp;
    }

    Real operator()(const int r, const int c) const
    {
        if (0 < r && r < 4 && 0 < c && c < 4)
            return m[(r-1)*3+c-1];
//        if (c == 1)
//            return col1(r);
//        else if (c == 2)
//            return col2(r);
//        else if (c == 3)
//            return col3(r);

        return std::numeric_limits<Real>::max();
    }

    Real& operator[](const int index)
    {
        return m[index];
    }

    Real operator[](const int index) const
    {
        return m[index];
    }

    Matrix3x3 operator* (const Real d) const
    {
        Matrix3x3 mx(m[0]*d, m[1]*d, m[2]*d,
                     m[3]*d, m[4]*d, m[5]*d,
                     m[6]*d, m[7]*d, m[8]*d);
        return mx;
    }

    Vector3D operator* (const Vector3D& vc) const
    {
        Real r1 = m[0]*vc.x + m[1]*vc.y + m[2]*vc.z;
        Real r2 = m[3]*vc.x + m[4]*vc.y + m[5]*vc.z;
        Real r3 = m[6]*vc.x + m[7]*vc.y + m[8]*vc.z;
        return Vector3D(r1, r2, r3);
    }

    friend Matrix3x3 operator* (const Real d, const Matrix3x3& mx)
    {
        return mx*d;
    }

    friend Vector3D operator* (const Vector3D vc, const Matrix3x3& mx)
    {
//        Real r1 = mx.col1*vc;
//        Real r2 = mx.col2*vc;
//        Real r3 = mx.col3*vc;
        Real r1 = mx[0]*vc.x + mx[3]*vc.y + mx[6]*vc.z;
        Real r2 = mx[1]*vc.x + mx[4]*vc.y + mx[7]*vc.z;
        Real r3 = mx[2]*vc.x + mx[5]*vc.y + mx[8]*vc.z;
        return Vector3D(r1, r2, r3);
    }

    friend std::ostream& operator<<(std::ostream &os, const Matrix3x3 &val)
    {
        os << "[ " << val[0] << ", " << val[1] << ", " << val[2] << "] ";
        os << "[ " << val[3] << ", " << val[4] << ", " << val[5] << "] ";
        os << "[ " << val[6] << ", " << val[7] << ", " << val[8] << "] ";
        return os;
    }
};

class HomogeneousVector3D
{
public:
    Real x, y, z, w;
    HomogeneousVector3D()
        : x(0.0)
        , y(0.0)
        , z(0.0)
        , w(1.0)
    {}

    HomogeneousVector3D(Real x, Real y, Real z, Real w)
        : x(x)
        , y(y)
        , z(z)
        , w(w)
    {}

    HomogeneousVector3D(const cap::Vector3D& v)
        : x(v.x)
        , y(v.y)
        , z(v.z)
        , w(1.0)
    {}

    Point3D ToPoint3D()
    {
        return Point3D(x/w, y/w, z/w);
    }

    Real operator()(const int r)
    {
        if (r ==1)
            return x;
        else if (r == 2)
            return y;
        else if (r == 3)
            return z;
        else if (r == 4)
            return w;

        return std::numeric_limits<Real>::max();
    }

};

class Matrix4x4
{
public:
    Real m[16];

    Matrix4x4(){Identity();}

    Matrix4x4(Real xx, Real yx, Real zx, Real wx,
              Real xy, Real yy, Real zy, Real wy,
              Real xz, Real yz, Real zz, Real wz,
              Real xw, Real yw, Real zw, Real ww)
    {
        m[0] = xx,  m[1] = yx,  m[2] = zx,  m[3] = wx;
        m[4] = xy,  m[5] = yy,  m[6] = zy,  m[7] = wy;
        m[8] = xz,  m[9] = yz,  m[10] = zz, m[11] = wz;
        m[12] = xw, m[13] = yw, m[14] = zw, m[15] = ww;
    }

    Real& operator[](const int index)
    {
        return m[index];
    }

    Real operator[](const int index) const
    {
        return m[index];
    }

    Real operator()(const int r, const int c) const
    {
        if (0 < r && r < 5 && 0 < c && c < 5)
            return m[(r-1)*4+c-1];

        return std::numeric_limits<Real>::max();
    }

    HomogeneousVector3D operator *(const HomogeneousVector3D& h) const
    {
        HomogeneousVector3D r;
        r.x = m[0]*h.x+m[1]*h.y+m[2]*h.z+m[3]*h.w;
        r.y = m[4]*h.x+m[5]*h.y+m[6]*h.z+m[7]*h.w;
        r.z = m[8]*h.x+m[9]*h.y+m[10]*h.z+m[11]*h.w;
        r.w = m[12]*h.x+m[13]*h.y+m[14]*h.z+m[15]*h.w;
        return r;
    }

    Matrix4x4 operator* (const Matrix4x4& n) const
    {
        Matrix4x4 res(m[0]*n[0]  + m[1]*n[4]  + m[2]*n[8]  + m[3]*n[12],   m[0]*n[1]  + m[1]*n[5]  + m[2]*n[9]  + m[3]*n[13],   m[0]*n[2]  + m[1]*n[6]  + m[2]*n[10]  + m[3]*n[14],   m[0]*n[3]  + m[1]*n[7]  + m[2]*n[11]  + m[3]*n[15],
                      m[4]*n[0]  + m[5]*n[4]  + m[6]*n[8]  + m[7]*n[12],   m[4]*n[1]  + m[5]*n[5]  + m[6]*n[9]  + m[7]*n[13],   m[4]*n[2]  + m[5]*n[6]  + m[6]*n[10]  + m[7]*n[14],   m[4]*n[3]  + m[5]*n[7]  + m[6]*n[11]  + m[7]*n[15],
                      m[8]*n[0]  + m[9]*n[4]  + m[10]*n[8] + m[11]*n[12],  m[8]*n[1]  + m[9]*n[5]  + m[10]*n[9] + m[11]*n[13],  m[8]*n[2]  + m[9]*n[6]  + m[10]*n[10] + m[11]*n[14],  m[8]*n[3]  + m[9]*n[7]  + m[10]*n[11] + m[11]*n[15],
                      m[12]*n[0] + m[13]*n[4] + m[14]*n[8] + m[15]*n[12],  m[12]*n[1] + m[13]*n[5] + m[14]*n[9] + m[15]*n[13],  m[12]*n[2] + m[13]*n[6] + m[14]*n[10] + m[15]*n[14],  m[12]*n[3] + m[13]*n[7] + m[14]*n[11] + m[15]*n[15]);

        return res;
    }

    Matrix4x4& Identity()
    {
        m[0] = m[5] = m[10] = m[15] = 1.0;
        m[1] = m[2] = m[3] = m[4] = 0.0;
        m[6] = m[7] = m[8] = m[9] = 0.0;
        m[11] = m[12] = m[13] = m[14] = 0.0;

        return *this;
    }

    void Transpose()
    {
        std::swap(m[1], m[4]);
        std::swap(m[2], m[8]);
        std::swap(m[3], m[12]);
        std::swap(m[6], m[9]);
        std::swap(m[7], m[13]);
        std::swap(m[11], m[14]);
    }

    void InvertEuclidean()
    {
        cap::Matrix3x3 rot(m[0], m[1], m[2],
                           m[4], m[5], m[6],
                           m[8], m[9], m[10]);
        rot.Transpose();
        SetRotation(rot);
        cap::Matrix3x3 mrot = -1.0*rot;
        cap::Vector3D tr(m[3], m[7], m[11]);
        cap::Vector3D mrottr = mrot * tr;
        SetTranslation(mrottr);
    }

    void InvertAffine()
    {
        cap::Matrix3x3 rot(m[0], m[1], m[2],
                           m[4], m[5], m[6],
                           m[8], m[9], m[10]);
        rot.Invert();
        SetRotation(rot);
        cap::Matrix3x3 mrot = -1.0*rot;
        cap::Vector3D tr(m[3], m[7], m[11]);
        cap::Vector3D mrottr = mrot * tr;
        SetTranslation(mrottr);
    }

    void SetRotation(const cap::Matrix3x3& rot)
    {
        m[0] = rot(1,1), m[1] = rot(1,2), m[2] = rot(1,3);
        m[4] = rot(2,1), m[5] = rot(2,2), m[6] = rot(2,3);
        m[8] = rot(3,1), m[9] = rot(3,2), m[10] = rot(3,3);
    }

    void SetTranslation(const cap::Vector3D& vc)
    {
        m[3] = vc(1);
        m[7] = vc(2);
        m[11] = vc(3);
    }

    friend std::ostream& operator<<(std::ostream &os, const Matrix4x4 &val)
    {
        os << "[ " << val[0] << ", " << val[1] << ", " << val[2] << ", " << val[3] << "] ";
        os << "[ " << val[4] << ", " << val[5] << ", " << val[6] << ", " << val[7] << "] ";
        os << "[ " << val[8] << ", " << val[9] << ", " << val[10] << ", " << val[11] << "] ";
        os << "[ " << val[12] << ", " << val[13] << ", " << val[14] << ", " << val[15] << "] ";
        return os;
    }
};

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

//--template <typename T>
inline double DotProduct(const Type3D& a, const Type3D& b)
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

//--template <typename V>
inline Vector3D CrossProduct(const Type3D& vec1, const Type3D& vec2)
{
    return Vector3D((vec1).y*(vec2).z - (vec1).z*(vec2).y,
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

//template <typename V>
inline
double ComputeVolumeOfTetrahedron(const Point3D& a, const Point3D& b, const Point3D& c, const Point3D& d)

{
    double vol = DotProduct((a - d), CrossProduct((b - d), (c - d)));
    return fabs(vol);
}

//template <typename V>
//inline
//V& operator*=(V& v, double number)
//{
//	v.x *= number;
//	v.y *= number;
//	v.z *= number;
//	return v;
//}

//template <typename V>
//inline
//V& operator/=(V& v, double number)
//{
//	v.x /= number;
//	v.y /= number;
//	v.z /= number;
//	return v;
//}

//template <typename V>
//inline
//V& operator+=(V& v, const V& rhs)
//{
//	v.x += rhs.x;
//	v.y += rhs.y;
//	v.z += rhs.z;
//	return v;
//}

struct Plane
{
    Vector3D normal;
    Point3D position;
};



/**
 * Solve a*sine(x coordinate) + b*cosine(x coordinate) = c.  For 0 < x < 180.
 *
 * @param	a	a.
 * @param	b	The b.
 * @param	c	The c.
 *
 * @return	.
 */
inline double SolveASinXPlusBCosXIsEqualToC(double a, double b, double c)
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

/**
 * Image plane.  This class defines a rectangular object in 3D space
 */
class ImagePlane
{
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
