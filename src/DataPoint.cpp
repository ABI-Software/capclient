/*
 * DataPoint.cpp
 *
 *  Created on: Jun 26, 2009
 *      Author: jchu014
 */

#include "DataPoint.h"

extern "C" {
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
}
#include "CmguiExtensions.h"

#include <limits>

DataPoint::DataPoint(Cmiss_node* node, const Point3D& coord, float time, float weight)
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
	
DataPoint::DataPoint(const DataPoint& other)
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
		
DataPoint::~DataPoint()
{
	if (2 == get_FE_node_access_count(cmissNode_)) // means this is the last reference to the cmiss node
	{
		FE_region* fe_region = FE_node_get_FE_region(cmissNode_); //REVISE
		fe_region = FE_region_get_data_FE_region(fe_region);
		FE_region_remove_FE_node(fe_region, cmissNode_); // access = 1; 
	}
	destroy_Cmiss_node(&cmissNode_);
}
	
const Cmiss_node* DataPoint::GetCmissNode() const
{
	return cmissNode_;
}

Cmiss_node* DataPoint::GetCmissNode()
{
	return cmissNode_;
}

const Point3D& DataPoint::GetCoordinate() const
{
	return coordinate_;
}

void DataPoint::SetCoordinate(const Point3D& coord)
{
	coordinate_ = coord;
}

float DataPoint::GetTime() const
{
	return time_;
}

void DataPoint::SetValidPeriod(float startTime, float endTime)
{
	const float EPSILON = std::numeric_limits<float>::epsilon();
	startTime_ = startTime;
	endTime_ = endTime - EPSILON;
}

void DataPoint::SetVisible(bool visibility)
{
	Cmiss_node_set_visibility_field(*this, startTime_, endTime_, visibility);
}

//HACK
int DataPoint::GetSurfaceType() const
{
	return surfaceType_;
}

void DataPoint::SetSurfaceType(int type)
{
	surfaceType_ = type;
}

// assignment operator
DataPoint& DataPoint::operator=(const DataPoint& rhs)
{
	DEACCESS(Cmiss_node)(&cmissNode_);
	cmissNode_ = ACCESS(Cmiss_node)(rhs.cmissNode_);
	coordinate_ = rhs.coordinate_;
	time_ = rhs.time_;
	weight_ = rhs.weight_;
	startTime_ = rhs.startTime_;
	endTime_ = rhs.endTime_;
	
	return *this;
}
	
