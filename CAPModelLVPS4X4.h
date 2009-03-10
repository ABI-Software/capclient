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
	
public:
	typedef float gtMatrix[4][4];
	
	enum RenderMode
	{
		LINE,
		WIREFRAME,
		SOLID
	};
	
	CAPModelLVPS4X4(const std::string& name);
	
	typedef std::vector<float> parameters;
	int ReadModelFromFiles(const std::string& path);
	
	int SetParameters(const std::vector<float>& globalParemeters);
	const std::vector<float>& GetParameters() const;

	double CalculateVolume();
	double CalculateMass();
	
	void SetRenderMode(RenderMode mode);
	
	int SetLocalToGlobalTransformation(const gtMatrix& transform);
	
	const gtMatrix& GetLocalToGlobalTransformation()
	{
		return patientToGlobalTransform_;
	}
	
	void SetMIIVisibility(bool visibility);
	
	void SetModelVisibility(bool visibility);
	
private:
	
	void ReadModelInfo(std::string modelInfoFilePath);
	
	gtMatrix patientToGlobalTransform_; // model to world transformation
	std::string modelName_;
	int numberOfModelFrames_;
};

#endif /* CAPMODELLVPS4X4_H_ */
