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
#include <boost/scoped_ptr.hpp>

#include <api/cmiss_context.h>

#include "SliceInfo.h"
struct Scene_object;

namespace cap
{

class Point3D;
class Vector3D;
class Plane;

/**
 * Left ventricular prolate spheriod 4x4 model.
 */
class CAPModelLVPS4X4
{
	
public:
	
	enum RenderMode
	{
		LINE,
		WIREFRAME,
		SOLID
	};
	
//	enum SurfaceType
//	{
//		UNDEFINED,
//		ENDOCARDIUM,
//		EPICARDIUM
//	};
	
	CAPModelLVPS4X4(const std::string& name, Cmiss_context_id context);
	~CAPModelLVPS4X4();
	
	typedef std::vector<double> parameters;
	int ReadModelFromFiles(const std::string& modelDirectory, const std::string& directoryPrefix);
	
	int ReadModelFromFiles(const std::string& dir_path, const std::vector<std::string>& modelFilenames);

	void WriteToFile(const std::string& path);
	
	const std::vector<double> GetLambda(int frame) const;

	void SetLambda(const std::vector<double>& lambdaParams, double time = 0);
	void SetLambdaForFrame(const std::vector<double>& lambdaParams, int frameNumber);
	
	void SetMuFromBasePlaneForFrame(const Plane& basePlane, int frameNumber);
	
	void SetTheta(int frameNumber);
	
//	const std::vector<double>& GetParameters() const;

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
	int ComputeXi(const Point3D& dataPoint, Point3D& xi, double time) const;
	
	Point3D TransformToProlateSheroidal(const Point3D& rc) const;
	
	Point3D TransformToLocalCoordinateRC(const Point3D& global) const;
	
	Vector3D TransformToLocalCoordinateRC(const Vector3D& global) const;
	
	double GetFocalLength() const
	{
		return focalLength_;
	}
	
	void SetFocalLengh(double focalLength);
	
	// Member functions related to rendering
	void SetRenderMode(RenderMode mode);
	
	void SetMIIVisibility(bool visibility);
	
	void SetMIIVisibility(bool visibility, int index);
	
	void UpdateMII(int index, double iso_value);
	
	void SetModelVisibility(bool visibility);

	double MapToModelFrameTime(double time) const; // maps the argument time to the closest frame in time and returns the frame time
	
	int MapToModelFrameNumber(double time) const;
	
	int GetNumberOfModelFrames() const
	{
		return numberOfModelFrames_;
	}
	
	void SetNumberOfModelFrames(const int numberOfFrames)
	{
		numberOfModelFrames_ = numberOfFrames;
	}
	
	double ComputeVolume(SurfaceType surface, double time) const;
	
	std::vector<std::string> const& GetExnodeFileNames() const
	{
		return exnodeModelFileNames_;
	}
	
	std::string const& GetExelemFileName() const
	{
		static std::string const exelemFileName("GlobalHermiteParam.exelem");
		return exelemFileName;
	}

private:
	
	static const int NUMBER_OF_NODES = 40; 
	
	void ReadModelInfo(const std::string& modelInfoFilePath);
	void WriteModelInfo(const std::string& modelInfoFilePath);
	
	gtMatrix patientToGlobalTransform_; // model to world transformation
	std::string modelName_;
	std::vector<std::string> exnodeModelFileNames_;
	int numberOfModelFrames_;
	
	double focalLength_;
	
	
	Scene_object* modelSceneObject_; //pointer to the Cmgui scene object for the model
	
	class HeartModelImpl;
	boost::scoped_ptr<HeartModelImpl> pImpl_; // use PIMPL to hide Cmgui related implementation details (region, scene object , etc)
	
	// Non copyable
	CAPModelLVPS4X4(const CAPModelLVPS4X4& rhs);
	CAPModelLVPS4X4& operator=(const CAPModelLVPS4X4& rhs);
};

} // end namespace cap

#endif /* CAPMODELLVPS4X4_H_ */
