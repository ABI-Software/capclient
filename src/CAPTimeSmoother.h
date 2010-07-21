/*
 * CAPTimeSmoother.h
 *
 *  Created on: May 5, 2009
 *      Author: jchu014
 */

#ifndef CAPTIMESMOOTHER_H_
#define CAPTIMESMOOTHER_H_

#include <vector>

namespace cap
{

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
	
	std::vector<double> FitModel(int parameterIndex, 
			const std::vector<float>& dataPoints,
			const std::vector<int>& framesWithDataPoints) const;
	
	float ComputeLambda(double xi, const std::vector<double>& params) const;
	
	std::vector<double> GetPrior(int paramNumber) const;
	
private:
	double MapToXi(float time) const;
	CAPTimeSmootherImpl* pImpl;
	
	static const int NUMBER_OF_PARAMETERS = 11;
	static const int CAP_WEIGHT_GP = 10;
};

} // end namespace cap
#endif /* CAPTIMESMOOTHER_H_ */
