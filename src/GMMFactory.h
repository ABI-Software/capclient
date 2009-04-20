/*
 * GMMFactory.h
 *
 *  Created on: Apr 6, 2009
 *      Author: jchu014
 */

#ifndef GMMFACTORY_H_
#define GMMFACTORY_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include "gmm/gmm.h"
#include "SolverLibraryFactory.h"

class GMMMatrix : public Matrix
{
public:
	GMMMatrix(gmm::csc_matrix<double>& m)
	: impl_(&m)
	{}
	
	gmm::csc_matrix<double>& GetImpl()
	{
		return *impl_;
	}
	
	const gmm::csc_matrix<double>& GetImpl() const
	{
		return *impl_;
	}
	
private:
	gmm::csc_matrix<double>* impl_;
};

class GMMVector: public Vector
{
public:
	GMMVector(int dim, double value)
	: impl_(new std::vector<double>(dim, value))
	{}
	
	GMMVector(std::vector<double>& vec)
	: impl_(&vec)
	{}
	
	const std::vector<double>& GetImpl() const
	{
		return *impl_;
	}
	
	std::vector<double>& GetImpl()
	{
		return *impl_;
	}
	
	Vector& operator=(double value)
	{
		(*impl_).assign(134, value); //FIX
		return *this;
	}
	
	std::string ToString() const
	{
		std::stringstream ss;
		ss << *impl_;
		
		return ss.str();
	}
	
private:	
	std::vector<double> *impl_;
};

namespace gmm {
typedef gmm::scaled_vector_const_ref<std::vector<double>, double> VectorRef;
typedef std::vector<double> VectorImpl;

class GMMGSmoothAMatrix : public GSmoothAMatrix
{
	// This class only need to conform to the interface GMM++ cg requires 
	
public:
	gmm::csc_matrix<double>* S, *B;
	gmm::csc_matrix<double>* P;
	
	GMMGSmoothAMatrix(gmm::csc_matrix<double>& Sref, gmm::csc_matrix<double>& Bref)
	{
		S = &Sref;
		B = &Bref;
		
		nr = Sref.nrows();
		nc = Bref.ncols();
		
		pr = new double[1];  ir = new IND_TYPE[1];
		jc = new IND_TYPE[nc+1];
	}
	
	void UpdateData(Matrix& M)
	{
		P = &(static_cast<GMMMatrix*>(&M)->GetImpl());
	}
	
	typedef size_t size_type;
	typedef unsigned int IND_TYPE;
	
    size_type nrows() const
    {
    	return nr;
    }
    
    size_type ncols() const 
    {
    	return nc;
    }
    
    void do_clear()
    {
    	cout << "ERROR :do_clear()" << endl;
    }
    
    IND_TYPE* ir, *jc ,nc, nr;
    double* pr;
   
    friend void mult(const GMMGSmoothAMatrix &m, const VectorImpl &x, VectorImpl &y); 
    friend void mult(const GMMGSmoothAMatrix &m, const VectorRef &ref, const VectorImpl &v, VectorImpl &y); 
};

//namespace gmm {

void mult(const GMMGSmoothAMatrix &m, const std::vector<double> &x, std::vector<double> &y) 
{
//	cout <<" fsssff" << endl;
	std::vector<double> temp1(512), temp2(141);
	
	
	gmm::mult(*m.B,x,temp1);//temp1 = G * x;
	
//	cout << *m.P << endl;
	gmm::mult(*m.P, temp1, temp2);//temp2 = P * temp1;
	gmm::mult(gmm::transposed(*m.P), temp2, temp1);//temp1 = P.trans_mult(temp2);
	gmm::mult(gmm::transposed(*m.B), temp1, y);//temp2 = G.trans_mult(temp1);

	gmm::mult(*m.S,x,y,y);
	
//	cout << " mult1 exit" <<endl;

}

void mult(const GMMGSmoothAMatrix &m, const VectorRef &ref, const std::vector<double> &v, std::vector<double>& y) 
{
//	cout <<" fff" << endl;
	std::vector<double> x(134), temp1(512), temp2(141);

	x.assign(ref.begin_, ref.end_);
	
	mult(*m.B,x,temp1);//temp1 = G * x;
	gmm::mult(*m.P, temp1, temp2);//temp2 = P * temp1;
	gmm::mult(gmm::transposed(*m.P), temp2, temp1);//temp1 = P.trans_mult(temp2);
	gmm::mult(gmm::transposed(*m.B), temp1, y);//temp2 = G.trans_mult(temp1);
	
	gmm::mult(*m.S,x,y,y);
	gmm::add(v,y,y);

}


