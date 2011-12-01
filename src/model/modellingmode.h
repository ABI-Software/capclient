/*
 * ModellingMode.h
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLINGMODE_H_
#define CAPMODELLINGMODE_H_

#include <vector>
#include <map>

extern "C"
{
#include <api/cmiss_region.h>
}

#include "datapoint.h"
#include "model/modellingpoint.h"
#include "math/timesmoother.h"

namespace cap
{

class Modeller;

/**
 * Defines an alias representing the data points.
 */
typedef std::map<Cmiss_node*, DataPoint> DataPoints;

/**
 * Modelling mode.  This is an abstract base class for the modelling modes. Implementation of
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
	 * Executes the accept action.
	 *
	 * @param [in,out]	modeller	The modeller.
	 *
	 * @return	null if it fails, else.
	 */
	virtual ModellingMode* OnAccept(Modeller& modeller) = 0;

	/**
	 * Adds a data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	coord			   	The coordinate.
	 * @param	time			   	The time.
	 */
	//virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time) = 0;

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
	 * Move data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	coord			   	The coordinate.
	 * @param	time			   	The time.
	 */
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time) = 0;
	virtual void MoveModellingPoint(int node_id, const Point3D& position, double time) {}

	/**
	 * Removes the data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	time			   	The time.
	 */
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time) = 0;

	/**
	 * Gets the modelling points.  The modelling points are sorted in ascending modelling point time.
	 *
	 * @return	The modelling points.
	 */
	virtual ModellingPoints GetModellingPoints() const = 0;

	/**
	 * Perform entry action.
	 */
	virtual void PerformEntryAction() = 0;

	/**
	 * Perform exit action.
	 */
	virtual void PerformExitAction() = 0;
};

/**
 * Modelling mode apex.
 */
class ModellingModeApex : public ModellingMode
{
public:
	ModellingModeApex() {}
	~ModellingModeApex();

	ModellingMode* OnAccept(Modeller& modeller);
	//virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
	void MoveModellingPoint(int node_id, const Point3D& position, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	ModellingPoints GetModellingPoints() const;
	const ModellingPoint& GetApex() const; //REVISE design:
	// Probably its more extensible to provide a uniform interface (virtual GetDataPoints) on all Modes.
	// However it is unlikely that new modes will be added in the future so probably its ok.

	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	ModellingPointsMap apex_;   /**< The apex, holds at most one item */
};

/**
 * Modelling mode base.
 */
class ModellingModeBase : public ModellingMode
{
public:
	ModellingModeBase() {}
	
	ModellingMode* OnAccept(Modeller& modeller);
	//virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	ModellingPoints GetModellingPoints() const;
	const ModellingPoint& GetBase() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	ModellingPointsMap base_;   /**< The base, holds at most one item */
};

class HeartModel;

/**
 * Modelling mode rv.
 */
class ModellingModeRV : public ModellingMode
{
public:
	ModellingModeRV() {}
	
	ModellingMode* OnAccept(Modeller& modeller);
	//virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	ModellingPoints GetModellingPoints() const;
	const ModellingPointsMap& GetRVInsertPoints() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	ModellingPointsMap rvInserts_;	/**< The rv inserts, holds n pairs of DataPoints ( n >= 1 ) */
};

/**
 * Modelling mode base plane.
 */
class ModellingModeBasePlane : public ModellingMode
{
public:
	ModellingModeBasePlane() {}
	
	ModellingMode* OnAccept(Modeller& modeller);
	//virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	ModellingPoints GetModellingPoints() const;
	const ModellingPointsMap& GetBasePlanePoints() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	ModellingPointsMap basePlanePoints_;	/**< The base plane points, holds n pairs of DataPoints ( n >= 1 ) */
};

class Vector;
/**
 * Modelling mode guide points.
 */
class ModellingModeGuidePoints : public ModellingMode
{
public:
	ModellingModeGuidePoints();
	~ModellingModeGuidePoints();
	
	ModellingMode* OnAccept(Modeller& modeller);
	//virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	void AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	ModellingPoints GetModellingPoints() const;
	std::vector<ModellingPointsMap> GetGuidePoints() const;
	//const std::vector< std::vector<double> >& GetTimeVaryingDataPoints() const { return timeVaryingDataPoints_; }
	const std::vector<int>& GetFramesWithDataPoints() const { return framesWithDataPoints_; }
	void Reset(unsigned int numFrames);

	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	ModellingPointsMap guidePoints_;	/**< The guide points */
	std::vector<ModellingPointsMap> vectorOfModellingPoints_;
	std::vector<int> framesWithDataPoints_;
};

} // end namespace cap

#endif /* CAPMODELLINGMODE_H_ */
