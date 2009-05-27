/*
 * CAPModellingMode.cpp
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#include "CAPModellingMode.h"
#include "CAPModeller.h"
#include <iostream>

extern "C"
{
#include "finite_element/finite_element_region.h"
}

CAPModellingMode::CAPModellingMode(CAPModeller& modeller) 
:
	modeller_(modeller)
{

}

CAPModellingMode::~CAPModellingMode() {

}

// CAPModellingModeApex

CAPModellingMode* CAPModellingModeApex::OnAccept()
{
	if (apex_.empty())
	{
		std::cout << __func__ << "Apex not defined" << std::endl;
		return 0;
	}
	return modeller_.GetModellingModeBase();
}

void CAPModellingModeApex::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	if (!apex_.empty())
	{
		//remove Cmiss_node from the FE_region
		FE_region* fe_region = FE_node_get_FE_region(dataPointID); //REVISE
		fe_region = FE_region_get_data_FE_region(fe_region);
		FE_region_remove_FE_node(fe_region, dataPointID); // access = 1;
		apex_.clear();
	}
	else
	{
		apex_.push_back(dataPoint);
	}
}

void CAPModellingModeApex::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	assert(apex_.size() == 1);
	assert(apex_[0].GetCmissNode() == dataPointID);
	apex_[0].SetCoordinate(coord);
	//apex_[0].SetTime(time); ??
}

void CAPModellingModeApex::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	assert(!apex_.empty());
	//remove Cmiss_node from the FE_region
	FE_region* fe_region = FE_node_get_FE_region(dataPointID); //REVISE
	fe_region = FE_region_get_data_FE_region(fe_region);
	FE_region_remove_FE_node(fe_region, dataPointID); // access = 1;
	apex_.clear();
}

// CAPModellingModeBase

CAPModellingMode* CAPModellingModeBase::OnAccept()
{
	if (base_.empty())
	{
		std::cout << __func__ << "Apex not defined" << std::endl;
		return 0;
	}
	return modeller_.GetModellingModeRV();
}

void CAPModellingModeBase::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	if (!base_.empty())
	{
		//remove Cmiss_node from the FE_region
		FE_region* fe_region = FE_node_get_FE_region(dataPointID); //REVISE
		fe_region = FE_region_get_data_FE_region(fe_region);
		FE_region_remove_FE_node(fe_region, dataPointID); // access = 1;
		base_.clear();
	}
	else
	{
		base_.push_back(dataPoint);
	}
}

void CAPModellingModeBase::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	assert(base_.size() == 1);
	assert(base_[0].GetCmissNode() == dataPointID);
	base_[0].SetCoordinate(coord);
	//apex_[0].SetTime(time); ?? TODO time and duration is mode dependent!
}

void CAPModellingModeBase::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	assert(!base_.empty());
	//remove Cmiss_node from the FE_region
	FE_region* fe_region = FE_node_get_FE_region(dataPointID); //REVISE
	fe_region = FE_region_get_data_FE_region(fe_region);
	FE_region_remove_FE_node(fe_region, dataPointID); // access = 1;
	base_.clear();
}

// CAPModellingModeRV

CAPModellingMode* CAPModellingModeRV::OnAccept()
{
	if (!(rvInserts_.size() % 2) || rvInserts_.empty())
	{
		std::cout << __func__ << "Need n pairs of rv insertion points" << std::endl;
		return 0;
	}
	return modeller_.GetModellingModeBasePlane();
}

void CAPModellingModeRV::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	rvInserts_.insert(std::pair<Cmiss_node* ,DataPoint>(dataPointID,dataPoint));
}

void CAPModellingModeRV::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	std::map<Cmiss_node*, DataPoint>::iterator itr = rvInserts_.find(dataPointID);
	assert(itr != rvInserts_.end());
	itr->second.SetCoordinate(coord);
}

void CAPModellingModeRV::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	std::map<Cmiss_node*, DataPoint>::iterator itr = rvInserts_.find(dataPointID);
	assert(itr != rvInserts_.end());
	rvInserts_.erase(itr);
}

// CAPModellingModeBasePlane

CAPModellingMode* CAPModellingModeBasePlane::OnAccept()
{
	
	return modeller_.GetModellingModeGuidePoints();
}

void CAPModellingModeBasePlane::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	
}

void CAPModellingModeBasePlane::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	
}

void CAPModellingModeBasePlane::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	
}

// CAPModellingModeGuidePoints

CAPModellingMode* CAPModellingModeGuidePoints::OnAccept()
{
	return this;
}

void CAPModellingModeGuidePoints::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	
}

void CAPModellingModeGuidePoints::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	
}

void CAPModellingModeGuidePoints::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	
}
