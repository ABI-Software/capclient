/*
 * ModellingMode.cpp
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#include <iostream>
#include <algorithm>
#include <assert.h>
#include <ctime>

// ModellingModeGuidePoints

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "capclientconfig.h"
#include "hexified/GlobalSmoothPerFrameMatrix.dat.h"
#include "hexified/GlobalMapBezierToHermite.dat.h"
#include "hexified/prior.dat.h"
#include "model/modellingmode.h"
#include "model/modeller.h"
#include "math/totalleastsquares.h"
#include "SolverLibraryFactory.h"
#include "GMMFactory.h"
#include "VNLFactory.h"
#include "CAPMath.h"
#include "heartmodel.h"
#include "CAPBasis.h"
#include "filesystem.h"
#include "utils/debug.h"

namespace
{
//const static char* Sfile = "Data/templates/GlobalSmoothPerFrameMatrix.dat";
//const static char* Gfile = "Data/templates/GlobalMapBezierToHermite.dat";
//const static char* priorFile = "Data/templates/prior.dat";
}

namespace cap
{

ModellingMode::ModellingMode()
{
}

ModellingMode::~ModellingMode() 
{
}

// ModellingModeApex

void ModellingModeApex::PerformEntryAction()
{
	std::vector<DataPoint>::iterator itr = apex_.begin();
	for (;itr != apex_.end(); ++itr)
	{
		itr->SetVisible(true);
	}
}

void ModellingModeApex::PerformExitAction()
{
	std::vector<DataPoint>::iterator itr = apex_.begin();
	for (;itr != apex_.end(); ++itr)
	{
		itr->SetVisible(false);
	}
}

ModellingMode* ModellingModeApex::OnAccept(Modeller& modeller)
{
	if (apex_.empty())
	{
		std::cout << __func__ << ": Apex not defined" << std::endl;
		return 0;
	}

	return modeller.GetModellingModeBase();
}

void ModellingModeApex::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	if (!apex_.empty())
	{
		apex_.clear();
	}

	DataPoint dataPoint(dataPointID, coord, APEX, time);
	dataPoint.SetValidPeriod(0,1); //REVISE
	dataPoint.SetVisible(true);
	
	apex_.push_back(dataPoint);
}

void ModellingModeApex::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	assert(apex_.size() == 1);
	assert(apex_[0].GetCmissNode() == dataPointID);
	apex_[0].SetCoordinate(coord);
}

void ModellingModeApex::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	assert(!apex_.empty());
	apex_.clear();
}

const DataPoint& ModellingModeApex::GetApex() const
{
	assert(apex_.size() == 1);
	return apex_[0];
}

// ModellingModeBase

void ModellingModeBase::PerformEntryAction()
{
	std::vector<DataPoint>::iterator itr = base_.begin();
	for (;itr != base_.end(); ++itr)
	{
		itr->SetVisible(true);
	}
}

void ModellingModeBase::PerformExitAction()
{
	std::vector<DataPoint>::iterator itr = base_.begin();
	for (;itr != base_.end(); ++itr)
	{
		itr->SetVisible(false);
	}
}

ModellingMode* ModellingModeBase::OnAccept(Modeller& modeller)
{
	if (base_.empty())
	{
		std::cout << __func__ << ": Base not defined" << std::endl;
		return 0;
	}
	return modeller.GetModellingModeRV();
}

void ModellingModeBase::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	if (!base_.empty())
	{
		base_.clear();
	}

	DataPoint dataPoint(dataPointID, coord, BASE, time);
	dataPoint.SetValidPeriod(0,1); //REVISE
	dataPoint.SetVisible(true);
	base_.push_back(dataPoint);
}

void ModellingModeBase::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	assert(base_.size() == 1);
	assert(base_[0].GetCmissNode() == dataPointID);
	base_[0].SetCoordinate(coord);
}

void ModellingModeBase::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	assert(!base_.empty());
	base_.clear();
}

const DataPoint& ModellingModeBase::GetBase() const
{
	assert(base_.size()==1);
	return base_[0];
}

// ModellingModeRV

void ModellingModeRV::PerformEntryAction()
{
	std::map<Cmiss_node*, DataPoint>::iterator itr = rvInserts_.begin();
	for (;itr != rvInserts_.end(); ++itr)
	{
		itr->second.SetVisible(true);
	}
}

void ModellingModeRV::PerformExitAction()
{
	std::map<Cmiss_node*, DataPoint>::iterator itr = rvInserts_.begin();
	for (;itr != rvInserts_.end(); ++itr)
	{
		itr->second.SetVisible(false);
	}
}

ModellingMode* ModellingModeRV::OnAccept(Modeller& modeller)
{
	if ((rvInserts_.size() % 2) || rvInserts_.empty())
	{
		std::cout << __func__ << ": Need n pairs of rv insertion points" << std::endl;
		return 0;
	}
	return modeller.GetModellingModeBasePlane();
}

void ModellingModeRV::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	DataPoint dataPoint(dataPointID, coord, RV, time);
	//double startTime = heartModel_.MapToModelFrameTime(time);
	//double duration = (double)1.0f / heartModel_.GetNumberOfModelFrames();
	//double endTime = startTime + duration;
	
	//std::cout << __func__ << ": time = " << time << ", startTime = " << startTime << ", endTime = " << endTime << std::endl;
	//dataPoint.SetValidPeriod(startTime,endTime); //REVISE
	dataPoint.SetVisible(true);
	rvInserts_.insert(std::pair<Cmiss_node* ,DataPoint>(dataPointID,dataPoint));
}

void ModellingModeRV::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	std::map<Cmiss_node*, DataPoint>::iterator itr = rvInserts_.find(dataPointID);
	assert(itr != rvInserts_.end());
	itr->second.SetCoordinate(coord);
}

void ModellingModeRV::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	std::map<Cmiss_node*, DataPoint>::iterator itr = rvInserts_.find(dataPointID);
	assert(itr != rvInserts_.end());
	rvInserts_.erase(itr);
}

const std::map<Cmiss_node*, DataPoint>& ModellingModeRV::GetRVInsertPoints() const
{
	return rvInserts_;
}

// ModellingModeBasePlane

void ModellingModeBasePlane::PerformEntryAction()
{
	std::vector<DataPoint>::iterator itr = basePlanePoints_.begin();
	for (;itr != basePlanePoints_.end(); ++itr)
	{
		itr->SetVisible(true);
	}
}

void ModellingModeBasePlane::PerformExitAction()
{
	std::vector<DataPoint>::iterator itr = basePlanePoints_.begin();
	for (;itr != basePlanePoints_.end(); ++itr)
	{
		itr->SetVisible(false);
	}
}

ModellingMode* ModellingModeBasePlane::OnAccept(Modeller& modeller)
{
	if ((basePlanePoints_.size() % 2) || basePlanePoints_.empty())
	{
		std::cout << __func__ << ": Need n pairs of base plane points" << std::endl;
		return 0;
	}
	
	DataPointTimeLessThan lessThan; // need lambda functions !
	std::sort(basePlanePoints_.begin(), basePlanePoints_.end(), lessThan);
	
	modeller.AlignModel();
	modeller.UpdateTimeVaryingModel();
	return modeller.GetModellingModeGuidePoints();
}

void ModellingModeBasePlane::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	DataPoint dataPoint(dataPointID, coord, BASEPLANE, time);
	//double startTime = heartModel_.MapToModelFrameTime(time);
	//double duration = (double)1.0f / heartModel_.GetNumberOfModelFrames();
	//double endTime = startTime + duration;
	//dataPoint.SetValidPeriod(startTime,endTime); //REVISE
	dataPoint.SetVisible(true);
	basePlanePoints_.push_back(dataPoint);
}

void ModellingModeBasePlane::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	DataPointCmissNodeEqualTo equalTo(dataPointID);
	std::vector<DataPoint>::iterator itr = std::find_if(basePlanePoints_.begin(), basePlanePoints_.end(), equalTo);
	assert(itr != basePlanePoints_.end());
	
	itr->SetCoordinate(coord);
}

void ModellingModeBasePlane::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	DataPointCmissNodeEqualTo equalTo(dataPointID);
	std::vector<DataPoint>::iterator itr = std::find_if(basePlanePoints_.begin(), basePlanePoints_.end(), equalTo);
	assert(itr != basePlanePoints_.end());
	std::cout << "Removing BP : " << itr->GetSliceName() << ", time = " << itr->GetTime() << '\n';//debug
	basePlanePoints_.erase(itr);
}

const std::vector<DataPoint>& ModellingModeBasePlane::GetBasePlanePoints() const
{
	return basePlanePoints_;
}

// ModellingModeGuidePoints

ModellingModeGuidePoints::ModellingModeGuidePoints()
	: solverFactory_(new GMMFactory)
	//, solverFactory_(new VNLFactory)
	, timeVaryingDataPoints_(134)
	, timeSmoother_()
{
	SolverLibraryFactory& factory = *solverFactory_;
	
	dbg("Solver Library = " + factory.GetName());

	// Read in S (smoothness matrix)
	std::string tmpFileName = FileSystem::CreateTemporaryEmptyFile();
	FileSystem::WriteCharBufferToFile(tmpFileName, GlobalSmoothPerFrameMatrix_dat, GlobalSmoothPerFrameMatrix_dat_len);
	S_ = factory.CreateSparseMatrixFromFile(tmpFileName);
	FileSystem::RemoveFile(tmpFileName);
	// Read in G (global to local parameter map)
	tmpFileName = FileSystem::CreateTemporaryEmptyFile();
	FileSystem::WriteCharBufferToFile(tmpFileName, GlobalMapBezierToHermite_dat, GlobalMapBezierToHermite_dat_len);
	G_ = factory.CreateSparseMatrixFromFile(tmpFileName);
	FileSystem::RemoveFile(tmpFileName);

	dbg("Done reading S & G matrices");
	
	// initialize preconditioner and GSMoothAMatrix
	
	preconditioner_ = factory.CreateDiagonalPreconditioner(*S_);
	
	aMatrix_ = factory.CreateGSmoothAMatrix(*S_, *G_);
	dbg("Done creating GSmoothAMatrix");
	
	tmpFileName = FileSystem::CreateTemporaryEmptyFile();
	FileSystem::WriteCharBufferToFile(tmpFileName, prior_dat, prior_dat_len);
	prior_ = factory.CreateVectorFromFile(tmpFileName);
	FileSystem::RemoveFile(tmpFileName);
}

ModellingModeGuidePoints::~ModellingModeGuidePoints()
{
	delete aMatrix_;
	delete preconditioner_;
	//delete P_;
	delete S_;
	delete G_;
	delete prior_;
	delete solverFactory_;
}

void ModellingModeGuidePoints::PerformEntryAction()
{
	std::vector<DataPoints>::iterator vectorIter = vectorOfDataPoints_.begin();
	for (;vectorIter != vectorOfDataPoints_.end(); ++vectorIter)
	{
		std::map<Cmiss_node*, DataPoint>::iterator itr = vectorIter->begin();
		for (;itr != vectorIter->end(); ++itr)
		{
			itr->second.SetVisible(true);
		}
	}
}

void ModellingModeGuidePoints::PerformExitAction()
{
	std::vector<DataPoints>::iterator vectorIter = vectorOfDataPoints_.begin();
	for (;vectorIter != vectorOfDataPoints_.end(); ++vectorIter)
	{
		std::map<Cmiss_node*, DataPoint>::iterator itr = vectorIter->begin();
		for (;itr != vectorIter->end(); ++itr)
		{
			itr->second.SetVisible(false);
		}
	}
}

ModellingMode* ModellingModeGuidePoints::OnAccept(Modeller& modeller)
{
	return 0;
}

void ModellingModeGuidePoints::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
#if defined(NDEBUG)
	std::cout << "NDEBUG" << std::endl;
#endif
	
	DataPoint dataPoint(dataPointID, coord, GUIDEPOINT, time);
	//double startTime = heartModel_.MapToModelFrameTime(time);
	//double duration = (double)1.0f / heartModel_.GetNumberOfModelFrames();
	//double endTime = startTime + duration;
	//dataPoint.SetValidPeriod(startTime,endTime); //REVISE
	dataPoint.SetVisible(true);
		
	int frameNumber = 0;//heartModel_.MapToModelFrameNumber(dataPoint.GetTime());
	
	dbg(std::string("**** Change me **** ModellingModeGuidePoints::AddDataPoint time = ") + toString(dataPoint.GetTime()) + ", frame number = " + toString(frameNumber));
	
	vectorOfDataPoints_[frameNumber].insert(std::pair<Cmiss_node* ,DataPoint>(dataPointID,dataPoint));
	framesWithDataPoints_[frameNumber]++;
	
//	Point3D xi;
//	int elementNumber = heartModel_.ComputeXi(dataPoint->GetCoordinate(), xi);
	FitModel(vectorOfDataPoints_[frameNumber], frameNumber);
//	
//	std::vector<double> test(160);
//	
//	heartModel_.SetLambda(test); //Test
}

void ModellingModeGuidePoints::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	dbg("**** FIX, move to modeller class ****");
	//int frameNumber = heartModel_.MapToModelFrameNumber(time);
	//DataPoints::iterator itr = vectorOfDataPoints_[frameNumber].find(dataPointID);
	//assert(itr != vectorOfDataPoints_[frameNumber].end());
	//itr->second.SetCoordinate(coord);
	//FitModel(vectorOfDataPoints_[frameNumber], frameNumber);
}

void ModellingModeGuidePoints::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	dbg("**** FIX, move to modeller class ****");
	//int frameNumber = heartModel_.MapToModelFrameNumber(time);
	//DataPoints::iterator itr = vectorOfDataPoints_[frameNumber].find(dataPointID);
	//assert(itr != vectorOfDataPoints_[frameNumber].end());
	//vectorOfDataPoints_[frameNumber].erase(itr);
	//framesWithDataPoints_[frameNumber]--;
	//FitModel(vectorOfDataPoints_[frameNumber], frameNumber);
}

void ModellingModeGuidePoints::FitModel(DataPoints& dataPoints, int frameNumber)
{
	dbg("**** FIX, move to modeller class ****");
	// Compute P 
	// 1. find xi coords for each data point
	DataPoints::iterator itr = dataPoints.begin();
	DataPoints::const_iterator end = dataPoints.end();
	std::vector<Point3D> xi_vector;
	std::vector<int> element_id_vector;
	Vector* dataLambda = solverFactory_->CreateVector(dataPoints.size()); // for rhs

	for (int i = 0; itr!=end; ++itr, ++i)
	{
		Point3D xi;
		int elem_id = 0;//--heartModel_.ComputeXi(itr->second.GetCoordinate(), xi, (double)frameNumber/heartModel_.GetNumberOfModelFrames());
	//	if(!itr->second.GetSurfaceType())
		{
			if (xi.z < 0.5)
			{
				xi.z = 0.0f; // projected on endocardium
				itr->second.SetSurfaceType(ENDOCARDIUM);
			}
			else
			{
				xi.z = 1.0f; // projected on epicardium
				itr->second.SetSurfaceType(EPICARDIUM);
			}
		}
		xi_vector.push_back(xi);
		element_id_vector.push_back(elem_id - 1); // element id starts at 1!!
		
		Point3D dataPointLocal;//-- = heartModel_.TransformToLocalCoordinateRC(itr->second.GetCoordinate());
		Point3D dataPointPS;//-- = heartModel_.TransformToProlateSpheroidal(dataPointLocal);
		(*dataLambda)[i] = dataPointPS.x; // x = lambda, y = mu, z = theta 
	}
	
	//debug
#ifndef NDEBUG
	std::cout << "dataLambda = " << *dataLambda << std::endl;
#endif
	
	// 2. evaluate basis at the xi coords
	//    use this function as a temporary soln until Cmgui supports this
	double psi[32]; //FIX 32?
	std::vector<Entry> entries;
	CAPBiCubicHermiteLinearBasis basis;
	std::vector<Point3D>::iterator itr_xi = xi_vector.begin();
	std::vector<Point3D>::const_iterator end_xi = xi_vector.end();

	for (int xiIndex = 0; itr_xi!=end_xi; ++itr_xi, ++xiIndex)
	{
		double temp[3];
		temp[0] = itr_xi->x;
		temp[1] = itr_xi->y;
		temp[2] = itr_xi->z;
		basis.Evaluate(psi, temp);
		
		for (int nodalValueIndex = 0; nodalValueIndex < 32; nodalValueIndex++)
		{
			Entry e;
			e.value = psi[nodalValueIndex];
			e.colIndex = 32*(element_id_vector[xiIndex])+nodalValueIndex;
			e.rowIndex = xiIndex;
			entries.push_back(e);
		}
	}
	
	// 3. construct P
	SparseMatrix* P = solverFactory_->CreateSparseMatrix(dataPoints.size(), 512, entries); //FIX
	
	aMatrix_->UpdateData(*P);
	
	// Compute RHS - GtPt(dataLamba - priorLambda)

//	std::cout << "prior_ = " << *prior_ << endl;
	Vector* lambda = G_->mult(*prior_);
	//std::cout << "lambda = " << *lambda << endl;
	
	// p = P * lambda : prior at projected data points
	Vector* p = P->mult(*lambda);
//	std::cout << "p = " << *p << endl;
	
	// transform to local --> one above
	// transform to PS --> done above
	// dataLambda = dataPoints in the same order as P (* weight) TODO : implement weight!
	
	// dataLambda = dataLambda - p
	*dataLambda -= *p;
	// rhs = GtPt p
	Vector* temp = P->trans_mult(*dataLambda);
	Vector* rhs = G_->trans_mult(*temp);
	
	// Solve Normal equation
	const double tolerance = 1.0e-3;
	const int maximumIteration = 100;
	
	Vector* x = solverFactory_->CreateVector(134); //FIX magic number
	
	clock_t before = clock();
	
	solverFactory_->CG(*aMatrix_, *x, *rhs, *preconditioner_, maximumIteration, tolerance);

	clock_t after = clock();
	std::cout << solverFactory_->GetName() << " CG time = " << (after - before) << std::endl;
	std::cout << "Frame number = " << frameNumber << std::endl;

	        
	*x += *prior_;
//	std::cout << "x = " << *x << std::endl;
//	std::cout << "prior_ = " << *prior_ << endl;
	
	const std::vector<double>& hermiteLambdaParams = ConvertToHermite(*x);
	
	// Model should have the notion of frames
//	heartModel_.SetLambda(hermiteLambdaParams);
#define UPDATE_CMGUI
#ifdef UPDATE_CMGUI
	//--heartModel_.SetLambdaForFrame(hermiteLambdaParams, frameNumber); //Hermite
	
	UpdateTimeVaryingDataPoints(*x, frameNumber); //Bezier
#endif
//	SmoothAlongTime();
	
	delete P;
	delete lambda;
	delete p;
	delete dataLambda;
	delete temp;
	delete rhs;
	delete x;
	
	return;
}

void ModellingModeGuidePoints::SmoothAlongTime()
{
	// For each global parameter in the per frame model
	dbg("**** FIX ME, move to modeller class ****");
	clock_t before = clock();
		
#define SMOOTH_ALONG_TIME
#ifdef SMOOTH_ALONG_TIME
	for (int i=0; i < 134; i++) // FIX magic number
	{
//		std::cout << "timeVaryingDataPoints_[i] = " << timeVaryingDataPoints_[i] << std::endl;
		const std::vector<double>& lambdas = timeSmoother_.FitModel(i, timeVaryingDataPoints_[i], framesWithDataPoints_);
		
//		std::cout << lambdas << std::endl;
		
		for(int j=0; j<1/*--heartModel_.GetNumberOfModelFrames()*/;j++) //FIX duplicate code
		{
			double xi = 0;//--(double)j/heartModel_.GetNumberOfModelFrames();
			double lambda = timeSmoother_.ComputeLambda(xi, lambdas);
			timeVaryingDataPoints_[i][j] = lambda;
		}
	}
