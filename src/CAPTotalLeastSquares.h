/*
 * CAPTotalLeastSquares.h
 *
 *  Created on: Feb 12, 2010
 *      Author: jchu014
 */

#ifndef CAPTOTALLEASTSQUARES_H_
#define CAPTOTALLEASTSQUARES_H_
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_sparse_matrix.h>
#include <vnl/algo/vnl_svd.h>
#include <vnl/vnl_sparse_matrix_linear_system.h>
#include <vnl/algo/vnl_lsqr.h>

#include <vnl/vnl_linear_system.h>


void TestLSQR()
{
	std::cout << __func__ << "\n";
	{
		vnl_sparse_matrix<double> m(2,2);
		m(0,0) = 1; m(0,1) = 2;
		m(1,0) = 3; m(1,1) = 4;
		
		vnl_vector<double> b(2);
		b[0] = 3;
		b[1] = 7;
		
		vnl_sparse_matrix_linear_system<double> ls(m,b);
		vnl_lsqr lsqr(ls);
		lsqr.set_max_iterations(100);
		vnl_vector<double> x(2);
		lsqr.minimize(x);
		lsqr.diagnose_outcome(std::cout);
		std::cout << x << std::endl;
	}
	
	{
		vnl_sparse_matrix<double> m(3,2);
		m(0,0) = 1; m(0,1) = 2;
		m(1,0) = 3; m(1,1) = 4;
		m(2,0) = 1; m(2,1) = 3;
		
		vnl_vector<double> b(3);
		b[0] = 3;
		b[1] = 7;
		b[2] = 3;
		
		vnl_sparse_matrix_linear_system<double> ls(m,b);
		vnl_lsqr lsqr(ls);
		lsqr.set_max_iterations(100);
		vnl_vector<double> x(2);
		lsqr.minimize(x);
		lsqr.diagnose_outcome(std::cout);
		std::cout << x << std::endl;
	}
}

#include <iomanip>

int PerformSVDTest()
{
	vnl_matrix<double> P(4,5);
	P(0,0) = 1; P(0,4) = 2;
	P(1,2) = 3;
	P(3,1) = 4;
	
	vnl_svd<double> svd(P);
	
	std::cout << svd.W() << std::endl;
	std::cout << svd.V().transpose() << std::endl;
	
	vnl_matrix<double> M(3,2);
	M(0,0) = 2; M(0,1) = 4;
	M(1,0) = 1; M(1,1) = -1;
	M(2,0) = -4; M(2,1) = -2;
	
	vnl_svd<double> decomp(M);
	std::cout << decomp.W() << std::endl;
	int index = decomp.rank() - 1;
	std::cout << "Smallest singular value = " << decomp.W(index) << std::endl;
	std::cout << "Normal Vector = " << decomp.V().get_column(index) << std::endl;
	
	{
		vnl_matrix<double> M(3,2);
		M(0,0) = 2; M(0,1) = 4;
		M(1,0) = 1; M(1,1) = -1;
		M(2,0) = -4; M(2,1) = -2;
		
		vnl_matrix<double> A(M.transpose() * M);
		vnl_svd<double> decomp(A); // same as diagonalization/eigendecomposition
		std::cout << decomp.W() << std::endl;
		int index = decomp.rank() - 1;
		std::cout << "Smallest singular value = " << decomp.W(index) << std::endl;
		std::cout << "Normal Vector = " << decomp.V().get_column(index) << std::endl;
	}
	
	{ //another example
		vnl_matrix<double> M(3,2);
		M(0,0) = -2; M(0,1) = -1;
		M(1,0) = -1; M(1,1) =  3;
		M(2,0) =  3; M(2,1) = -2;
		
		vnl_matrix<double> A(M.transpose() * M);
		vnl_svd<double> decomp(A);
		std::cout << decomp.W() << std::endl;
		int index = decomp.rank() - 1;
		std::cout << "Smallest singular value = " << decomp.W(index) << std::endl;
		std::cout << "Normal Vector = " << decomp.V().get_column(index) << std::endl;
	}
	
	{   //Ordinary Least Squares.
		vnl_matrix<double> M(3,2);
		M(0,0) = 1; M(0,1) = 1;
		M(1,0) = 2; M(1,1) = 1;
		M(2,0) = 6; M(2,1) = 1;
		
		vnl_matrix<double> A(M.transpose() * M);
		
		vnl_vector<double> b(3);
		b(0) = 2; b(1) = 6; b(2) = 1;
		b = M.transpose() * b;
		
		
		vnl_svd<double> decomp(A);
		vnl_vector<double> x(decomp.solve(b));
		
		std::cout << "a & b = " << x << std::endl;
 
		
	}
	return 0;
}

#endif /* CAPTOTALLEASTSQUARES_H_ */