  template<>
  struct linalg_traits<GMMGSmoothAMatrix> {
	  static const int shift = 0;
    typedef GMMGSmoothAMatrix this_type;
    typedef this_type::IND_TYPE IND_TYPE;
    typedef linalg_const is_reference;
    typedef abstract_matrix linalg_type;
    typedef double value_type;
    typedef double origin_type;
    typedef double reference;
    typedef abstract_sparse storage_type;
    typedef abstract_null_type sub_row_type;
    typedef abstract_null_type const_sub_row_type;
    typedef abstract_null_type row_iterator;
    typedef abstract_null_type const_row_iterator;
    typedef abstract_null_type sub_col_type;
    typedef cs_vector_ref<const double *, const IND_TYPE *, 0>
    const_sub_col_type;
    typedef sparse_compressed_iterator<const double *, const IND_TYPE *,
				       const IND_TYPE *, 0>
    const_col_iterator;
    typedef abstract_null_type col_iterator;
    typedef col_major sub_orientation;
    typedef linalg_true index_sorted;
    static size_type nrows(const this_type &m) { return m.nrows(); }
    static size_type ncols(const this_type &m) { return m.ncols(); }
    static const_col_iterator col_begin(const this_type &m)
    { 
    	return const_col_iterator(m.S->pr, m.S->ir, m.S->jc, m.S->nr, m.S->pr); }
    static const_col_iterator col_end(const this_type &m)
    { 
    	return const_col_iterator(m.S->pr, m.S->ir, m.S->jc + m.S->nc, m.S->nr, m.S->pr);
	}
    static const_sub_col_type col(const const_col_iterator &it) {
      return const_sub_col_type(it.pr + *(it.jc) - shift,
				it.ir + *(it.jc) - shift,
				*(it.jc + 1) - *(it.jc), it.n);
    }
    static const origin_type* origin(const this_type &m) { return m.S->pr; }
    static void do_clear(this_type &m) { m.do_clear(); }
    static value_type access(const const_col_iterator &itcol, size_type j)
    { return col(itcol)[j]; }
  };
};  

class GMMPreconditioner : public Preconditioner
{
public:
	GMMPreconditioner(const gmm::csc_matrix<double>& m)
	: impl_(m)
	{}
	
	const gmm::diagonal_precond<gmm::csc_matrix<double> >& GetImpl() const
	{
		return impl_;
	}
private:
	gmm::diagonal_precond<gmm::csc_matrix<double> > impl_;
};

class GMMFactory : public SolverLibraryFactory
{
public:
	GMMFactory()
	: SolverLibraryFactory("GMM++")
	{}
	
	Vector* CreateVector(int dimension = 0, double value = 0.0)
	{
		return new GMMVector(dimension, value);
	}
	
	Vector* CreateVectorFromFile(const std::string& filename)
	{
		gmm::VectorImpl* y = new gmm::VectorImpl(134);//FIX
		readVector(filename.c_str(), *y);
//		std::cout << "DEBUG: y = " << *y << std::endl;
		return new GMMVector(*y);
	}
	
	Preconditioner* CreateDiagonalPreconditioner(const Matrix& m)
	{
		return new GMMPreconditioner(
				static_cast<const GMMMatrix*>(&m)->GetImpl());
	}
	
	Matrix* CreateMatrixFromFile(const std::string& filename)
	{
		gmm::csc_matrix<double>* m = new gmm::csc_matrix<double>;
		Harwell_Boeing_load(filename, *m);
		
		return new GMMMatrix(*m);
	}
	
	GSmoothAMatrix* CreateGSmoothAMatrix(Matrix& S, Matrix& G)
	{
		return new gmm::GMMGSmoothAMatrix(
				static_cast<const GMMMatrix*>(&S)->GetImpl(),
				static_cast<const GMMMatrix*>(&G)->GetImpl());
	}
	
	void CG(const Matrix& A, Vector& x, const Vector& rhs, const Preconditioner& pre, int maximumIteration, double tolerance)
	{
		std::vector<double>& xImpl = static_cast<GMMVector*>(&x)->GetImpl();
		const std::vector<double>& rhsImpl = static_cast<const GMMVector*>(&rhs)->GetImpl();
		const gmm::diagonal_precond<gmm::csc_matrix<double> >& preImpl = static_cast<const GMMPreconditioner*>(&pre)->GetImpl();
		const gmm::GMMGSmoothAMatrix* AConc = static_cast<const gmm::GMMGSmoothAMatrix*>(&A);

		gmm::iteration iter(tolerance, 0,maximumIteration);
		gmm::cg(*AConc, xImpl, rhsImpl, preImpl, iter);
	}
	
private:
	
	void readVector(const char* filename, std::vector<double>& v)
	{
		std::ifstream in(filename);
		
		int dimension;
		in >> dimension;
		
		v.reserve(dimension);
		
		for (int index = 0; index < dimension; index++)
		{
			in >> v[index];
		}
	}
};

#endif /* GMMFACTORY_H_ */
