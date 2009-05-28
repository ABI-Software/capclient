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
//	heartModel_(heartModel),
//	solverFactory_(new GMMFactory),
//	timeVaryingDataPoints_(134),
//	timeSmoother_(),
	modellingModeApex_(*this),
	modellingModeBase_(*this),
	modellingModeRV_(*this),
	modellingModeBasePlane_(*this),
	modellingModeGuidePoints_(*this, heartModel),
	currentModellingMode_(&modellingModeApex_)
{	
//	SolverLibraryFactory& factory = *solverFactory_;
//	
//	std::cout << "Solver Library = " << factory.GetName() << std::endl;
//
//	// Read in S (smoothness matrix)
//	S_ = factory.CreateMatrixFromFile(Sfile);
//	// Read in G (global to local parameter map)
//	G_ = factory.CreateMatrixFromFile(Gfile);
//
//	// initialize preconditioner and GSMoothAMatrix
//	
//	preconditioner_ = factory.CreateDiagonalPreconditioner(*S_);
//	
//	aMatrix_ = factory.CreateGSmoothAMatrix(*S_, *G_);
//	
//	prior_ = factory.CreateVectorFromFile(priorFile);
//	return;
}

//CAPModeller::~CAPModeller()
//{
//	delete aMatrix_;
//	delete preconditioner_;
//	delete P_;
//	delete S_;
//	delete G_;
//	delete prior_;
//	delete solverFactory_;
//}

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

void CAPModeller::OnAccept()
{
	CAPModellingMode* newMode = currentModellingMode_->OnAccept();
	if (newMode) 
	{
		currentModellingMode_ = currentModellingMode_->OnAccept();
	}
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
//	CAPModellingModeGuidePoints* gpMode = dynamic_cast<CAPModellingModeGuidePoints*>(currentModellingMode_); //REVISE
//	if (gpMode)
//	{
//		gpMode->InitialiseModel();
//	}
	modellingModeGuidePoints_.InitialiseModel();
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
