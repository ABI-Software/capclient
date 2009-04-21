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

struct Scene_object;
class Point3D;

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
	~CAPModelLVPS4X4();
	
	typedef std::vector<float> parameters;
	int ReadModelFromFiles(const std::string& path);
	
	int SetParameters(const std::vector<float>& globalParemeters);
	const std::vector<float>& GetParameters() const;

	double CalculateVolume();
	double CalculateMass();
	
	int SetLocalToGlobalTransformation(const gtMatrix& transform);
	
	const gtMatrix& GetLocalToGlobalTransformation() const
	{
		return patientToGlobalTransform_;
	}
	
	/** Projects a point to the model and computes the xi coords and the element id
	 *  @param dataPoint the coordinate of the point
	 *  @param xi the computed xi coord. (output)
	 *  return value : id of the element that the point is projected onto
	 */ 
	int ComputeXi(const Point3D& dataPoint, Point3D& xi);
	
	
	// Member functions related to rendering
	void SetRenderMode(RenderMode mode);
	
	void SetMIIVisibility(bool visibility);
	
	void SetModelVisibility(bool visibility);
	
private:
	
	void ReadModelInfo(std::string modelInfoFilePath);
	
	gtMatrix patientToGlobalTransform_; // model to world transformation
	std::string modelName_;
	int numberOfModelFrames_;
	
	Scene_object* modelSceneObject_; //pointer to the Cmgui scene object for the model
	
	class HeartModelImpl;
	HeartModelImpl* pImpl_; // TODO use PIMPL to hide Cmgui related implementation details (region, scene object , etc)
};

#endif /* CAPMODELLVPS4X4_H_ */
