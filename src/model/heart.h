/*
 * HeartModel.h
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLVPS4X4_H_
#define CAPMODELLVPS4X4_H_

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>

extern "C"
{
#include <configure/cmgui_configure.h>
#include <api/cmiss_context.h>
}

#include "capclientconfig.h"
#include "standardheartdefinitions.h"
#include "math/algebra.h"

struct Scene_object;

namespace cap
{

class Point3D;
class Vector3D;
struct Plane;

/**
 * Left ventricular prolate spheriod heart model.  This class
 * holds the information related to the heart model.
 */
class HeartModel
{
	
public:

	/**
	 * Values that represent RenderMode. 
	 */
	enum RenderMode
	{
		WIREFRAME,
		SHADED
	};

	/**
	 * Constructor.
	 *
	 * @param	cmissContext	The context for the heart model.
	 */
	explicit HeartModel(Cmiss_context_id cmissContext);

	/**
	 * Destructor.
	 */
	~HeartModel();

	/**
	 * Defines an alias representing options for controlling the operation.
	 */
	typedef std::vector<double> parameters;

	/**
	 * Reads a model from files.
	 *
	 * @param	modelDirectory 	Pathname of the model directory.
	 * @param	directoryPrefix	The directory prefix.
	 *
	 * @return	The model from files.
	 */
	int ReadModelFromFiles(const std::string& modelDirectory, const std::string& directoryPrefix);

	/**
	 * Reads a model from files.
	 *
	 * @param	dir_path	  	Pathname of the directory.
	 * @param	modelFilenames	The model filenames.
	 *
	 * @return	The model from files.
	 */
	int ReadModelFromFiles(const std::string& dir_path, const std::vector<std::string>& modelFilenames);

	/**
	 * Writes to file.
	 *
	 * @param	path	Full pathname of the file.
	 */
	void WriteToFile(const std::string& path);

	/**
	 * Gets a lambda.
	 *
	 * @param	frame	The frame.
	 *
	 * @return	The lambda.
	 */
	const std::vector<double> GetLambda(int frame) const;

	/**
	 * Sets a lambda.
	 *
	 * @param	lambdaParams	Options for controlling the lambda.
	 * @param	time			(optional) the time.
	 */
	void SetLambda(const std::vector<double>& lambdaParams, double time = 0);

	/**
	 * Sets a lambda for frame.
	 *
	 * @param	lambdaParams	Options for controlling the lambda.
	 * @param	frameNumber 	The frame number.
	 */
	void SetLambdaForFrame(const std::vector<double>& lambdaParams, int frameNumber);

	/**
	 * Sets a mu from base plane for frame.
	 *
	 * @param	basePlane  	The base plane.
	 * @param	frameNumber	The frame number.
	 */
	//void SetMuFromBasePlaneForFrame(const Plane& basePlane, int frameNumber);
	void SetMuFromBasePlaneForFrame(const Plane& basePlane, double time);

	/**
	 * Sets a theta.
	 *
	 * @param	frameNumber	The frame number.
	 */
	void SetTheta(int frameNumber);
	
//	const std::vector<double>& GetParameters() const;

//	double CalculateVolume();
//	double CalculateMass();

	/**
	 * Sets a local to global transformation.  The transformation
	 * matrix defines a 4x4 rectangular cartesian coordinate 
	 * transformation matrix.  The matrix is specified as follows
	 * 
	 * [ Sx*R11, Sx*R12, Sx*R13, 0 ]
	 * [ Sy*R21, Sy*R22, Sy*R23, 0 ]
	 * [ Sz*R31, Sz*R32, Sz*R33, 0 ]
	 * [   x   ,   y   ,   z   , 1 ]
	 * 
	 *
	 * @param	transform	The transform.
	 */
	void SetLocalToGlobalTransformation(const gtMatrix& transform);

	/**
	 * Gets the local to global transformation.
	 *
	 * @return	The local to global transformation.
	 */
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

