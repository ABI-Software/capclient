#include <gtest/gtest.h>
#include "../src/CAPMath.h"

TEST(Vector3DTest, Creation)
{
	using cap::Vector3D;
	
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
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	EXPECT_FLOAT_EQ(vec1.Length(), 3.741657386773941);
	Vector3D vec2(3,2,1);
	EXPECT_FLOAT_EQ(vec1.Length(), vec2.Length());
}

TEST(Vector3DTest, Nomalize)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	vec1.Normalise();
	EXPECT_FLOAT_EQ(vec1.Length(), 1);
	EXPECT_FLOAT_EQ(vec1.x, 0.267261241912424);
	EXPECT_FLOAT_EQ(vec1.y, 0.534522483824849);
	EXPECT_FLOAT_EQ(vec1.z, 0.8017837257372730);
}

TEST(Vector3DTest, Equality)
{
	using cap::Vector3D;
	
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
	using cap::Vector3D;
	
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
	
	Vector3D vec4 = CrossProduct(vec1,Vector3D(3,2,1));
	EXPECT_FLOAT_EQ(vec4.x, -4);
	EXPECT_FLOAT_EQ(vec4.y, 8);
	EXPECT_FLOAT_EQ(vec4.z, -4);
}

TEST(Vector3DTest, Addition)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	Vector3D vec2(0,9,8);
	EXPECT_EQ(Vector3D(1,11,11), vec1+vec2);
}

TEST(Vector3DTest, Subtraction)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	Vector3D vec2(0,9,8);
	EXPECT_EQ(Vector3D(1,-7,-5), vec1-vec2);
}

TEST(Vector3DTest, Point3D)
{
	using cap::Point3D;
	using cap::Vector3D;
	
	Point3D point1(2,4,5);
	Point3D point2(1,2,2);
	Vector3D vec1(1,2,3);
	EXPECT_EQ(point1-point2, vec1);
	Point3D point3 = point2 + vec1;
	EXPECT_FLOAT_EQ(point3.x, point1.x);
	EXPECT_FLOAT_EQ(point3.y, point1.y);
	EXPECT_FLOAT_EQ(point3.z, point1.z);
}

TEST(ComputeVolumeOfTetrahedronTest, Volume)
{
	using namespace cap;
	
	// computes 6 * the actual volume (to save some computation)
	//EXPECT_FLOAT_EQ(ComputeVolumeOfTetrahedron(0.0f,0.0f,0.0f,
//			1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f), 1);
	
	Point3D a(0,0,0);
	Point3D b(1,0,0);
	Point3D c(0,1,0);
	Point3D d(0,0,1);
	
	EXPECT_FLOAT_EQ(ComputeVolumeOfTetrahedron(a,b,c,d), 1);
	
	d.z = -2.0f;
	EXPECT_FLOAT_EQ(ComputeVolumeOfTetrahedron(a,b,c,d), 2);
	
	d.z = -2.7f;
	EXPECT_FLOAT_EQ(ComputeVolumeOfTetrahedron(a,b,c,d), 2.7);
}

TEST(MathFunctionTest, SolveASinXPlusBCosXIsEqualToC)
{
	using namespace cap;
	
	double a = 12;
	double b = 5;
	double c = 4;
	
	double x = SolveASinXPlusBCosXIsEqualToC(a,b,c);
	double expected = 139.460 / 180.0 * M_PI;
	EXPECT_NEAR(x, expected, 0.00001);
	
	x = SolveASinXPlusBCosXIsEqualToC(10,10,10);
	expected = M_PI_2;
	EXPECT_NEAR(x, expected, 0.00001);
	
	x = SolveASinXPlusBCosXIsEqualToC(3,4,1.45099728905505);
	expected = 110.0 / 180.0 * M_PI;
	EXPECT_NEAR(x, expected, 0.00001);
}
