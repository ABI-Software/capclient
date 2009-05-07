/*
 * CAPTimeSmoother.cpp
 *
 *  Created on: May 5, 2009
 *      Author: jchu014
 */

//#include "SolverLibraryFactory.h"
#include "Cim1DFourierBasis.h"
#include "gmm/gmm.h" // TODO use wrapper

#include "CAPTimeSmoother.h"

const static char* Sfile = "Data/templates/GlobalSmoothTVMatrix.dat";
const static char* priorFile = "Data/templates/time_varying_prior.dat";

struct CAPTimeSmootherImpl
{
	CAPTimeSmootherImpl()
	:S(11,11),
	Priors(11,134)
	{}
	
	gmm::csc_matrix<double> S;
	gmm::dense_matrix<double> Priors;
};

CAPTimeSmoother::CAPTimeSmoother()
:
	pImpl(new CAPTimeSmootherImpl)
{
	//read in S
	Harwell_Boeing_load(Sfile, pImpl->S);
	
	std::ifstream in(priorFile);
	
	for (int row = 0; row < 11; row++)
	{
		for (int col = 0;col < 134;col++)
		{
			in >> pImpl->Priors(row,col);
		}
		for (int col = 0;col < 80 ;col ++)
		{
			double temp;
			in >> temp; // mu and theta?
		}
	}
	
	//Debug
	std::cout << pImpl->Priors << std::endl;
}

double CAPTimeSmoother::MapToXi(float time)
{
	return time;
}

std::vector<double> CAPTimeSmoother::FitModel(int parameterIndex, const std::vector<float>& dataPoints)
{
	// 1. Project data points (from each frame) to model to get corresponding xi
	// Here the data points are the nodal parameters at each frame and linearly map to xi
	// 2. Construct P
	
	Cim1DFourierBasis basis;
	int numRows = dataPoints.size();
	gmm::dense_matrix<double> P(numRows, NUMBER_OF_PARAMETERS);
	
//	std::cout << "\n\n\nnumRows = " << numRows << '\n';
	
	for (int i = 0; i < numRows; i++)
	{
		double xiDouble[1];
		xiDouble[0] = MapToXi(static_cast<float>(i)/numRows); //REVISE design
//		std::cout << "dataPoint(" << i << ") = " << dataPoints[i] << ", xi = "<< xiDouble[0] <<'\n';
		double psi[NUMBER_OF_PARAMETERS];
		basis.evaluateBasis(psi, xiDouble);
		for (int columnIndex = 0; columnIndex < NUMBER_OF_PARAMETERS; columnIndex++)
		{
			P(i, columnIndex) = psi[columnIndex];
		}
	}
	
//	std::cout << "P = " << P << std::endl;
	
	// 3. Construct A
	// Note that G is the identity matrix. StS is read in from file.
	gmm::dense_matrix<double> A(NUMBER_OF_PARAMETERS, NUMBER_OF_PARAMETERS), 
				temp(NUMBER_OF_PARAMETERS, NUMBER_OF_PARAMETERS);
	gmm::mult(gmm::transposed(P),P,temp);
	gmm::add(pImpl->S,temp,A);
	
//	std::cout << "A = " << A << std::endl;
	
	// 4. Construct rhs
	std::vector<double> prior(11), p(numRows), rhs(11);
	for (int i=0;i<11;i++)
	{
		prior[i] = pImpl->Priors(i, parameterIndex);
	}
	
//	std::cout << "prior: " << prior << std::endl;
	
	gmm::mult(P, prior, p);
	
	std::transform(dataPoints.begin(), dataPoints.end(), p.begin(), p.begin(), std::minus<double>());
	
	gmm::mult(transposed(P),p,rhs);
	
//	std::cout << "rhs: " << rhs << std::endl;
	
	// 5. Solve normal equation (direct solver) 
	std::vector<double> x(gmm::mat_nrows(A));
	gmm::lu_solve(A, x, rhs);
	
#ifndef NDEBUG
//	std::cout << "delta x (" << parameterIndex << ") " << x << std::endl;
#endif
	
	std::transform(x.begin(), x.end(), prior.begin(), x.begin(), std::plus<double>());
	return x;
}

std::vector<double> CAPTimeSmoother::GetPrior(int paramNumber)
{
	std::vector<double> prior(11);
	for (int i =0; i<11; i++)
	{
		prior[i] = pImpl->Priors(i, paramNumber);
	}
	
	return prior;
}

float CAPTimeSmoother::ComputeLambda(double xi, const std::vector<double>& params)
{
	double psi[NUMBER_OF_PARAMETERS];
	Cim1DFourierBasis basis;
	double xiDouble[1];
	xiDouble[0] = xi;
	basis.evaluateBasis(psi, xiDouble);
	
	float lambda(0);
	for (int i = 0; i<NUMBER_OF_PARAMETERS; i++)
	{
		lambda += params[i]*psi[i];
	}
	
	return lambda;
}