#endif
	
	clock_t after = clock();
	std::cout << solverFactory_->GetName() << " Smoothing time = " << (after - before) << std::endl;
	
	// feed the results back to Cmgui
	UpdateTimeVaryingModel();
}

void ModellingModeGuidePoints::UpdateTimeVaryingModel()
{
	for(int j=0; j<1/*--heartModel_.GetNumberOfModelFrames()*/;j++)
	{
		double time = 0.0;//--(double)j/heartModel_.GetNumberOfModelFrames();
		Vector* x = solverFactory_->CreateVector(134);
		for (int i=0; i< 134; i++)
		{
			(*x)[i] = timeVaryingDataPoints_[i][j];
		}
//		std::cout << "x(" << j << ")" << *x << std::endl;
		
		const std::vector<double>& hermiteLambdaParams = ConvertToHermite(*x);
		//--heartModel_.SetLambda(hermiteLambdaParams, time);
		delete x;
	}
}

void ModellingModeGuidePoints::UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber)
{
	// Update the (Bezier) parameters for the newly fitted frame
	// This is in turn used as data points for the time varying model in the smoothing step
	
	for (int i = 0; i < 134; i++)
	{
		timeVaryingDataPoints_[i][frameNumber] = x[i];
	}
}

std::vector<double> ModellingModeGuidePoints::ConvertToHermite(const Vector& bezierParams) const
{
	// convert Bezier params to hermite params to they can be fed to Cmgui
	// 
	//Vector* hermiteParams = (*bezierToHermiteTransform_).mult(bezierParams);
	// TODO REVISE inefficient
	Vector* hermiteParams = (*G_).mult(bezierParams);
	
	int indices[128] = {
		26,    25,    22,    21,     6,     5,     2,     1,
		27,    26,    23,    22,     7,     6,     3,     2,
		28,    27,    24,    23,     8,     7,     4,     3,
		25,    28,    21,    24,     5,     8,     1,     4,
		30,    29,    26,    25,    10,     9,     6,     5,
		31,    30,    27,    26,    11,    10,     7,     6,
		32,    31,    28,    27,    12,    11,     8,     7,
		29,    32,    25,    28,     9,    12,     5,     8,
		34,    33,    30,    29,    14,    13,    10,     9,
		35,    34,    31,    30,    15,    14,    11,    10,
		36,    35,    32,    31,    16,    15,    12,    11,
		33,    36,    29,    32,    13,    16,     9,    12,
		38,    37,    34,    33,    18,    17,    14,    13,
		39,    38,    35,    34,    19,    18,    15,    14,
		40,    39,    36,    35,    20,    19,    16,    15,
		37,    40,    33,    36,    17,    20,    13,    16
	};
	
	int invertedIndices[40];
		
	for (int i = 0; i <128; i++)
	{
		invertedIndices[indices[i]-1] = i;
	}
	
	std::vector<double> temp(160);
	
	for (int i =0; i < 40 ;i++)
	{
		temp[i*4] = (*hermiteParams)[invertedIndices[i]*4];
		temp[i*4+1] = (*hermiteParams)[invertedIndices[i]*4+1];
		temp[i*4+2] = (*hermiteParams)[invertedIndices[i]*4+2];
		temp[i*4+3] = (*hermiteParams)[invertedIndices[i]*4+3];
	}
//	for (int i = 0; i < 160 ; ++i)
//	{
//		temp[i] = (*hermiteParams)[i];
//	}
	
	delete hermiteParams;
	return temp;
}

