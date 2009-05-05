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
	:S(11,11)
	{}
	
	gmm::csc_matrix<double> S;
};

CAPTimeSmoother::CAPTimeSmoother()
:
	pImpl(new CAPTimeSmootherImpl)
{
	//read in S
	Harwell_Boeing_load(Sfile, pImpl->S);
}

double CAPTimeSmoother::MapToXi(float time)
{
	return time;
}

std::vector<double> CAPTimeSmoother::FitModel(const std::vector<float>& dataPoints)
{
	// 1. Project data points (from each frame) to model to get corresponding xi
	// Here the data points are the nodal parameters at each frame and linearly map to xi
	// 2. Construct P
	
	Cim1DFourierBasis basis;
	int numRows = dataPoints.size();
	const int NUMBER_OF_PARAMETERS = 11;
	gmm::dense_matrix<double> P(numRows, NUMBER_OF_PARAMETERS);
	for (int i = 0; i < numRows; i++)
	{
		double xiDouble[1];
		xiDouble[0] = MapToXi(dataPoints[i]);
		double psi[NUMBER_OF_PARAMETERS];
		basis.evaluateBasis(psi, xiDouble);
		for (int columnIndex = 0; columnIndex < NUMBER_OF_PARAMETERS; columnIndex++)
		{
			P(i, columnIndex) = psi[columnIndex];
		}
	}
	
	// 3. Construct A
	// Note that G is the identity matrix. StS is read in from file.
	gmm::dense_matrix<double> A(NUMBER_OF_PARAMETERS, NUMBER_OF_PARAMETERS), 
				temp(NUMBER_OF_PARAMETERS, NUMBER_OF_PARAMETERS);
	gmm::mult(gmm::transposed(P),P,temp);
	gmm::add(pImpl->S,temp,A);
	
	
	// 4. Construct rhs
	std::vector<double> prior(11), p(numRows), rhs(11);
	gmm::mult(P, prior, p);
	
	std::transform(dataPoints.begin(), dataPoints.end(), p.begin(), p.begin(), std::minus<double>());
	
	gmm::mult(transposed(P),p,rhs);
	
	// 5. Solve normal equation (direct solver) 
	std::vector<double> x(gmm::mat_nrows(A));
	gmm::lu_solve(A, x, rhs);
	
	return x;
}
