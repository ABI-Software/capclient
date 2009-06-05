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

CAPModellingMode::CAPModellingMode() 
{
}

CAPModellingMode::~CAPModellingMode() 
{
}

// CAPModellingModeApex

CAPModellingMode* CAPModellingModeApex::OnAccept(CAPModeller& modeller)
{
	if (apex_.empty())
	{
		std::cout << __func__ << "Apex not defined" << std::endl;
		return 0;
	}
	// TODO Make the Cmiss_node representation of the apex point invisible
	//		which should be made visible when entering this state(mode)
	// provide EntryAction and ExitAction that can be invoked by the context (= CAPModeller)
	
	return modeller.GetModellingModeBase();
}

void CAPModellingModeApex::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	if (!apex_.empty())
	{
		//remove Cmiss_node from the FE_region
		Cmiss_node* oldApex = const_cast<Cmiss_node*>(apex_[0].GetCmissNode());
		assert(oldApex);
		FE_region* fe_region = FE_node_get_FE_region(oldApex); //REVISE
		assert(fe_region);
		fe_region = FE_region_get_data_FE_region(fe_region);
		assert(fe_region);
		FE_region_remove_FE_node(fe_region, oldApex); // access = 1;
		apex_.clear();
	}

	apex_.push_back(dataPoint);
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

const DataPoint& CAPModellingModeApex::GetApex() const
{
	assert(apex_.size() == 1);
	return apex_[0];
}

// CAPModellingModeBase

CAPModellingMode* CAPModellingModeBase::OnAccept(CAPModeller& modeller)
{
	if (base_.empty())
	{
		std::cout << __func__ << "Apex not defined" << std::endl;
		return 0;
	}
	return modeller.GetModellingModeRV();
}

void CAPModellingModeBase::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	if (!base_.empty())
	{
		//remove Cmiss_node from the FE_region
		Cmiss_node* oldBase = const_cast<Cmiss_node*>(base_[0].GetCmissNode());
		assert(oldBase);
		FE_region* fe_region = FE_node_get_FE_region(oldBase); //REVISE
		assert(fe_region);
		fe_region = FE_region_get_data_FE_region(fe_region);
		assert(fe_region);
		FE_region_remove_FE_node(fe_region, oldBase); // access = 1;
		base_.clear();
	}

	base_.push_back(dataPoint);
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

const DataPoint& CAPModellingModeBase::GetBase() const
{
	assert(base_.size()==1);
	return base_[0];
}

// CAPModellingModeRV

CAPModellingMode* CAPModellingModeRV::OnAccept(CAPModeller& modeller)
{
	if ((rvInserts_.size() % 2) || rvInserts_.empty())
	{
		std::cout << __func__ << "Need n pairs of rv insertion points" << std::endl;
		return 0;
	}
	return modeller.GetModellingModeBasePlane();
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

const std::map<Cmiss_node*, DataPoint>& CAPModellingModeRV::GetRVInsertPoints() const
{
	return rvInserts_;
}

// CAPModellingModeBasePlane

CAPModellingMode* CAPModellingModeBasePlane::OnAccept(CAPModeller& modeller)
{
	DataPointTimeLessThan lessThan; // need real lambda functions !
	std::sort(basePlanePoints_.begin(),basePlanePoints_.end(),lessThan);
	
	modeller.InitialiseModel();
	return modeller.GetModellingModeGuidePoints();
}

void CAPModellingModeBasePlane::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
	basePlanePoints_.push_back(dataPoint);
}

void CAPModellingModeBasePlane::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	DataPointCmissNodeEqualTo equalTo(dataPointID);
	std::vector<DataPoint>::iterator itr = std::find_if(basePlanePoints_.begin(), basePlanePoints_.end(), equalTo);
	assert(itr != basePlanePoints_.end());
	
	itr->SetCoordinate(coord);
}

void CAPModellingModeBasePlane::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	DataPointCmissNodeEqualTo equalTo(dataPointID);
	std::vector<DataPoint>::iterator itr = std::find_if(basePlanePoints_.begin(), basePlanePoints_.end(), equalTo);
	assert(itr != basePlanePoints_.end());
	
	basePlanePoints_.erase(itr);
}

const std::vector<DataPoint>& CAPModellingModeBasePlane::GetBasePlanePoints() const
{
	return basePlanePoints_;
}

// CAPModellingModeGuidePoints

