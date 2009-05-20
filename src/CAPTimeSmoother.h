/*
 * CAPTimeSmoother.h
 *
 *  Created on: May 5, 2009
 *      Author: jchu014
 */

#ifndef CAPTIMESMOOTHER_H_
#define CAPTIMESMOOTHER_H_

#include <vector>

class CAPTimeSmootherImpl;

class CAPTimeSmoother
{
//	class TimeVaryingModel //per nodal parameter
//	{
//		
//	};

public:
	CAPTimeSmoother();
	
	~CAPTimeSmoother();
	
	std::vector<double> FitModel(int parameterIndex, const std::vector<float>& dataPoints);
	
	float ComputeLambda(double xi, const std::vector<double>& params);
	
	std::vector<double> GetPrior(int paramNumber);
	
private:
	double MapToXi(float time);
	CAPTimeSmootherImpl* pImpl;
	
	static const int NUMBER_OF_PARAMETERS = 11;
};
#endif /* CAPTIMESMOOTHER_H_ */
