/*
 * SolverLibraryFactory.h
 *
 *  Created on: Apr 6, 2009
 *      Author: jchu014
 */

#ifndef SOLVERLIBRARYFACTORY_H_
#define SOLVERLIBRARYFACTORY_H_

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Vector
{
public:
//	virtual Vector& operator=(double value) = 0;
	virtual ~Vector(){};
	
	virtual std::string ToString() const = 0;
	
	virtual Vector& operator-=(const Vector&) = 0;
	
	virtual Vector& operator+=(const Vector&) = 0;
	
	virtual double& operator[](int index) = 0;
	
	virtual const double& operator[](int index) const = 0;
	
	//virtual std::vector<double> ToSTDVector();
};

inline std::ostream& operator<<(std::ostream& out, const Vector& v)
{
	out << v.ToString();
	
	return out;
}

class SparseMatrix
{
public:
	virtual ~SparseMatrix() {};
	
	virtual Vector* mult(const Vector& v) const = 0; // Should be namespace level function?
	
	virtual Vector* trans_mult(const Vector& v) const = 0;
};

class GSmoothAMatrix : public SparseMatrix
{
	// Note that this class (and its subclasses) works as a proxy to the actual Matrices G,S and P
	// and hence does not own them and not responsible for deleting them
public:
	virtual void UpdateData(const SparseMatrix& P) = 0;
	virtual ~GSmoothAMatrix(){};
};

class Preconditioner
{
};

struct Entry
{
	double value;
	int rowIndex;
	int colIndex;
};

class SolverLibraryFactory
{
public:
	SolverLibraryFactory(const std::string& name)
	: name_(name)
	{}
	
	virtual ~SolverLibraryFactory(){};
	
	virtual Vector* CreateVector(int dimension = 0, double value = 0.0) const = 0;
	virtual Vector* CreateVectorFromFile(const std::string& filename) const = 0;
	
	virtual SparseMatrix* CreateSparseMatrixFromFile(const std::string& filename) const = 0;
	virtual SparseMatrix* CreateSparseMatrix(int m, int n, const std::vector<Entry>& entries) const = 0;
	
	virtual GSmoothAMatrix* CreateGSmoothAMatrix(const SparseMatrix& S, const SparseMatrix& G) const = 0;
	
	virtual Preconditioner* CreateDiagonalPreconditioner(const SparseMatrix& m) const = 0;
	
	virtual void CG(const SparseMatrix& A, Vector& x, const Vector& rhs, const Preconditioner& pre, int maximumIteration, double tolerance) const = 0;
	
	virtual const std::string& GetName() const
	{
		return name_;
	}
protected:
	std::string name_;
};

#endif /* SOLVERLIBRARYFACTORY_H_ */