Plane ModellingModeGuidePoints::InterpolateBasePlane(const std::map<int, Plane>& planes, int frame) const
{
	assert(!planes.empty());
	std::map<int, Plane>::const_iterator itr = planes.begin();
	
	
	int prevFrame = 0;
	Plane prevPlane;
	while (itr->first < frame && itr != planes.end())
	{
		prevFrame = itr->first;
		prevPlane = itr->second;
		itr++;
	}
	if (itr->first == frame) // Key frame, no interpolation needed
	{
		return itr->second;
	}
	
	// Handle edge cases where prevFrame > nextFrame (i.e interpolation occurs around the end point)
	int nextFrame;
	Plane nextPlane;
	int maxFrame = 1;//--heartModel_.GetNumberOfModelFrames();
	if (itr == planes.end())
	{
		nextFrame = planes.begin()->first + maxFrame;
		nextPlane = planes.begin()->second;
	}
	else 
	{
		nextFrame = itr->first;
		nextPlane = itr->second;
	}
	
	if (itr == planes.begin())
	{
		std::map<int, Plane>::const_reverse_iterator last = planes.rbegin();
		prevFrame = last->first - maxFrame;
		prevPlane = last->second;
	}
	
	Plane plane;
	double coefficient = (double)(frame - prevFrame)/(nextFrame - prevFrame);
	
	plane.normal = prevPlane.normal + coefficient * (nextPlane.normal - prevPlane.normal);
	
	plane.position = prevPlane.position + coefficient * (nextPlane.position - prevPlane.position);
	
	return plane;
}

