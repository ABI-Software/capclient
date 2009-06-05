/*
 * CAPModeller.cpp
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#include "CAPModeller.h"

//#include "SolverLibraryFactory.h"
//#include "GMMFactory.h"
//#include "CAPMath.h"
#include "CAPModelLVPS4X4.h"

//#include "CimBiCubicHermiteLinearBasis.h"

#include <iostream>

CAPModeller::CAPModeller(CAPModelLVPS4X4& heartModel)
:
	modellingModeApex_(),
	modellingModeBase_(),
	modellingModeRV_(),
	modellingModeBasePlane_(),
	modellingModeGuidePoints_(heartModel),
	currentModellingMode_(&modellingModeApex_)
{	
}

void CAPModeller::AddDataPoint(Cmiss_node* dataPointID,  const DataPoint& dataPoint)
{
	currentModellingMode_->AddDataPoint(dataPointID, dataPoint);
}

void CAPModeller::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	currentModellingMode_->MoveDataPoint(dataPointID, coord, time);
}

void CAPModeller::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	currentModellingMode_->RemoveDataPoint(dataPointID, time);
}

bool CAPModeller::OnAccept()
{
	CAPModellingMode* newMode = currentModellingMode_->OnAccept(*this);
	if (newMode) 
	{
		currentModellingMode_ = newMode;
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

void CAPModeller::UpdateTimeVaryingModel()
{
	CAPModellingModeGuidePoints* gpMode = dynamic_cast<CAPModellingModeGuidePoints*>(currentModellingMode_); //REVISE
	if (gpMode)
	{
		gpMode->UpdateTimeVaryingModel();
	}
}

void CAPModeller::SmoothAlongTime()
{
	CAPModellingModeGuidePoints* gpMode = dynamic_cast<CAPModellingModeGuidePoints*>(currentModellingMode_); //REVISE
	if (gpMode)
	{
		gpMode->SmoothAlongTime();
	}
}
