/*
 * DataPoint.cpp
 *
 *  Created on: Jun 26, 2009
 *      Author: jchu014
 */

#include "datapoint.h"

#include "utils/misc.h"
#include "utils/debug.h"

#include <limits>

extern "C" {
}

namespace cap
{

DataPoint::DataPoint(Cmiss_node* node, const Point3D& coord, ModellingEnum dataPointType, double time, double weight)
	: cmissNode_(Cmiss_node_access(node))
	, coordinate_(coord)
	, time_(time)
	, weight_(weight)
	, surfaceType_(UNDEFINED_HEART_SURFACE_TYPE)
	, dataPointType_(dataPointType)
	, startTime_(time)
	, endTime_(time)
{
};
	
DataPoint::DataPoint(const DataPoint& other)
	: cmissNode_(Cmiss_node_access(other.cmissNode_))
	, coordinate_(other.coordinate_)
	, time_(other.time_)
	, weight_(other.weight_)
	, surfaceType_(other.surfaceType_)
	, dataPointType_(other.dataPointType_)
	, startTime_(other.startTime_)
	, endTime_(other.endTime_)
{
};

DataPoint::~DataPoint()
{
	dbg("DataPoint::~DataPoint()");
	Cmiss_node_destroy(&cmissNode_);
}

void DataPoint::DecrementCmissNodeObjectCount()
{
//	if (2 == FE_node_get_access_count(cmissNode_)) // means this is the last reference to the cmiss node
	{
//		std::cout << "Cmiss_node use_count = 2\n";
//		FE_region* fe_region = FE_node_get_FE_region(cmissNode_); //REVISE
//		fe_region = FE_region_get_data_FE_region(fe_region);
//		FE_region_remove_FE_node(fe_region, cmissNode_); // access = 1; 
	}
//	std::cout << __LINE__ << " : " << FE_node_get_access_count(cmissNode_) << '\n';
//	Cmiss_node_destroy(&cmissNode_);
//	std::cout << __LINE__ << " : " << FE_node_get_access_count(cmissNode_) << '\n';
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

double DataPoint::GetTime() const
{
	return time_;
}

void DataPoint::SetValidPeriod(double startTime, double endTime)
{
	const double EPSILON = std::numeric_limits<double>::epsilon();
	startTime_ = startTime;
	endTime_ = endTime - EPSILON;
}

void DataPoint::SetVisible(bool visibility)
{
	dbg(std::string(__func__) + ": StartTime = " + ToString(startTime_) + " , endTime = " + ToString(endTime_));
	//--Cmiss_node_set_visibility_field(cmissNode_, startTime_, endTime_, visibility);
}

ModellingEnum DataPoint::GetDataPointType() const
{
	return dataPointType_;
}

void DataPoint::SetDataPointType(ModellingEnum type)
{
	dataPointType_ = type;
}

HeartSurfaceEnum DataPoint::GetSurfaceType() const
{
	return surfaceType_;
}

void DataPoint::SetSurfaceType(HeartSurfaceEnum type)
{
	surfaceType_ = type;
}

std::string DataPoint::GetSliceName() const
{
//	FE_region* fe_region = FE_node_get_FE_region(cmissNode_);
//	Cmiss_region* cmiss_region;
//	FE_region_get_Cmiss_region(fe_region, &cmiss_region);
	
//	char* name_c = Cmiss_region_get_name(cmiss_region);
//	std::string name(name_c);
//	free(name_c);
	
	return std::string("empty");
}

// assignment operator
DataPoint& DataPoint::operator=(const DataPoint& rhs)
{
	cmissNode_ = Cmiss_node_access(rhs.cmissNode_);
	coordinate_ = rhs.coordinate_;
	time_ = rhs.time_;
	weight_ = rhs.weight_;
	startTime_ = rhs.startTime_;
	endTime_ = rhs.endTime_;
	surfaceType_ = rhs.surfaceType_;
	dataPointType_ = rhs.dataPointType_;
	
	return *this;
}
	
} // end namespace cap
