/*
 * ModellingMode.h
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLINGMODE_H_
#define CAPMODELLINGMODE_H_

#include "model/modellingpoint.h"
#include "math/timesmoother.h"

#include <vector>
#include <map>

extern "C"
{
#include <zn/cmiss_region.h>
}

namespace cap
{

class Modeller;

/**
 * @brief Modelling mode.  This is an abstract base class for the modelling modes. Implementation of
 * FSM using the State Pattern.
 */
class ModellingMode
{
public:

	/**
	 * Default constructor.
	 */
	ModellingMode();

	/**
	 * Destructor.
	 */
	virtual ~ModellingMode();

	/**
	 * Executes the accept action.  If the action is accepted the next modelling mode
	 * is returned.
	 *
	 * @param [in,out]	modeller	The modeller.
	 *
	 * @return	null if it fails, otherwise it returns the next modelling mode.
	 */
	virtual ModellingMode* OnAccept(Modeller& modeller) = 0;

	virtual bool CanAccept() const { return false; }

	/**
	 * Test to see if the modelling point identified by the given node_id exists.
	 *
	 * @param node_id	The node identifier.
	 * @return	true if the modelling point exists, false otherwise.
	 */
	virtual bool DoesModellingPointExist(int node_id);

	/**
	 * Adds a modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	virtual void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time) = 0;

	/**
	 * Move modelling point.
	 *
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	virtual void MoveModellingPoint(int node_id, const Point3D& position, double time);

	/**
	 * Removes the modelling point.
	 *
	 * @param	node_id	Identifier for the node.
	 * @param	time   	The time.
	 */
	virtual void RemoveModellingPoint(int node_id, double time);

	/**
	 * Gets the modelling points.  The modelling points are sorted in ascending modelling point time.
	 *
	 * @return	The modelling points.
	 */
	virtual ModellingPoints GetModellingPoints() const;

	/**
	 * An image plane has moved, check all modelling points in this mode to see
	 * if any lie on this image plane and also require moving.  If a modelling point
	 * has been moved return true, otherwise return false.
	 *
	 * @param label	The label of the image plane.
	 * @param diff	The difference between the old image plane location and the new image plane location.
	 * @return	true if a modelling point was moved, false otherwise
	 */
	bool ImagePlaneMoved(const std::string &label, Vector3D diff);

	/**
	 * Attach the given label to the modelling point identified by the node_id.
	 * The tolerance for testing whether a modelling point is on the plane or
	 * not is set at 1e-4.
	 *
	 * @param node_id	The identifier for the node.
	 * @param label	The label of the image plane.
	 * @param location	The location of the image plane.
	 * @param normal	The normal of the image plane.
	 */
	void AttachToIfOn(int node_id, const std::string& label, const Point3D& location, const Vector3D& normal);

	/**
	 * Perform entry action. Typically changing the visibility of the modelling points.
	 */
	virtual void PerformEntryAction();

	/**
	 * Perform exit action. Typically changing the visibility of the modelling points.
	 */
	virtual void PerformExitAction();

protected:
	ModellingPointsMap modellingPoints_;	/**< The modelling points */
};

/**
 * @brief Modelling mode apex.
 */
class ModellingModeApex : public ModellingMode
{
public:

	/**
	 * Define our specialisation of the OnAccept method for the Apex modelling mode.  In this case
	 * we want to ensure that there is only one modelling point.
	 *
	 * @param [in,out]	modeller	The modeller.
	 *
	 * @return	null if it fails, else the base modelling mode.
	 */
	ModellingMode* OnAccept(Modeller& modeller);

	bool CanAccept() const;

	/**
	 * Adds an APEX type of modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
};

/**
 * @brief Modelling mode base.
 */
class ModellingModeBase : public ModellingMode
{
public:

	/**
	 * Define our specialisation of the OnAccept method for the Base modelling mode.  In this case
	 * we want to ensure that there is only one modelling point.
	 *
	 * @param [in,out]	modeller	The modeller.
	 *
	 * @return	null if it fails, else the rv modelling mode.
	 */
	ModellingMode* OnAccept(Modeller& modeller);

	bool CanAccept() const;

	/**
	 * Adds a BASE type of modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
};

/**
 * @brief Modelling mode rv.
 */
class ModellingModeRV : public ModellingMode
{
public:

	/**
	 * Define our specialisation of the OnAccept method for the RV modelling mode.  In this case
	 * we want to ensure that there is exactly zero or two modelling points per time frame.
	 *
	 * @param [in,out]	modeller	The modeller.
	 *
	 * @return	null if it fails, else the base plane modelling mode.
	 */
	ModellingMode* OnAccept(Modeller& modeller);

	bool CanAccept() const;

	/**
	 * Adds an RV type of modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
};

/**
 * @brief Modelling mode base plane.
 */
class ModellingModeBasePlane : public ModellingMode
{
public:

	/**
	 * Define our specialisation of the OnAccept method for the BASEPLANE modelling mode.  In this case
	 * we want to ensure that there is exactly zero or two modelling points per time frame.
	 *
	 * @param [in,out]	modeller	The modeller.
	 *
	 * @return	null if it fails, else.
	 */
	ModellingMode* OnAccept(Modeller& modeller);

	bool CanAccept() const;

	/**
	 * Adds a BASEPLANE type of modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
};

class Vector;
/**
 * Modelling mode guide points.
 */
class ModellingModeGuidePoints : public ModellingMode
{
public:

	/**
	 * Define our specialisation of the OnAccept method for the GUIDEPOINT modelling mode.  In this case
	 * we always return 0.  There are no more modelling modes.
	 *
	 * @param [in,out]	modeller	The modeller.
	 *
	 * @return	null, always.
	 */
	ModellingMode* OnAccept(Modeller& modeller);

	/**
	 * Adds a GUIDEPOINT type of modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);

	/**
	 * Gets the modelling points at the given time.  At the given time is taken to mean within 1e-06.
	 *
	 * @param	time	The time.
	 *
	 * @return	The modelling points at time.
	 */
	ModellingPoints GetModellingPointsAtTime(double time) const;

	/**
	 * Gets the frames with modelling points.  The frames with modelling points set the value 1 for
	 * the frame if it has one or more modelling points in it otherwise it is set to 0.
	 *
	 * @param	numFrames	Number of frames.
	 *
	 * @return	The frames with modelling points.
	 */
	std::vector<int> GetFramesWithModellingPoints(int numFrames) const;

	/**
	 * Set the heart surface type for the node.
	 *
	 * @param node_id   The node to set the surface type of.
	 * @param surface   The surface type to set.
	 */
	void SetHeartSurfaceType(int node_id, HeartSurfaceEnum surface);
};

} // end namespace cap

#endif /* CAPMODELLINGMODE_H_ */
