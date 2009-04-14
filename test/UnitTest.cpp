#include <gtest/gtest.h>
#include "../src/CAPMath.h"

TEST(Vector3DTest, Creation)
{
	Vector3D vec1(1,2,3);
	EXPECT_FLOAT_EQ(vec1.x,1);
	EXPECT_FLOAT_EQ(vec1.y,2);
	EXPECT_FLOAT_EQ(vec1.z,3);
	Vector3D vec2;
	EXPECT_FLOAT_EQ(vec2.x,0);
	EXPECT_FLOAT_EQ(vec2.y,0);
	EXPECT_FLOAT_EQ(vec2.z,0);
}

TEST(Vector3DTest, Length)
{
	Vector3D vec1(1,2,3);
	EXPECT_FLOAT_EQ(vec1.Length(), 3.741657386773941);
	Vector3D vec2(3,2,1);
	EXPECT_FLOAT_EQ(vec1.Length(), vec2.Length());
}

TEST(Vector3DTest, Nomalize)
{
	Vector3D vec1(1,2,3);
	vec1.Normalise();
	EXPECT_FLOAT_EQ(vec1.Length(), 1);
	EXPECT_FLOAT_EQ(vec1.x, 0.267261241912424);
	EXPECT_FLOAT_EQ(vec1.y, 0.534522483824849);
	EXPECT_FLOAT_EQ(vec1.z, 0.8017837257372730);
}

TEST(Vector3DTest, Equality)
{
	Vector3D vec1(1,2,3);
	Vector3D vec2(1,2,3);
	EXPECT_EQ(vec1, vec2);
	Vector3D vec3(2,2,3);
	EXPECT_FALSE(vec1 == vec3);
	Vector3D vec4(2,3,4);
	EXPECT_TRUE(vec1 < vec4);
	EXPECT_TRUE(vec4 > vec1);
	EXPECT_FALSE(vec1 < vec3);
}

TEST(Vector3DTest, Muliplication)
{
	Vector3D vec1(1,2,3);
	Vector3D vec2(2,4,6);
	EXPECT_EQ(vec1*2, vec2);
	EXPECT_EQ(2*vec1, vec2);
	EXPECT_FLOAT_EQ(vec1*vec1, 14);
	EXPECT_FLOAT_EQ(vec1*vec2, 28);
	Vector3D vec3;
	vec3.CrossProduct(vec1,vec2);
	EXPECT_FLOAT_EQ(vec3.x, 0);
	EXPECT_FLOAT_EQ(vec3.y, 0);
	EXPECT_FLOAT_EQ(vec3.z, 0);
	vec3.CrossProduct(vec1,Vector3D(3,2,1));
	EXPECT_FLOAT_EQ(vec3.x, -4);
	EXPECT_FLOAT_EQ(vec3.y, 8);
	EXPECT_FLOAT_EQ(vec3.z, -4);
}

TEST(Vector3DTest, Addition)
{
	Vector3D vec1(1,2,3);
	Vector3D vec2(0,9,8);
	EXPECT_EQ(Vector3D(1,11,11), vec1+vec2);
}

TEST(Vector3DTest, Subtraction)
{
	Vector3D vec1(1,2,3);
	Vector3D vec2(0,9,8);
	EXPECT_EQ(Vector3D(1,-7,-5), vec1-vec2);
}

TEST(Vector3DTest, Point3D)
{
	Point3D point1(2,4,5);
	Point3D point2(1,2,2);
	Vector3D vec1(1,2,3);
	EXPECT_EQ(point1-point2, vec1);
	Point3D point3 = point2 + vec1;
	EXPECT_FLOAT_EQ(point3.x, point1.x);
	EXPECT_FLOAT_EQ(point3.y, point1.y);
	EXPECT_FLOAT_EQ(point3.z, point1.z);
}

