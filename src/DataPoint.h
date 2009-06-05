/*
 * DataPoint.h
 *
 *  Created on: Apr 1, 2009
 *      Author: jchu014
 */

#ifndef DATAPOINT_H_
#define DATAPOINT_H_

#include "CAPMath.h"
extern "C" {
#include "api/cmiss_node.h"
#include "finite_element/finite_element.h"
}

class DataPoint
{
public:
	DataPoint(Cmiss_node* node, const Point3D& coord, float time = 0, float weight = 1.0f)
	:
		cmissNode_(ACCESS(Cmiss_node)(node)),
		coordinate_(coord),
		time_(time),
		weight_(weight),
		surfaceType_(0)
	{
	};
	
	DataPoint(const DataPoint& other)
	:
		cmissNode_(ACCESS(Cmiss_node)(other.cmissNode_)),
		coordinate_(other.coordinate_),
		time_(other.time_),
		weight_(other.weight_),
		surfaceType_(other.surfaceType_)
	{
	};
		
	~DataPoint()
	{
		destroy_Cmiss_node(&cmissNode_);
	}
	
	const Cmiss_node* GetCmissNode() const
	{
		return cmissNode_;
	}
	
	const Point3D& GetCoordinate() const
	{
		return coordinate_;
	}
	
	void SetCoordinate(const Point3D& coord)
	{
		coordinate_ = coord;
	}
	
	float GetTime() const
	{
		return time_;
	}
	
	//HACK
	int GetSurfaceType() const
	{
		return surfaceType_;
	}
	
	void SetSurfaceType(int type)
	{
		surfaceType_ = type;
	}
	
	DataPoint& operator=(const DataPoint& rhs)
	{
		DEACCESS(Cmiss_node)(&cmissNode_);
		cmissNode_ = ACCESS(Cmiss_node)(rhs.cmissNode_);
		coordinate_ = rhs.coordinate_;
		time_ = rhs.time_;
		weight_ = rhs.weight_;
		surfaceType_ = rhs.surfaceType_;
	}
	
private:
	Cmiss_node* cmissNode_;
	Point3D coordinate_;
	float weight_;
	float time_;
	
	int surfaceType_;
};

struct DataPointTimeComparator // used for sorting DataPoints with respect to time
{
	bool operator() (const DataPoint& i, const DataPoint& j) 
	{
		return (i.GetTime()<j.GetTime());
	}
};

#endif /* DATAPOINT_H_ */
