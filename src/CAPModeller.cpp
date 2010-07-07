/*
 * CAPModeller.cpp
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#include "CAPModeller.h"
#include "CAPModelLVPS4X4.h"

#include <iostream>
#include <assert.h>

#include <boost/bind.hpp>

namespace cap
{

CAPModeller::CAPModeller(CAPModelLVPS4X4& heartModel)
:
	modellingModeApex_(),
	modellingModeBase_(),
	modellingModeRV_(heartModel),
	modellingModeBasePlane_(heartModel),
	modellingModeGuidePoints_(heartModel),
	currentModellingMode_(&modellingModeApex_)
{	
}

void CAPModeller::AddDataPoint(Cmiss_node* dataPointID,  const Point3D& coord, double time)
{
	currentModellingMode_->AddDataPoint(dataPointID, coord, time);
}

void CAPModeller::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	currentModellingMode_->MoveDataPoint(dataPointID, coord, time);
}

void CAPModeller::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	currentModellingMode_->RemoveDataPoint(dataPointID, time);
}

bool CAPModeller::OnAccept()
{
	CAPModellingMode* newMode = currentModellingMode_->OnAccept(*this);
	if (newMode) 
	{
		ChangeMode(newMode);
		return true;
	}
	
	return false;
}

CAPModellingMode* CAPModeller::GetModellingModeApex()
{
	return &modellingModeApex_;
}

CAPModellingMode* CAPModeller::GetModellingModeBase()
{
	return &modellingModeBase_;
}

CAPModellingMode* CAPModeller::GetModellingModeRV()
{
	return &modellingModeRV_;
}

CAPModellingMode* CAPModeller::GetModellingModeBasePlane()
{
	return &modellingModeBasePlane_;
}

CAPModellingModeGuidePoints* CAPModeller::GetModellingModeGuidePoints()
{
	return &modellingModeGuidePoints_;
}

void CAPModeller::InitialiseModel()
{
	CAPModellingModeBasePlane* gpMode = dynamic_cast<CAPModellingModeBasePlane*>(currentModellingMode_); //REVISE
	if (gpMode)
	{
		const DataPoint& apex = modellingModeApex_.GetApex();
		const DataPoint& base = modellingModeBase_.GetBase();
		const std::map<Cmiss_node*, DataPoint>& rvInsert = modellingModeRV_.GetRVInsertPoints();
		const std::vector<DataPoint>& basePlanePoints = modellingModeBasePlane_.GetBasePlanePoints(); 
	
		modellingModeGuidePoints_.InitialiseModel(apex, base, rvInsert, basePlanePoints);
	}
	
	modellingModeGuidePoints_.InitialiseModelLambdaParams();
}

void CAPModeller::UpdateTimeVaryingModel() //REVISE
{
//	CAPModellingModeGuidePoints* gpMode = dynamic_cast<CAPModellingModeGuidePoints*>(currentModellingMode_); //REVISE
//	if (gpMode)
//	{
//		gpMode->UpdateTimeVaryingModel();
//	}
	modellingModeGuidePoints_.UpdateTimeVaryingModel();
}

void CAPModeller::SmoothAlongTime()
{
	CAPModellingModeGuidePoints* gpMode = dynamic_cast<CAPModellingModeGuidePoints*>(currentModellingMode_); //REVISE
	if (gpMode)
	{
		gpMode->SmoothAlongTime();
	}
}

void CAPModeller::ChangeMode(ModellingMode mode)
{
	CAPModellingMode* newMode;
	switch (mode)
	{
	case APEX:
		newMode = GetModellingModeApex();
		break;
	case BASE:
		newMode = GetModellingModeBase();
		break;
	case RV:
		newMode = GetModellingModeRV();
		break;
	case BASEPLANE:
		newMode = GetModellingModeBasePlane();
		break;
	case GUIDEPOINT:
		newMode = GetModellingModeGuidePoints();
		break;
	default :
		std::cout << __func__ << ": Error (Invalid mode)" << std::endl;
	}
	assert(newMode);
	ChangeMode(newMode);
}

void CAPModeller::ChangeMode(CAPModellingMode* newMode)
{
	currentModellingMode_->PerformExitAction();
	currentModellingMode_ = newMode;
	currentModellingMode_->PerformEntryAction();
}

std::vector<DataPoint> CAPModeller::GetDataPoints() const
{
	std::vector<DataPoint> dataPoints;
	
	typedef std::vector<DataPoint> Vector;
	Vector const& bps = modellingModeBasePlane_.GetBasePlanePoints();

	if (!bps.empty())
	{
		// This means the user has reached the guide points modelling stage
		// i.e the model has been initialised.

		dataPoints.push_back(modellingModeApex_.GetApex());
		dataPoints.push_back(modellingModeBase_.GetBase());

		typedef std::map<Cmiss_node*, DataPoint> Map;
		Map const& rvInsert = modellingModeRV_.GetRVInsertPoints();

		std::transform(rvInsert.begin(), rvInsert.end(), std::back_inserter(dataPoints),
				boost::bind(&Map::value_type::second, _1));

		std::copy(bps.begin(), bps.end(), std::back_inserter(dataPoints));
		Vector const& gps = modellingModeGuidePoints_.GetDataPoints();
		std::copy(gps.begin(), gps.end(), std::back_inserter(dataPoints));
	}
	else
	{
		// this means the model has not been initialised.
		// return an empty vector
		// TODO might make more sense to just return the data points that have been put on
		// even if the model has not been initialised (i.e guide point mode has not been reached)
		dataPoints.clear(); // this is actually redundant but left here for clarity
	}

	return dataPoints;
}

} // end namespace cap