void ModellingModeGuidePoints::InitialiseModel(
		const DataPoint& apex,
		const DataPoint& base,
		const std::map<Cmiss_node*, DataPoint>& rvInserts,
		const std::vector<DataPoint>& basePlanePoints)
{
	// Compute model coordinate axes from Apex, Base and RV insert points
	
	dbg("**** MOVE ME, to the modeller class ****");
	Vector3D xAxis= apex.GetCoordinate() - base.GetCoordinate();
	xAxis.Normalise();

	std::map<Cmiss_node*, DataPoint>::const_iterator itr = rvInserts.begin();
	std::map<Cmiss_node*, DataPoint>::const_iterator end = rvInserts.end();
	Point3D sum;
	for (;itr!=end;++itr)
	{
		sum += itr->second.GetCoordinate();
	}
	
	Point3D averageOfRVInserts = sum / rvInserts.size();
	
	Vector3D yAxis = averageOfRVInserts - base.GetCoordinate();
	Vector3D zAxis = CrossProduct(xAxis,yAxis);
	zAxis.Normalise();
	yAxis.CrossProduct(zAxis,xAxis);
	dbg("Model coord x axis vector" + toString(xAxis));
	std::cout << "Model coord y axis vector" << yAxis << std::endl;
	std::cout << "Model coord z axis vector" << zAxis << std::endl;
	
	// Compute the position of the model coord origin. (1/3 of the way from base to apex)
	Point3D origin = base.GetCoordinate() + (0.3333) * (apex.GetCoordinate() - base.GetCoordinate());
	
	// Transform heart model using the newly computed axes
	gtMatrix transform;
	transform[0][0]=static_cast<float>(xAxis.x);
	transform[0][1]=static_cast<float>(xAxis.y);
	transform[0][2]=static_cast<float>(xAxis.z);
	transform[0][3]=0; //NB this is the first column not row
	transform[1][0]=static_cast<float>(yAxis.x);
	transform[1][1]=static_cast<float>(yAxis.y);
	transform[1][2]=static_cast<float>(yAxis.z);
	transform[1][3]=0;
	transform[2][0]=static_cast<float>(zAxis.x);
	transform[2][1]=static_cast<float>(zAxis.y);
	transform[2][2]=static_cast<float>(zAxis.z);
	transform[2][3]=0;
	transform[3][0]=static_cast<float>(origin.x);
	transform[3][1]=static_cast<float>(origin.y);
	transform[3][2]=static_cast<float>(origin.z);
	transform[3][3]=1;
	
	//--heartModel_.SetLocalToGlobalTransformation(transform);
	
	// TODO properly Compute FocalLength
	double lengthFromApexToBase = (apex.GetCoordinate() - base.GetCoordinate()).Length();
	std::cout << __func__ << ": lengthFromApexToBase = " << lengthFromApexToBase << std::endl;
	
	//double focalLength = 0.9 * (2.0 * lengthFromApexToBase / (3.0 * cosh(1.0))); // FIX
	double focalLength = (apex.GetCoordinate() - origin).Length()  / cosh(1.0);
	std::cout << __func__ << ": new focal length = " << focalLength << std::endl;
	//--heartModel_.SetFocalLength(focalLength);
	
	// Construct base planes from the base plane points
	int numberOfModelFrames = 1;//--heartModel_.GetNumberOfModelFrames();
	
	std::map<int, Plane> planes; // value_type = (frame, plane) pair
	std::vector<DataPoint>::const_iterator itrSrc = basePlanePoints.begin();

	while ( itrSrc!=basePlanePoints.end())
	{
		int frameNumber = 1;//--heartModel_.MapToModelFrameNumber(itrSrc->GetTime());
		double timeOfNextFrame = (double)(frameNumber+1)/numberOfModelFrames;
		std::vector<DataPoint> basePlanePointsInOneFrame;
		for (; itrSrc!=basePlanePoints.end() && itrSrc->GetTime() < timeOfNextFrame; ++itrSrc)
		{
			basePlanePointsInOneFrame.push_back(*itrSrc);
		}
		// Fit plane to the points
		Plane plane = FitPlaneToBasePlanePoints(basePlanePointsInOneFrame, xAxis);
		planes.insert(std::make_pair(frameNumber, plane));
	}
	
	// Set initial model parameters lambda, mu and theta
	// initial values for lambda come from the prior
	// theta is 1/4pi apart)
	// mu is equally spaced up to the base plane
	
	for(int i = 0; i < numberOfModelFrames; i++)
	{
		//--heartModel_.SetTheta(i);
		const Plane& plane = InterpolateBasePlane(planes, i);
		
//		std::cout << "Frame ( "<< i << ") normal = " << plane.normal << ", pos = " << plane.position << std::endl; 
		//--heartModel_.SetMuFromBasePlaneForFrame(plane, i);
		//heartModel_.SetLambdaForFrame(lambdaParams, i); // done in UpdateTimeVaryingModel
	}
	
//	// REVISE
//	framesWithDataPoints_.clear();
//	framesWithDataPoints_.resize(numberOfModelFrames, 0);
}

