/*
 * TimeSmoother.h
 *
 *  Created on: May 5, 2009
 *      Author: jchu014
 */

#ifndef CAPTIMESMOOTHER_H_
#define CAPTIMESMOOTHER_H_

#include <vector>

namespace cap
{

struct TimeSmootherImpl;

class TimeSmoother
{
//	class TimeVaryingModel //per nodal parameter
//	{
//		
//	};

public:
	TimeSmoother();
	
	~TimeSmoother();
	
	std::vector<double> FitModel(int parameterIndex, 
			const std::vector<double>& dataPoints,
			const std::vector<int>& framesWithDataPoints) const;
	
	double ComputeLambda(double xi, const std::vector<double>& params) const;
	
	std::vector<double> GetPrior(int paramNumber) const;
	
private:
	double MapToXi(double time) const;
	TimeSmootherImpl* pImpl;
	
	static const int NUMBER_OF_PARAMETERS = 11;
	static const int CAP_WEIGHT_GP = 10;
};

} // end namespace cap
#endif /* CAPTIMESMOOTHER_H_ */