	/**
	 * Transform to prolate sheroidal.
	 *
	 * @param	rc	The rectangle.
	 *
	 * @return	.
	 */
	Point3D TransformToProlateSpheroidal(const Point3D& rc) const;

	/**
	 * Transform to local coordinate rectanglar cartesian.
	 *
	 * @param	global	The global.
	 *
	 * @return	.
	 */
	Point3D TransformToLocalCoordinateRC(const Point3D& global) const;

	/**
	 * Transform to local coordinate rectanglar cartesian.
	 *
	 * @param	global	The global.
	 *
	 * @return	.
	 */
	Vector3D TransformToLocalCoordinateRC(const Vector3D& global) const;

	/**
	 * Gets the focal length.
	 *
	 * @return	The focal length.
	 */
	double GetFocalLength() const
	{
		return focalLength_;
	}

	/**
	 * Sets a focal lengh.
	 *
	 * @param	focalLength	Length of the focal.
	 */
	void SetFocalLength(double focalLength);
	
	// Member functions related to rendering

	/**
	 * Map to model frame time.  Maps the argument time to the closest frame in time and returns the
	 * frame time.
	 *
	 * @param	time	The time.
	 *
	 * @return	Nearest frame time.
	 */
	double MapToModelFrameTime(double time) const;

	/**
	 * Map to model frame number.
	 *
	 * @param	time	The time.
	 *
	 * @return	.
	 */
	int MapToModelFrameNumber(double time) const;

	/**
	 * Gets the number of model frames.
	 *
	 * @return	The number of model frames.
	 */
	int GetNumberOfModelFrames() const
	{
		return numberOfModelFrames_;
	}

	/**
	 * Sets a number of model frames.
	 *
	 * @param	numberOfFrames	Number of frames.
	 */
	void SetNumberOfModelFrames(const int numberOfFrames)
	{
		numberOfModelFrames_ = numberOfFrames;
	}

	/**
	 * Calculates the volume.
	 *
	 * @param	surface	The surface.
	 * @param	time   	The time.
	 *
	 * @return	The calculated volume.
	 */
	double ComputeVolume(SurfaceType surface, double time) const;

	/**
	 * Gets the exnode file names.
	 *
	 * @return	The exnode file names.
	 */
	const std::vector<std::string>& GetExnodeFileNames() const
	{
		return exnodeModelFileNames_;
	}

	/**
	 * Gets the exelem file name.
	 *
	 * @return	The exelem file name.
	 */
	const std::string& GetExelemFileName() const
	{
		static std::string const exelemFileName("GlobalHermiteParam.exelem");
		return exelemFileName;
	}

private:
	
	static const int NUMBER_OF_NODES = 40;  /**< Number of nodes */

	/**
	 * Reads model information from file.
	 *
	 * @param	modelInfoFilePath	Full pathname of the model information file.
	 */
	void ReadModelInfo(const std::string& modelInfoFilePath);

	/**
	 * Writes model information to file.
	 *
	 * @param	modelInfoFilePath	Full pathname of the model information file.
	 */
	void WriteModelInfo(const std::string& modelInfoFilePath);
	
	gtMatrix patientToGlobalTransform_; /**< The patient to global transform */
	std::string modelName_; /**< Name of the model */
	std::vector<std::string> exnodeModelFileNames_; /**< List of names of the exnode model files */
	int numberOfModelFrames_;   /**< Number of model frames */
	
	double focalLength_;	/**< focal length for prolate spheriod coordinate system */
	
	class HeartModelImpl;
	HeartModelImpl* pImpl_; /**< use PIMPL to hide Cmgui related implementation details (region, scene object , etc) */

	/**
	 * Copy constructor. Non copyable
	 *
	 * @param	rhs	The right hand side.
	 */
	HeartModel(const HeartModel& rhs);

	/**
	 * Assignment operator. Non copyable
	 *
	 * @param	rhs	The right hand side.
	 *
	 * @return	A shallow copy of this object.
	 */
	HeartModel& operator=(const HeartModel& rhs);
};

} // end namespace cap

#endif /* CAPMODELLVPS4X4_H_ */