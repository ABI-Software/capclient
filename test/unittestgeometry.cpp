#include <gtest/gtest.h>
#include "math/geometry.h"

TEST(Geometry, FitPlaneUsingTLS)
{
	using namespace cap;

	Point3D points[] =
	{
		Point3D(-2,-1,-1), Point3D(-2,-1, 1),
		Point3D(-1, 3,-1), Point3D(-1, 3, 1),
		Point3D( 3,-2,-1), Point3D( 3,-2, 1)
	};

	size_t n = sizeof(points)/ sizeof(Point3D);
//	Point3D* ptr = points;
	std::vector<Point3D> v(points, points+n);

	Plane p = FitPlaneUsingTLS(v);
//	std::cout << p.normal << ", " << p.position << "\n";

	EXPECT_DOUBLE_EQ(p.position.x, 0.0);
	EXPECT_DOUBLE_EQ(p.position.y, 0.0);
	EXPECT_DOUBLE_EQ(p.position.z, 0.0);
	EXPECT_NEAR(p.normal.x, 0.0, 0.0000001);
	EXPECT_NEAR(p.normal.y, 0.0, 0.0000001);
	EXPECT_NEAR(p.normal.z, 1.0, 0.0000001);
}

TEST(Geometry, FitPlaneUsingTLS_2)
{
	using namespace cap;

	Point3D points[] =
	{
		Point3D( 2, 4,-10), Point3D( 2, 4, 100),
		Point3D( 1,-1,-10), Point3D( 1,-1, 100),
		Point3D(-4,-2,-10), Point3D(-4,-2, 100)
	};

	size_t n = sizeof(points)/ sizeof(Point3D);
	//Point3D* ptr = points;
	std::vector<Point3D> v(points, points+n);

	Plane p = FitPlaneUsingTLS(v);
//	std::cout << p.normal << ", " << p.position << "\n";

	EXPECT_DOUBLE_EQ(p.position.x, -1.0/3.0);
	EXPECT_DOUBLE_EQ(p.position.y, 1.0/3.0);
	EXPECT_DOUBLE_EQ(p.position.z, 270/6.0);
	EXPECT_NEAR(p.normal.x/p.normal.y , -1.0, 0.0000001);
	EXPECT_NEAR(p.normal.z, 0.0, 0.0000001);
}

//TEST(TotalLeastSquares, SVDTest)
//{
//	PerformSVDTest();
//}

TEST(Geometry, SVDTest_2)
{
	vnl_matrix<double> M(6,3);
	M(0,0) = -2; M(0,1) = -1; M(0,2) = -10;
	M(1,0) = -1; M(1,1) =  3; M(1,2) = -10;
	M(2,0) =  3; M(2,1) = -2; M(2,2) = -10;
	M(3,0) = -2; M(3,1) = -1; M(3,2) =  10;
	M(4,0) = -1; M(4,1) =  3; M(4,2) =  10;
	M(5,0) =  3; M(5,1) = -2; M(5,2) =  10;
	vnl_svd<double> decomp(M);
//	std::cout << decomp.W() << std::endl;
	int index = decomp.rank() - 1;
//	std::cout << "Smallest singular value = " << decomp.W(index) << std::endl;
//	std::cout << "Normal Vector = " << decomp.V().get_column(index) << std::endl;

	vnl_vector<double> v(decomp.V().get_column(index));
	EXPECT_DOUBLE_EQ(v(0) / v(1), 1.0);
	EXPECT_DOUBLE_EQ(v(2), 0);
}

TEST(Geometry, SVDTest_3)
{
	vnl_matrix<double> M(5,3);
	M(0,0) =  2; M(0,1) = -2; M(0,2) =  0;
	M(1,0) = -1; M(1,1) =  0; M(1,2) =  1;
	M(2,0) =  0; M(2,1) =  0; M(2,2) =  0;
	M(3,0) =  1; M(3,1) =  1; M(3,2) = -2;
	M(4,0) =  2; M(4,1) =  2; M(4,2) = -4;

	vnl_svd<double> decomp(M);
//	std::cout << decomp.W() << std::endl;
	int index = decomp.rank() - 1;
//	std::cout << "Smallest singular value = " << decomp.W(index) << std::endl;
//	std::cout << "Normal Vector = " << decomp.V().get_column(index) << std::endl;
	vnl_vector<double> v(decomp.V().get_column(index));
	EXPECT_DOUBLE_EQ(v(0), v(1));
	EXPECT_DOUBLE_EQ(v(2), v(1));
}

TEST(Geometry, ComputeCentroid)
{
	using namespace cap;

	Point3D points[] =
	{
		Point3D( 2, 1, 0),
		Point3D(-1, 3, 1),
		Point3D( 3,-2, 0)
	};

	size_t n = sizeof(points)/ sizeof(Point3D);
	//Point3D* ptr = points;
	std::vector<Point3D> v(points, points+n);

	Point3D c = ComputeCentroid(v);
	EXPECT_DOUBLE_EQ(c.x, 4.0/3.0);
	EXPECT_DOUBLE_EQ(c.y, (1+3-2)/3.0);
	EXPECT_DOUBLE_EQ(c.z, 1.0/3.0);
}

TEST(Geometry, FitPlaneUsingTLS_Illdefined)
{
	cap::Point3D points[] =
	{
		cap::Point3D( 43.875263893798689, -31.327755841858153,  49.4522910562259600),
		cap::Point3D( 27.780713203789222, -19.947668699134610,   8.7570269946642281),
		cap::Point3D( 30.213103685565645, -22.394406862998537,   5.5534712125380929),
		cap::Point3D( 48.648122315972287, -34.992845384900519,   57.783933663597807)
	};

	size_t n = sizeof(points)/ sizeof(cap::Point3D);
	//Point3D* ptr = points;
	std::vector<cap::Point3D> v(points, points+n);

	cap::Plane p = FitPlaneUsingTLS(v);
	EXPECT_NE(0.61218, p.normal.x);
	EXPECT_NE(-0.593729, p.normal.y);
	EXPECT_NE(-0.522227, p.normal.z);
}
