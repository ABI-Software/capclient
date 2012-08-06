#ifndef CAP_GEOMETRY_H
#define CAP_GEOMETRY_H

#include "math/algebra.h"
//#include "math/totalleastsquares.h"

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_svd.h>

namespace cap
{

/**
 * Geometric description of a plane.
 */
struct Plane
{
	Vector3D normal;
	Point3D position;
};

/**
 * Compute the centroid.
 *
 * @param points	The points to compute the centroid of.
 * @return The centroid.
 */
inline Point3D ComputeCentroid(const std::vector<Point3D>& points)
{
	Point3D centroid;
	for (std::vector<Point3D>::const_iterator i = points.begin();
			i != points.end(); ++i)
	{
		centroid += *i;
	}
	centroid /= points.size();
	return centroid;
}

/**
 * Fit plane using Total Least Squares.  Uses SVD.
 *
 * @param points
 * @return The fitted plane
 */
inline Plane FitPlaneUsingTLS(const std::vector<Point3D>& points)
{
	// 1. Compute centroid
	Point3D centroid = ComputeCentroid(points);

	// 2. Build m*3 matrix from (data points - centroid)
	vnl_matrix<double> M(points.size(), 3);
	for (size_t i = 0; i < points.size(); ++i)
	{
		Vector3D diff = points[i] - centroid;
		M(i,0) = diff.x;
		M(i,1) = diff.y;
		M(i,2) = diff.z;
	}

	// 3. perform svd
	vnl_svd<double> svd(M);

	int index = svd.rank() - 1; // index of smallest non-zero singular value
	vnl_vector<double> v = svd.V().get_column(index); // and the corresponding vector

	Plane plane;
	plane.normal = Vector3D(v(0), v(1), v(2));
	plane.position = centroid;

	return plane;
}

/**
 * Fit plane through the given points.  If necessary (ill-defined plane, two or one point situations)
 * use the surrogate normal to define the plane.
 *
 * @param	points	The points to fit.
 * @param	surrogateNormal	The surrogate normal.
 *
 * @return	The fitted plane.
 */
inline Plane FitPlaneThroughPoints(const std::vector<Point3D>& points, const Vector3D& surrogateNormal)
{
	Plane plane;

	if (points.size() > 2)
	{
		// Total Least Squares using SVD
		plane = FitPlaneUsingTLS(points);
	}
	else if (points.size() == 2)
	{
		// When only 2 base plane points have been specified
		Vector3D temp1 = points[1] - points[0];
		temp1.Normalise();

		Vector3D temp2 = CrossProduct(temp1, surrogateNormal);

		plane.normal = CrossProduct(temp1, temp2);
		plane.normal.Normalise();

		plane.position = points[0] + (0.5 * (points[1] - points[0]));
	}
	else
	{
		// One base plane point
		plane.position = points[0];
		plane.normal = surrogateNormal;
		plane.normal.Normalise();
	}

	// make sure plane normal is always pointing toward the apex
	if (DotProduct(plane.normal, surrogateNormal) < 0)
	{
		plane.normal *= -1;
	}

	return plane;
}

}

#endif // CAP_GEOMETRY_H
