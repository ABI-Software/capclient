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
	
	enum SurfaceType
	{
		UNDEFINED,
		ENDOCARDIUM,
		EPICARDIUM
	};
	
	CAPModelLVPS4X4(const std::string& name);
	~CAPModelLVPS4X4();
	
	typedef std::vector<float> parameters;
	int ReadModelFromFiles(const std::string& path);
	
	void SetLambda(const std::vector<float>& lambdaParams, float time = 0);
	
	const std::vector<float> GetLambda(int frame);
	
	void CAPModelLVPS4X4::SetLambdaForFrame(const std::vector<float>& lambdaParams, int frameNumber);
	
//	const std::vector<float>& GetParameters() const;

//	double CalculateVolume();
//	double CalculateMass();
	
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
	int ComputeXi(const Point3D& dataPoint, Point3D& xi, float time) const;
	
	Point3D TransformToProlateSheroidal(const Point3D& rc) const;
	
	Point3D TransformToLocalCoordinateRC(const Point3D& global) const;
	
	float GetFocalLength() const
	{
		return focalLength_;
	}
	
	// Member functions related to rendering
	void SetRenderMode(RenderMode mode);
	
	void SetMIIVisibility(bool visibility);
	
	void SetMIIVisibility(bool visibility, int index);
	
	void SetModelVisibility(bool visibility);

	float MapToModelFrameTime(float time) const; // maps the argument time to the closest frame in time and returns the frame time
	
	int MapToModelFrameNumber(float time) const;
	
	int GetNumberOfModelFrames() const
	{
		return numberOfModelFrames_;
	}
	
	float ComputeVolume(SurfaceType surface, float time);
	
private:
	
	static const int NUMBER_OF_NODES = 40; 
	
	void ReadModelInfo(std::string modelInfoFilePath);
	
	gtMatrix patientToGlobalTransform_; // model to world transformation
	std::string modelName_;
	int numberOfModelFrames_;
	
	float focalLength_;
	
	
	Scene_object* modelSceneObject_; //pointer to the Cmgui scene object for the model
	
	class HeartModelImpl;
	HeartModelImpl* pImpl_; // TODO use PIMPL to hide Cmgui related implementation details (region, scene object , etc)
};

#endif /* CAPMODELLVPS4X4_H_ */
