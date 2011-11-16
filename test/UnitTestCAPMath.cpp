#include <gtest/gtest.h>
#include "CAPMath.h"

TEST(Vector3DTest, Creation)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	EXPECT_NEAR(1, vec1.x, 1e-08);
	EXPECT_NEAR(2, vec1.y, 1e-08);
	EXPECT_NEAR(3, vec1.z, 1e-08);
	Vector3D vec2;
	EXPECT_NEAR(0, vec2.x, 1e-08);
	EXPECT_NEAR(0, vec2.y, 1e-08);
	EXPECT_NEAR(0, vec2.z, 1e-08);
}

TEST(Vector3DTest, Length)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	EXPECT_NEAR(3.741657386773941, vec1.Length(), 1e-08);
	Vector3D vec2(3,2,1);
	EXPECT_NEAR(vec1.Length(), vec2.Length(), 1e-08);
}

TEST(Vector3DTest, Nomalize)
{
	using cap::Vector3D;
	
	Vector3D vec1(1,2,3);
	vec1.Normalise();
	EXPECT_NEAR(1, vec1.Length(), 1e-8);
	EXPECT_NEAR(0.267261241912424, vec1.x, 1e-8);
	EXPECT_NEAR(0.534522483824849, vec1.y, 1e-8);
	EXPECT_NEAR(0.8017837257372730, vec1.z, 1e-8);
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
	EXPECT_NEAR(14, vec1*vec1, 1e-08);
	EXPECT_NEAR(28, vec1*vec2, 1e-08);
	Vector3D vec3;
	vec3.CrossProduct(vec1,vec2);
	EXPECT_NEAR(0, vec3.x, 1e-08);
	EXPECT_NEAR(0, vec3.y, 1e-08);
	EXPECT_NEAR(0, vec3.z, 1e-08);
	vec3.CrossProduct(vec1,Vector3D(3,2,1));
	EXPECT_NEAR(-4, vec3.x, 1e-08);
	EXPECT_NEAR(8, vec3.y, 1e-08);
	EXPECT_NEAR(-4, vec3.z, 1e-08);
	
	Vector3D vec4 = CrossProduct(vec1,Vector3D(3,2,1));
	EXPECT_NEAR(-4, vec4.x, 1e-08);
	EXPECT_NEAR(8, vec4.y, 1e-08);
	EXPECT_NEAR(-4, vec4.z, 1e-08);
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
	EXPECT_NEAR(point3.x, point1.x, 1e-08);
	EXPECT_NEAR(point3.y, point1.y, 1e-08);
	EXPECT_NEAR(point3.z, point1.z, 1e-08);
}

TEST(ComputeVolumeOfTetrahedronTest, Volume)
{
	using namespace cap;
	
	// computes 6 * the actual volume (to save some computation)
	//EXPECT_NEAR(ComputeVolumeOfTetrahedron(0.0f,0.0f,0.0f,
//			1.0f,0.0f,0.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,1.0f), 1);
	
	Point3D a(0,0,0);
	Point3D b(1,0,0);
	Point3D c(0,1,0);
	Point3D d(0,0,1);
	
	EXPECT_NEAR(1, ComputeVolumeOfTetrahedron(a,b,c,d), 1e-07);
	
	d.z = -2.0f;
	EXPECT_NEAR(2, ComputeVolumeOfTetrahedron(a,b,c,d), 1e-07);
	
	d.z = -2.7f;
	EXPECT_NEAR(2.7, ComputeVolumeOfTetrahedron(a,b,c,d), 1e-07);
}

TEST(MathFunctionTest, SolveASinXPlusBCosXIsEqualToC)
{
	using namespace cap;
	
	double a = 12;
	double b = 5;
	double c = 4;
	
	double x = SolveASinXPlusBCosXIsEqualToC(a,b,c);
	double expected = 139.460 / 180.0 * M_PI;
	EXPECT_NEAR(expected, x, 0.00001);
	
	x = SolveASinXPlusBCosXIsEqualToC(10,10,10);
	expected = M_PI_2;
	EXPECT_NEAR(expected, x, 0.00001);
	
	x = SolveASinXPlusBCosXIsEqualToC(3,4,1.45099728905505);
	expected = 110.0 / 180.0 * M_PI;
	EXPECT_NEAR(expected, x, 0.00001);
}
