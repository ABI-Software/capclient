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

namespace cap
{

DataPoint::DataPoint(Cmiss_node* node, const Point3D& coord, DataPointType dataPointType, float time, float weight)
:
	cmissNode_(ACCESS(Cmiss_node)(node)),
	coordinate_(coord),
	time_(time),
	weight_(weight),
	surfaceType_(UNDEFINED_SURFACE_TYPE),
	dataPointType_(dataPointType),
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
	if (2 == FE_node_get_access_count(cmissNode_)) // means this is the last reference to the cmiss node
	{
		FE_region* fe_region = FE_node_get_FE_region(cmissNode_); //REVISE
		fe_region = FE_region_get_data_FE_region(fe_region);
		FE_region_remove_FE_node(fe_region, cmissNode_); // access = 1; 
	}
	Cmiss_node_destroy(&cmissNode_);
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
	Cmiss_node_set_visibility_field(cmissNode_, startTime_, endTime_, visibility);
}

DataPointType DataPoint::GetDataPointType() const
{
	return dataPointType_;
}

void DataPoint::SetDataPointType(DataPointType type)
{
	dataPointType_ = type;
}

SurfaceType DataPoint::GetSurfaceType() const
{
	return surfaceType_;
}

void DataPoint::SetSurfaceType(SurfaceType type)
{
	surfaceType_ = type;
}

std::string DataPoint::GetSliceName() const
{
	FE_region* fe_region = FE_node_get_FE_region(cmissNode_);
	Cmiss_region* cmiss_region;
	FE_region_get_Cmiss_region(fe_region, &cmiss_region);
	
	char* name_c = Cmiss_region_get_name(cmiss_region);
	std::string name(name_c);
	free(name_c);
	
	return name;
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
	
} // end namespace cap
