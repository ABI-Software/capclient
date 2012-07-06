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
#include <zn/cmgui_configure.h>
#include <zn/cmiss_context.h>
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
	 * Values that represent the types of RenderMode for the heart model.
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
	 * Initialises the heart model.  This must be called after the heart model has been loaded
	 * either from the template or a project file.  The name passed to this function determines the
	 * region where all the downstream fields are created.
	 *
	 * @param	name	The name of the model.
	 */
	void Initialise(const std::string& name);

	/**
	 * Sets the render mode for the heart model surfaces.  The mode are defined by RenderMode.
	 *
	 * @param	mode	The mode.
	 */
	void SetRenderMode(RenderMode mode);

	/**
	 * Sets a visibility.
	 *
	 * @param	visible	true to show, false to hide.
	 */
	void SetVisibility(bool visible);

	/**
	 * Returns the lambda parameters from the model at the given time.
	 *
	 * @param	time	The time.
	 *
	 * @return	The lambda.
	 */
	const std::vector<double> GetLambdaAtTime(double time) const;

	/**
	 * Sets the lambda parameters for the given time.
	 *
	 * @param	lambdaParams	Options for controlling the lambda.
	 * @param	time			(optional) the time.
	 */
	void SetLambdaAtTime(const std::vector<double>& lambdaParams, double time = 0.0);

    /**
     * Returns the mu parameters from the model at the given time.
     *
     * @param	time	The time.
     *
     * @return	The mus.
     */
    const std::vector<double> GetMuAtTime(double time) const;

    /**
	 * Sets a mu from base plane for frame.
	 *
	 * @param	basePlane  	The base plane.
	 * @param	frameNumber	The frame number.
	 */
	//void SetMuFromBasePlaneForFrame(const Plane& basePlane, int frameNumber);
	void SetMuFromBasePlaneAtTime(const Plane& basePlane, double time);
	
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
	
	/**   
	 * Projects a point to the model and computes the xi coords and the element id
	 * @param	position	the coordinate of the point
	 * @param	time	The time of the model to use
	 * @param xi the computed xi coord. (output)
	 * 
	 * @return Id of the element that the point is projected onto
	 */ 
	int ComputeXi(const Point3D& position, double time, Point3D& xi) const;

	/**
	 * Transform to prolate sheroidal.
	 *
	 * @param	rc	The rectangle.
	 *
	 * @return	.
	 */
	Point3D TransformToProlateSpheroidal(const Point3D& rc) const;

	/**
	 * Transform to patient rectanglar cartesian coordinate system.
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
	double GetFocalLength() const;
	/**
	 * Sets a focal lengh.
	 *
	 * @param	focalLength	Length of the focal.
	 */
	void SetFocalLength(double focalLength);

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
	double ComputeVolume(HeartSurfaceEnum surface, double time) const;

private:
	
	static const int NUMBER_OF_NODES = 40;  /**< Number of nodes */
	
	gtMatrix patientToGlobalTransform_; /**< The patient to global transform */
	std::string modelName_; /**< Name of the model */
	std::vector<std::string> exnodeModelFileNames_; /**< List of names of the exnode model files */
	int numberOfModelFrames_;   /**< Number of model frames */
	
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
