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

DataPoint::DataPoint(Cmiss_node* node, const Point3D& coord, DataPointType dataPointType, double time, double weight)
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
	dataPointType_(other.dataPointType_),
	startTime_(other.startTime_),
	endTime_(other.endTime_)
{
};
		
DataPoint::~DataPoint()
{
//	std::cout << __func__ << ": Node id = " << Cmiss_node_get_identifier(cmissNode_) <<
//			": use count = " << FE_node_get_access_count(cmissNode_) << '\n';

	DecrementCmissNodeObjectCount();
}

void DataPoint::DecrementCmissNodeObjectCount()
{
	if (2 == FE_node_get_access_count(cmissNode_)) // means this is the last reference to the cmiss node
	{
//		std::cout << "Cmiss_node use_count = 2\n";
		FE_region* fe_region = FE_node_get_FE_region(cmissNode_); //REVISE
		fe_region = FE_region_get_data_FE_region(fe_region);
		FE_region_remove_FE_node(fe_region, cmissNode_); // access = 1; 
	}
//	std::cout << __LINE__ << " : " << FE_node_get_access_count(cmissNode_) << '\n';
	Cmiss_node_destroy(&cmissNode_);
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
	std::cout << __func__ << ": StartTime = " << startTime_ << " , endTime = " << endTime_ << '\n';
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
	DecrementCmissNodeObjectCount();

	cmissNode_ = ACCESS(Cmiss_node)(rhs.cmissNode_);
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