void ModellingModeGuidePoints::InitialiseModelLambdaParams()
{
	dbg("**** MOVE ME, to the modeller class");
	//Initialise bezier global params for each model
	for (int i=0; i<134;i++)
	{
		int num = 0;//--heartModel_.GetNumberOfModelFrames();
		timeVaryingDataPoints_[i].resize(1/*--heartModel_.GetNumberOfModelFrames()*/);
		
//		std::cout << std::endl;
		for(int j = 0; j < 1/*--heartModel_.GetNumberOfModelFrames()*/;j++)
		{
			double xi = 0.0;//--(double)j/heartModel_.GetNumberOfModelFrames();
			const std::vector<double>& prior = timeSmoother_.GetPrior(i);
			double lambda = timeSmoother_.ComputeLambda(xi, prior);
//			std::cout << "(" << xi << ", " << lambda << ") ";
			timeVaryingDataPoints_[i][j] = lambda;
		}
//		std::cout << std::endl;
//		std::cout << "timeVaryingDataPoints_ : " << timeVaryingDataPoints_[i]  << std::endl;
	}
	
	vectorOfDataPoints_.clear();
	//--vectorOfDataPoints_.resize(heartModel_.GetNumberOfModelFrames());
	//--framesWithDataPoints_.assign(heartModel_.GetNumberOfModelFrames(), 0);
	
//#ifndef NDEBUG
//	std::cout << "vectorOfDataPoints_.size() = " << vectorOfDataPoints_.size() << '\n';
//	for (int i=0; i<vectorOfDataPoints_.size();i++)
//	{
//		std::cout << "vectorOfDataPoints_["<< i << "] : " << vectorOfDataPoints_[i].size() << '\n';
//	}
//#endif
}

std::vector<DataPoint> ModellingModeGuidePoints::GetDataPoints() const
{
	using boost::bind;
	std::vector<DataPoint> v;
	//for each map:
	typedef std::map<Cmiss_node*, DataPoint> Map;
	BOOST_FOREACH(Map const& map, vectorOfDataPoints_)
	{
		std::transform(map.begin(), map.end(), std::back_inserter(v), bind(&Map::value_type::second, _1));
	}
	return v;
}

} // end namespace cap
