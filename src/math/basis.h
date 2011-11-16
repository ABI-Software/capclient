/*
 * CAPBasis.h
 *
 *  Created on: Jun 17, 2009
 *      Author: jchu014
 */

#ifndef CAPBASIS_H_
#define CAPBASIS_H_

#include <cmath>
#ifndef M_PI
# define M_PI	3.14159265358979323846
#endif

namespace cap
{


class FourierBasis
{
public:
	void Evaluate(double bf[], const double xi[]) const
	{
		double x = xi[0];
		bf[0] = 1.0;
		bf[1] = cos(2*M_PI*x);
		bf[2] = sin(2*M_PI*x);
		bf[3] = cos(4*M_PI*x);
		bf[4] = sin(4*M_PI*x);
		bf[5] = cos(6*M_PI*x);
		bf[6] = sin(6*M_PI*x);
		bf[7] = cos(8*M_PI*x);
		bf[8] = sin(8*M_PI*x);
		bf[9] = cos(10*M_PI*x);
		bf[10]= sin(10*M_PI*x);
	}
};

class BiCubicHermiteLinearBasis
{
public:
	void Evaluate(double bf[], const double xi[]) const
	{
		double bf1[4], bf2[4], bf3[2], temp[1];
		
		temp[0] = xi[0];
		this->EvaluateHermiteBasis(bf1, temp);
		temp[0] = xi[1];
		this->EvaluateHermiteBasis(bf2, temp);
		temp[0] = xi[2];
		this->EvaluateLinearBasis(bf3, temp);
		
		bf[0] = bf1[0]*bf2[0]*bf3[0];
		bf[1] = bf1[1]*bf2[0]*bf3[0];
		bf[2] = bf1[0]*bf2[1]*bf3[0];
		bf[3] = bf1[1]*bf2[1]*bf3[0];
		
		bf[4] = bf1[2]*bf2[0]*bf3[0];
		bf[5] = bf1[3]*bf2[0]*bf3[0];
		bf[6] = bf1[2]*bf2[1]*bf3[0];
		bf[7] = bf1[3]*bf2[1]*bf3[0];
		
		bf[8] = bf1[0]*bf2[2]*bf3[0];
		bf[9] = bf1[1]*bf2[2]*bf3[0];
		bf[10]= bf1[0]*bf2[3]*bf3[0];
		bf[11]= bf1[1]*bf2[3]*bf3[0];
		
		bf[12] = bf1[2]*bf2[2]*bf3[0];
		bf[13] = bf1[3]*bf2[2]*bf3[0];
		bf[14]= bf1[2]*bf2[3]*bf3[0];
		bf[15]= bf1[3]*bf2[3]*bf3[0];
		
		bf[16] = bf1[0]*bf2[0]*bf3[1];
		bf[17] = bf1[1]*bf2[0]*bf3[1];
		bf[18] = bf1[0]*bf2[1]*bf3[1];
		bf[19] = bf1[1]*bf2[1]*bf3[1];
		
		bf[20] = bf1[2]*bf2[0]*bf3[1];
		bf[21] = bf1[3]*bf2[0]*bf3[1];
		bf[22] = bf1[2]*bf2[1]*bf3[1];
		bf[23] = bf1[3]*bf2[1]*bf3[1];
		
		bf[24] = bf1[0]*bf2[2]*bf3[1];
		bf[25] = bf1[1]*bf2[2]*bf3[1];
		bf[26]= bf1[0]*bf2[3]*bf3[1];
		bf[27]= bf1[1]*bf2[3]*bf3[1];
		
		bf[28] = bf1[2]*bf2[2]*bf3[1];
		bf[29] = bf1[3]*bf2[2]*bf3[1];
		bf[30]= bf1[2]*bf2[3]*bf3[1];
		bf[31]= bf1[3]*bf2[3]*bf3[1];
	}
	
private:
	void EvaluateHermiteBasis(double bf[], const double xi[]) const
	{
	  bf[0] = 1.0 - 3.0 * xi[0] * xi[0] + 2.0 * xi[0] * xi[0] * xi[0];
	  bf[1] = 1.0 * xi[0] - 2.0 * xi[0] * xi[0] + xi[0] * xi[0] * xi[0];
	  bf[2] = 3.0 * xi[0] * xi[0] - 2.0 * xi[0] * xi[0] * xi[0];
	  bf[3] = -1.0 * xi[0] * xi[0] + xi[0] * xi[0] * xi[0];
	}
	
	void EvaluateLinearBasis(double bf[], const double xi[]) const
	{
	  bf[0] = 1.0 - xi[0];
	  bf[1] = xi[0];
	}
};

} //end namespace cap

#endif /* CAPBASIS_H_ */