#include "SolverLibraryFactory.h"
#include "GMMFactory.h"
#include "CAPMath.h"
#include "CAPModelLVPS4X4.h"

#include "CimBiCubicHermiteLinearBasis.h"

const static char* Sfile = "Data/templates/GlobalSmoothPerFrameMatrix.dat";
const static char* Gfile = "Data/templates/GlobalMapBezierToHermite.dat";
const static char* priorFile = "Data/templates/prior.dat";

CAPModellingModeGuidePoints::CAPModellingModeGuidePoints(CAPModelLVPS4X4& heartModel)
: 
	heartModel_(heartModel),
	solverFactory_(new GMMFactory),
	timeVaryingDataPoints_(134),
	timeSmoother_()
{
	SolverLibraryFactory& factory = *solverFactory_;
	
	std::cout << "Solver Library = " << factory.GetName() << std::endl;

	// Read in S (smoothness matrix)
	S_ = factory.CreateMatrixFromFile(Sfile);
	// Read in G (global to local parameter map)
	G_ = factory.CreateMatrixFromFile(Gfile);

	// initialize preconditioner and GSMoothAMatrix
	
	preconditioner_ = factory.CreateDiagonalPreconditioner(*S_);
	
	aMatrix_ = factory.CreateGSmoothAMatrix(*S_, *G_);
	
	prior_ = factory.CreateVectorFromFile(priorFile);
	return;
}

CAPModellingModeGuidePoints::~CAPModellingModeGuidePoints()
{
	delete aMatrix_;
	delete preconditioner_;
	delete P_;
	delete S_;
	delete G_;
	delete prior_;
	delete solverFactory_;
}

CAPModellingMode* CAPModellingModeGuidePoints::OnAccept(CAPModeller& modeller)
{
	return 0;
}

void CAPModellingModeGuidePoints::AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint)
{
#if defined(NDEBUG)
	std::cout << "NDEBUG" << std::endl;
#endif
	
	int frameNumber = heartModel_.MapToModelFrameNumber(dataPoint.GetTime());
	
#ifndef NDEBUG
	std::cout << "frame number = " << frameNumber << std::endl;
#endif
	
	vectorOfDataPoints_[frameNumber].insert(std::pair<Cmiss_node* ,DataPoint>(dataPointID,dataPoint));
//	Point3D xi;
//	int elementNumber = heartModel_.ComputeXi(dataPoint->GetCoordinate(), xi);
	FitModel(vectorOfDataPoints_[frameNumber], frameNumber);
//	
//	std::vector<float> test(160);
//	
//	heartModel_.SetLambda(test); //Test
}

void CAPModellingModeGuidePoints::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time)
{
	int frameNumber = heartModel_.MapToModelFrameNumber(time);
	DataPoints::iterator itr = vectorOfDataPoints_[frameNumber].find(dataPointID);
	assert(itr != vectorOfDataPoints_[frameNumber].end());
	itr->second.SetCoordinate(coord);
	FitModel(vectorOfDataPoints_[frameNumber], frameNumber);
}

void CAPModellingModeGuidePoints::RemoveDataPoint(Cmiss_node* dataPointID, float time)
{
	int frameNumber = heartModel_.MapToModelFrameNumber(time);
	DataPoints::iterator itr = vectorOfDataPoints_[frameNumber].find(dataPointID);
	assert(itr != vectorOfDataPoints_[frameNumber].end());
	vectorOfDataPoints_[frameNumber].erase(itr);
	FitModel(vectorOfDataPoints_[frameNumber], frameNumber);
}

#include <ctime>

