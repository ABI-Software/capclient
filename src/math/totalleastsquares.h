/*
 * math/totalleastsquares.h
 *
 *  Created on: Feb 12, 2010
 *      Author: jchu014
 */

#ifndef CAPTOTALLEASTSQUARES_H_
#define CAPTOTALLEASTSQUARES_H_

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.h>
#include <vnl/algo/vnl_svd.h>
//#include <vnl/vnl_sparse_matrix.h>
//#include <vnl/vnl_sparse_matrix_linear_system.h>
//#include <vnl/algo/vnl_lsqr.h>
//#include <vnl/vnl_linear_system.h>

#include "capclientconfig.h"
#include "math/algebra.h"

namespace cap
{

//struct Plane;

//inline Point3D ComputeCentroid(const std::vector<Point3D>& points)
//{
//	Point3D centroid;
//	for (std::vector<Point3D>::const_iterator i = points.begin();
//			i != points.end(); ++i)
//	{
//		centroid += *i;
//	}
//	centroid /= points.size();
//	return centroid;
//}

//Plane FitPlaneUsingTLS(const std::vector<Point3D>& points);

//inline void TestLSQR()
//{
//	std::cout << __func__ << "\n";
//	{
//		vnl_sparse_matrix<double> m(2,2);
//		m(0,0) = 1; m(0,1) = 2;
//		m(1,0) = 3; m(1,1) = 4;

//		vnl_vector<double> b(2);
//		b[0] = 3;
//		b[1] = 7;

//		vnl_sparse_matrix_linear_system<double> ls(m,b);
//		vnl_lsqr lsqr(ls);
//		lsqr.set_max_iterations(100);
//		vnl_vector<double> x(2);
//		lsqr.minimize(x);
//		lsqr.diagnose_outcome(std::cout);
//		std::cout << x << std::endl;
//	}

//	{
//		vnl_sparse_matrix<double> m(3,2);
//		m(0,0) = 1; m(0,1) = 2;
//		m(1,0) = 3; m(1,1) = 4;
//		m(2,0) = 1; m(2,1) = 3;

//		vnl_vector<double> b(3);
//		b[0] = 3;
//		b[1] = 7;
//		b[2] = 3;

//		vnl_sparse_matrix_linear_system<double> ls(m,b);
//		vnl_lsqr lsqr(ls);
//		lsqr.set_max_iterations(100);
//		vnl_vector<double> x(2);
//		lsqr.minimize(x);
//		lsqr.diagnose_outcome(std::cout);
//		std::cout << x << std::endl;
//	}
//}

} // end namespace cap

#endif /* CAPTOTALLEASTSQUARES_H_ */
