/*
 * CAPModelLVPS4X4.h
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLVPS4X4_H_
#define CAPMODELLVPS4X4_H_

#include <string>
#include <vector>

class CAPModelLVPS4X4
{
	typedef float gtMatrix[4][4];
public:
	CAPModelLVPS4X4();
	
	typedef std::vector<float> parameters;
	int readModelFromFiles(const std::string& path);
	
	int setParameters(const std::vector<float>& globalParemeters);
	const std::vector<float>& getParameters() const;

	double calculateVolume();
	double calculateMass();
	
	int setLocalToGlobalTransformation(const gtMatrix& transform);
private:
	gtMatrix patientToGlobalTransform; // model to world transformation
};

#endif /* CAPMODELLVPS4X4_H_ */