void CAPModellingModeGuidePoints::FitModel(DataPoints& dataPoints, int frameNumber)
{		
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
		int elem_id = heartModel_.ComputeXi(itr->second.GetCoordinate(), xi, (float)frameNumber/heartModel_.GetNumberOfModelFrames());
	//	if(!itr->second.GetSurfaceType())
		{
			if (xi.z < 0.5)
			{
				xi.z = 0.0f; // projected on endocardium
				itr->second.SetSurfaceType(CAPModelLVPS4X4::ENDOCARDIUM);
			}
			else
			{
				xi.z = 1.0f; // projected on epicardium
				itr->second.SetSurfaceType(CAPModelLVPS4X4::EPICARDIUM);
			}
		}
		xi_vector.push_back(xi);
		element_id_vector.push_back(elem_id);
		
		const Point3D dataPointLocal = heartModel_.TransformToLocalCoordinateRC(itr->second.GetCoordinate());
		const Point3D dataPointPS = heartModel_.TransformToProlateSheroidal(dataPointLocal);
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
	CimBiCubicHermiteLinearBasis basis;
	std::vector<Point3D>::iterator itr_xi = xi_vector.begin();
	std::vector<Point3D>::const_iterator end_xi = xi_vector.end();

	for (int xiIndex = 0; itr_xi!=end_xi; ++itr_xi, ++xiIndex)
	{
		double temp[3];
		temp[0] = itr_xi->x;
		temp[1] = itr_xi->y;
		temp[2] = itr_xi->z;
		basis.evaluateBasis(psi, temp);
		
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
	Matrix* P = solverFactory_->CreateMatrix(dataPoints.size(), 512, entries); //FIX
	
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


	        
	*x += *prior_;
//	std::cout << "x = " << *x << std::endl;
//	std::cout << "prior_ = " << *prior_ << endl;
	
	const std::vector<float>& hermiteLambdaParams = ConvertToHermite(*x);
	
	// Model should have the notion of frames
//	heartModel_.SetLambda(hermiteLambdaParams);
#define UPDATE_CMGUI
#ifdef UPDATE_CMGUI
	heartModel_.SetLambdaForFrame(hermiteLambdaParams, frameNumber); //Hermite
	
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

void CAPModellingModeGuidePoints::SmoothAlongTime()
{
	// For each global parameter in the per frame model
	clock_t before = clock();
		
#define SMOOTH_ALONG_TIME
#ifdef SMOOTH_ALONG_TIME
	for (int i=0; i < 134; i++) // FIX magic number
	{
//		std::cout << "timeVaryingDataPoints_[i] = " << timeVaryingDataPoints_[i] << std::endl;
		const std::vector<double>& lambdas = timeSmoother_.FitModel(i, timeVaryingDataPoints_[i]);
		
//		std::cout << lambdas << std::endl;
		
		for(int j=0; j<heartModel_.GetNumberOfModelFrames();j++) //FIX duplicate code
		{
			float xi = (float)j/heartModel_.GetNumberOfModelFrames();
			float lambda = timeSmoother_.ComputeLambda(xi, lambdas);
			timeVaryingDataPoints_[i][j] = lambda;
		}
	}
#endif
	
	clock_t after = clock();
	std::cout << solverFactory_->GetName() << " Smoothing time = " << (after - before) << std::endl;
	
	// feed the results back to Cmgui
	UpdateTimeVaryingModel();
}

void CAPModellingModeGuidePoints::UpdateTimeVaryingModel()
{
	for(int j=0; j<heartModel_.GetNumberOfModelFrames();j++)
	{
		float time = (float)j/heartModel_.GetNumberOfModelFrames();
		Vector* x = solverFactory_->CreateVector(134);
		for (int i=0; i< 134; i++)
		{
			(*x)[i] = timeVaryingDataPoints_[i][j];
		}
//		std::cout << "x(" << j << ")" << *x << std::endl;
		
		const std::vector<float>& hermiteLambdaParams = ConvertToHermite(*x);
		heartModel_.SetLambda(hermiteLambdaParams, time);
		delete x;
	}
}

void CAPModellingModeGuidePoints::UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber)
{
	// Update the (Bezier) parameters for the newly fitted frame
	// This is in turn used as data points for the time varying model in the smoothing step
	
	for (int i = 0; i < 134; i++)
	{
		timeVaryingDataPoints_[i][frameNumber] = x[i];
	}
}

std::vector<float> CAPModellingModeGuidePoints::ConvertToHermite(const Vector& bezierParams)
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
	
	std::vector<float> temp(160);
	
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

Plane CAPModellingModeGuidePoints::InterpolateBasePlane(const std::map<int, Plane>& planes, int frame)
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
	int maxFrame = heartModel_.GetNumberOfModelFrames();
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
	float coefficient = (float)(frame - prevFrame)/(nextFrame - prevFrame);
	
	plane.normal = prevPlane.normal + coefficient * (nextPlane.normal - prevPlane.normal);
	
	plane.position = prevPlane.position + coefficient * (nextPlane.position - prevPlane.position);
	
	return plane;
}

