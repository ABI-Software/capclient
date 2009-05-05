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
	
	std::vector<double> FitModel(const std::vector<float>& dataPoints);
	
private:
	double MapToXi(float time);
	CAPTimeSmootherImpl* pImpl;
};
#endif /* CAPTIMESMOOTHER_H_ */
