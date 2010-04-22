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
class Vector3D;
class Plane;

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
	int ReadModelFromFiles(const std::string& modelDirectory, const std::string& directoryPrefix);
	
	void WriteToFile(const std::string& path);
	
	const std::vector<float> GetLambda(int frame) const;

	void SetLambda(const std::vector<float>& lambdaParams, float time = 0);
	void SetLambdaForFrame(const std::vector<float>& lambdaParams, int frameNumber);
	
	void SetMuFromBasePlaneForFrame(const Plane& basePlane, int frameNumber);
	
	void SetTheta(int frameNumber);
	
//	const std::vector<float>& GetParameters() const;

//	double CalculateVolume();
//	double CalculateMass();
	
	void SetLocalToGlobalTransformation(const gtMatrix& transform);
	
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
	
	Vector3D TransformToLocalCoordinateRC(const Vector3D& global) const;
	
	float GetFocalLength() const
	{
		return focalLength_;
	}
	
	void SetFocalLengh(float focalLength);
	
	// Member functions related to rendering
	void SetRenderMode(RenderMode mode);
	
	void SetMIIVisibility(bool visibility);
	
	void SetMIIVisibility(bool visibility, int index);
	
	void UpdateMII(int index, double iso_value);
	
	void SetModelVisibility(bool visibility);

	float MapToModelFrameTime(float time) const; // maps the argument time to the closest frame in time and returns the frame time
	
	int MapToModelFrameNumber(float time) const;
	
	int GetNumberOfModelFrames() const
	{
		return numberOfModelFrames_;
	}
	
	void SetNumberOfModelFrames(const int numberOfFrames)
	{
		numberOfModelFrames_ = numberOfFrames;
	}
	
	double ComputeVolume(SurfaceType surface, float time) const;
	
private:
	
	static const int NUMBER_OF_NODES = 40; 
	
	void ReadModelInfo(const std::string& modelInfoFilePath);
	
	void WriteModelInfo(const std::string& modelInfoFilePath);
	
	gtMatrix patientToGlobalTransform_; // model to world transformation
	std::string modelName_;
	int numberOfModelFrames_;
	
	float focalLength_;
	
	
	Scene_object* modelSceneObject_; //pointer to the Cmgui scene object for the model
	
	class HeartModelImpl;
	HeartModelImpl* pImpl_; // use PIMPL to hide Cmgui related implementation details (region, scene object , etc)
	
	// Non copyable
	CAPModelLVPS4X4(const CAPModelLVPS4X4& rhs);
	CAPModelLVPS4X4& operator=(const CAPModelLVPS4X4& rhs);
};

#endif /* CAPMODELLVPS4X4_H_ */