Plane CAPModellingModeGuidePoints::FitPlaneToBasePlanePoints(const std::vector<DataPoint>& basePlanePoints, const Vector3D& xAxis)
{
	// TODO implement cases where there are more than 2 points 
	Vector3D temp1 = basePlanePoints[1].GetCoordinate() - basePlanePoints[0].GetCoordinate();
	temp1.Normalise();
	
	Vector3D temp2 = CrossProduct(temp1, xAxis);
	
	Plane plane;
	plane.normal = CrossProduct(temp1, temp2);
	plane.normal.Normalise();
	
	plane.position = basePlanePoints[0].GetCoordinate() + (0.5 * (basePlanePoints[1].GetCoordinate() - basePlanePoints[0].GetCoordinate()));
	
	// make sure plane normal is always pointing toward the apex
	if (DotProduct(plane.normal, xAxis) < 0)
	{
		plane.normal *= -1; 
	}
	return plane;
}

void CAPModellingModeGuidePoints::InitialiseModel(
		const DataPoint& apex,
		const DataPoint& base,
		const std::map<Cmiss_node*, DataPoint>& rvInserts,
		const std::vector<DataPoint>& basePlanePoints)
{
	// Compute model coordinate axes from Apex, Base and RV insert points

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
	std::cout << "Model coord x axis vector" << xAxis << std::endl;
	std::cout << "Model coord y axis vector" << yAxis << std::endl;
	std::cout << "Model coord z axis vector" << zAxis << std::endl;
	
	// Compute the position of the model coord origin. (1/3 of the way from base to apex)
	Point3D origin = base.GetCoordinate() + 1/3 * (base.GetCoordinate() - apex.GetCoordinate());
	
	// TODO properly Compute FocalLength
	float lengthFromApexToBase = (apex.GetCoordinate() - base.GetCoordinate()).Length();
	float focalLength = 0.9 * (2.0 * lengthFromApexToBase / (3.0 * cosh(1.0))); // FIX
	std::cout << __func__ << ": new focal length = " << focalLength << std::endl;
	heartModel_.SetFocalLengh(focalLength);
	
	// Construct base planes from the base plane points
	int numberOfModelFrames = heartModel_.GetNumberOfModelFrames();
	
	std::map<int, Plane> planes; // value_type = (frame, plane) pair
	std::vector<DataPoint>::const_iterator itrSrc = basePlanePoints.begin();

	while ( itrSrc!=basePlanePoints.end())
	{
		int frameNumber = heartModel_.MapToModelFrameNumber(itrSrc->GetTime());
		float timeOfNextFrame = (float)(frameNumber+1)/numberOfModelFrames;
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
		heartModel_.SetTheta(i);
		const Plane& plane = InterpolateBasePlane(planes, i);
		
		std::cout << "Frame ( "<< i << ") normal = " << plane.normal << ", pos = " << plane.position << std::endl; 
		heartModel_.SetMuFromBasePlaneForFrame(plane, i);
		//heartModel_.SetLambdaForFrame(lambdaParams, i); // done in UpdateTimeVaryingModel
	}
	
	return;
}

void CAPModellingModeGuidePoints::InitialiseModelLambdaParams()
{
	//Initialise bezier global params for each model
	for (int i=0; i<134;i++)
	{
		timeVaryingDataPoints_[i].resize(heartModel_.GetNumberOfModelFrames());
		
//		std::cout << std::endl;
		for(int j=0; j<heartModel_.GetNumberOfModelFrames();j++)
		{
			float xi = (float)j/heartModel_.GetNumberOfModelFrames();
			const std::vector<double>& prior = timeSmoother_.GetPrior(i);
			float lambda = timeSmoother_.ComputeLambda(xi, prior);
//			std::cout << "(" << xi << ", " << lambda << ") ";
			timeVaryingDataPoints_[i][j] = lambda;
		}
//		std::cout << std::endl;
//		std::cout << "timeVaryingDataPoints_ : " << timeVaryingDataPoints_[i]  << std::endl;
	}
	
	vectorOfDataPoints_.resize(heartModel_.GetNumberOfModelFrames());
	
//#ifndef NDEBUG
//	std::cout << "vectorOfDataPoints_.size() = " << vectorOfDataPoints_.size() << '\n';
//	for (int i=0; i<vectorOfDataPoints_.size();i++)
//	{
//		std::cout << "vectorOfDataPoints_["<< i << "] : " << vectorOfDataPoints_[i].size() << '\n';
//	}
//#endif
}
