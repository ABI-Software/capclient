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
#include "CmguiExtensions.h"

class DataPoint
{
public:
	
	DataPoint(Cmiss_node* node, const Point3D& coord, float time = 0, float weight = 1.0f)
	:
		cmissNode_(ACCESS(Cmiss_node)(node)),
		coordinate_(coord),
		time_(time),
		weight_(weight),
		surfaceType_(0),
		startTime_(time),
		endTime_(time)
	{
	};
	
	DataPoint(const DataPoint& other)
	:
		cmissNode_(ACCESS(Cmiss_node)(other.cmissNode_)),
		coordinate_(other.coordinate_),
		time_(other.time_),
		weight_(other.weight_),
		surfaceType_(other.surfaceType_),
		startTime_(other.startTime_),
		endTime_(other.endTime_)
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
	
	Cmiss_node* GetCmissNode()
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
	
	void SetValidPeriod(float startTime, float endTime)
	{
		startTime_ = startTime;
		endTime_ = endTime;
	}
	
	void SetVisible(bool visibility)
	{
		Cmiss_node_set_visibility_field(*this, startTime_, endTime_, visibility);
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
	
	// assignment operator
	DataPoint& operator=(const DataPoint& rhs)
	{
		DEACCESS(Cmiss_node)(&cmissNode_);
		cmissNode_ = ACCESS(Cmiss_node)(rhs.cmissNode_);
		coordinate_ = rhs.coordinate_;
		time_ = rhs.time_;
		weight_ = rhs.weight_;
		startTime_ = rhs.startTime_;
		endTime_ = rhs.endTime_;
	}
	
private:
	Cmiss_node* cmissNode_;
	Point3D coordinate_;
	float weight_;
	float time_;
	int surfaceType_;
	float startTime_, endTime_; // these can't be inferred from time only 
};

struct DataPointTimeLessThan // used for sorting DataPoints with respect to time
{
	bool operator() (const DataPoint& i, const DataPoint& j) 
	{
		return (i.GetTime()<j.GetTime());
	}
};

class DataPointCmissNodeEqualTo
{
public:
	DataPointCmissNodeEqualTo(const Cmiss_node* id)
	: id_(id)
	{}
	
	bool operator() (const DataPoint& point)
	{
		return (id_ == point.GetCmissNode());
	}
	
private:
	const Cmiss_node* id_;
};
#endif /* DATAPOINT_H_ */
