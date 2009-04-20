/*
 * SolverLibraryFactory.h
 *
 *  Created on: Apr 6, 2009
 *      Author: jchu014
 */

#ifndef SOLVERLIBRARYFACTORY_H_
#define SOLVERLIBRARYFACTORY_H_

#include <string>
#include <iostream>
#include <fstream>

class Matrix
{
};

class Vector
{
public:
	virtual Vector& operator=(double value) = 0;
	virtual ~Vector(){};
	
	virtual std::string ToString() const = 0;
};

std::ostream& operator<<(std::ostream& out, const Vector& v)
{
	out << v.ToString();
	
	return out;
}

class GSmoothAMatrix : public Matrix
{
public:
	virtual void UpdateData(Matrix& P) = 0;
	virtual ~GSmoothAMatrix(){};
};

class Preconditioner
{
};

class SolverLibraryFactory
{
public:
	SolverLibraryFactory(const std::string name)
	: name_(name)
	{}
	
	virtual ~SolverLibraryFactory(){};
	
	virtual Vector* CreateVector(int dimension = 0, double value = 0.0) = 0;
	virtual Vector* CreateVectorFromFile(const std::string& filename) = 0;
	
	virtual Matrix* CreateMatrixFromFile(const std::string& filename) = 0;
	virtual GSmoothAMatrix* CreateGSmoothAMatrix(Matrix& S, Matrix& G) = 0;
	
	virtual Preconditioner* CreateDiagonalPreconditioner(const Matrix& m) = 0;
	
	virtual void CG(const Matrix& A, Vector& x, const Vector& rhs, const Preconditioner& pre, int maximumIteration, double tolerance) = 0;
	
	virtual const std::string& GetName()
	{
		return name_;
	}
protected:
	std::string name_;
};

#endif /* SOLVERLIBRARYFACTORY_H_ */
