/*
 * Modeller.cpp
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#include "model/modeller.h"

#include <iostream>
#include <assert.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "capclientconfig.h"
#include "hexified/GlobalSmoothPerFrameMatrix.dat.h"
#include "hexified/GlobalMapBezierToHermite.dat.h"
#include "hexified/prior.dat.h"
#include "utils/debug.h"
#include "utils/filesystem.h"
#include "math/totalleastsquares.h"
#include "math/solverlibraryfactory.h"
#include "math/gmmfactory.h"
#include "math/vnlfactory.h"
#include "CAPMath.h"
#include "math/basis.h"

namespace cap
{

const Modeller::ModellingModeEnumMap Modeller::ModellingModeStrings = Modeller::InitModellingModeStrings();

Modeller::Modeller(CAPClient *mainApp)
	: mainApp_(mainApp)
	, modellingModeApex_()
	, modellingModeBase_()
	, modellingModeRV_()
	, modellingModeBasePlane_()
	, modellingModeGuidePoints_()
	, currentModellingMode_(&modellingModeApex_)
	, solverFactory_(new GMMFactory)
	//, solverFactory_(new VNLFactory)
	, timeSmoother_()
{
	SolverLibraryFactory& factory = *solverFactory_;
	
	dbg("Solver Library = " + factory.GetName());

	// Read in S (smoothness matrix)
	std::string tmpFileName = FileSystem::CreateTemporaryEmptyFile();
	FileSystem::WriteCharBufferToFile(tmpFileName, globalsmoothperframematrix_dat, globalsmoothperframematrix_dat_len);
	S_ = factory.CreateSparseMatrixFromFile(tmpFileName);
	FileSystem::RemoveFile(tmpFileName);
	// Read in G (global to local parameter map)
	tmpFileName = FileSystem::CreateTemporaryEmptyFile();
	FileSystem::WriteCharBufferToFile(tmpFileName, globalmapbeziertohermite_dat, globalmapbeziertohermite_dat_len);
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

Modeller::~Modeller()
{
	delete aMatrix_;
	delete preconditioner_;
	//delete P_;
	delete S_;
	delete G_;
	delete prior_;
	delete solverFactory_;
}

void Modeller::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	currentModellingMode_->AddDataPoint(dataPointID, coord, time);
}

void Modeller::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	currentModellingMode_->MoveDataPoint(dataPointID, coord, time);
}

void Modeller::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	currentModellingMode_->RemoveDataPoint(dataPointID, time);
}

bool Modeller::OnAccept()
{
	ModellingMode* newMode = currentModellingMode_->OnAccept(*this);
	if (newMode) 
	{
		ChangeMode(newMode);
		return true;
	}
	
	return false;
}

ModellingMode* Modeller::GetModellingModeApex()
{
	return &modellingModeApex_;
}

ModellingMode* Modeller::GetModellingModeBase()
{
	return &modellingModeBase_;
}

ModellingMode* Modeller::GetModellingModeRV()
{
	return &modellingModeRV_;
}

ModellingMode* Modeller::GetModellingModeBasePlane()
{
	return &modellingModeBasePlane_;
}

ModellingModeGuidePoints* Modeller::GetModellingModeGuidePoints()
{
	return &modellingModeGuidePoints_;
}

Plane Modeller::FitPlaneToBasePlanePoints(const std::vector<DataPoint>& basePlanePoints, const Vector3D& xAxis) const
{
	Plane plane;
	
	if (basePlanePoints.size() > 2)
	{
		// Total Least Squares using SVD
		std::vector<Point3D> vectorOfPoints;
		for (std::vector<DataPoint>::const_iterator i = basePlanePoints.begin();
				i != basePlanePoints.end(); ++i)
		{
			vectorOfPoints.push_back(i->GetCoordinate());
		}
		plane = FitPlaneUsingTLS(vectorOfPoints);
	}
	else
	{
		// When only 2 base plane points have been specified
		Vector3D temp1 = basePlanePoints[1].GetCoordinate() - basePlanePoints[0].GetCoordinate();
		temp1.Normalise();
		
		Vector3D temp2 = CrossProduct(temp1, xAxis);
		
		plane.normal = CrossProduct(temp1, temp2);
		plane.normal.Normalise();
		
		plane.position = basePlanePoints[0].GetCoordinate() + (0.5 * (basePlanePoints[1].GetCoordinate() - basePlanePoints[0].GetCoordinate()));
	}
	
	// make sure plane normal is always pointing toward the apex
	if (DotProduct(plane.normal, xAxis) < 0)
	{
		plane.normal *= -1; 
	}
	
	return plane;
}

void Modeller::AlignModel()
{
	if (GetCurrentMode() == BASEPLANE)
	{
		const DataPoint& apex = modellingModeApex_.GetApex();
		const DataPoint& base = modellingModeBase_.GetBase();
		const std::map<Cmiss_node*, DataPoint>& rvInserts = modellingModeRV_.GetRVInsertPoints();
		const std::vector<DataPoint>& basePlanePoints = modellingModeBasePlane_.GetBasePlanePoints(); 
	
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
		InitialiseModelLambdaParams();
	}
	
	
}

Plane Modeller::InterpolateBasePlane(const std::map<int, Plane>& planes, int frame) const
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

void Modeller::UpdateTimeVaryingModel() //REVISE
{
	if (GetCurrentMode() == GUIDEPOINT)
	{
		const std::vector< std::vector<double> >& timeVaryingDataPoints = modellingModeGuidePoints_.GetTimeVaryingDataPoints();
		for(int j=0; j<1/*--heartModel_.GetNumberOfModelFrames()*/;j++)
		{
			double time = 0.0;//--(double)j/heartModel_.GetNumberOfModelFrames();
			Vector* x = solverFactory_->CreateVector(134);
			for (int i=0; i< 134; i++)
			{
				(*x)[i] = timeVaryingDataPoints[i][j];
			}
	//		std::cout << "x(" << j << ")" << *x << std::endl;
			
			const std::vector<double>& hermiteLambdaParams = ConvertToHermite(*x);
			//--heartModel_.SetLambda(hermiteLambdaParams, time);
			delete x;
		}
	}
}

void Modeller::SmoothAlongTime()
{
	ModellingModeGuidePoints* gpMode = dynamic_cast<ModellingModeGuidePoints*>(currentModellingMode_); //REVISE
	if (GetCurrentMode() == GUIDEPOINT)
	{
		// For each global parameter in the per frame model
		clock_t before = clock();
			
#define SMOOTH_ALONG_TIME
#ifdef SMOOTH_ALONG_TIME
		const std::vector< std::vector<double> >& timeVaryingDataPoints = modellingModeGuidePoints_.GetTimeVaryingDataPoints();
		const std::vector<int>& framesWithDataPoints = modellingModeGuidePoints_.GetFramesWithDataPoints();
		for (int i=0; i < 134; i++) // FIX magic number
		{
	//		std::cout << "timeVaryingDataPoints_[i] = " << timeVaryingDataPoints_[i] << std::endl;
			const std::vector<double>& lambdas = timeSmoother_.FitModel(i, timeVaryingDataPoints[i], framesWithDataPoints);
			
	//		std::cout << lambdas << std::endl;
			
			for(int j=0; j<1/*--heartModel_.GetNumberOfModelFrames()*/;j++) //FIX duplicate code
			{
				double xi = 0;//--(double)j/heartModel_.GetNumberOfModelFrames();
				double lambda = timeSmoother_.ComputeLambda(xi, lambdas);
				//--timeVaryingDataPoints[i][j] = lambda;
			}
		}
#endif
		
		clock_t after = clock();
		dbg(solverFactory_->GetName() + " Smoothing time = " + toString(after - before));
		
		// feed the results back to Cmgui
		UpdateTimeVaryingModel();
	}
}

void Modeller::ChangeMode(ModellingModeEnum mode)
{
	ModellingMode* newMode;
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

void Modeller::ChangeMode(ModellingMode* newMode)
{
	currentModellingMode_->PerformExitAction();
	currentModellingMode_ = newMode;
	currentModellingMode_->PerformEntryAction();
}

std::vector<DataPoint> Modeller::GetDataPoints() const
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
		Vector const& gps = modellingModeGuidePoints_.GetGuidePoints();
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

void Modeller::SetDataPoints(std::vector<DataPoint>& dataPoints)
{
	if (dataPoints.empty()) //FIXME 
	{
		// This handles the case where no data points are defined
		// e.g model files converted from CIM models
		// FIXME - this does not work in cases where neither data points nor
		//         model files are defined in the xml file.
		InitialiseModelLambdaParams();
//		ChangeMode(GetModellingModeGuidePoints());
		return;
	}
	
	std::sort(dataPoints.begin(), dataPoints.end(),
			boost::bind( std::less<DataPointType>(),
					boost::bind(&DataPoint::GetDataPointType, _1),
					boost::bind(&DataPoint::GetDataPointType, _2)));

	currentModellingMode_ = GetModellingModeApex();
	ModellingModeEnum currentModeEnum = APEX;
	BOOST_FOREACH(DataPoint& dataPoint, dataPoints)
	{
		// type unsafe but much less verbose than switch cases
		ModellingModeEnum mode = static_cast<ModellingModeEnum>(dataPoint.GetDataPointType());
		if (mode != currentModeEnum)
		{
			// Change mode and call OnAccept on the currentModellingMode_
			OnAccept();
			currentModeEnum = mode;
		}
		AddDataPoint(dataPoint.GetCmissNode(), dataPoint.GetCoordinate(), dataPoint.GetTime());
	}
	if (currentModeEnum == BASEPLANE) // no guide points defined
	{
		OnAccept();
	}

	SmoothAlongTime();
//	std::cout << "Base is in " << modellingModeBase_.GetBase().GetSliceName() << '\n';
}

void Modeller::InitialiseModelLambdaParams()
{
	dbg("**** MOVE ME, to the modeller class");
	//Initialise bezier global params for each model
	for (int i=0; i<134;i++)
	{
		int num = 0;//--heartModel_.GetNumberOfModelFrames();
		//--timeVaryingDataPoints_[i].resize(1/*--heartModel_.GetNumberOfModelFrames()*/);
		
//		std::cout << std::endl;
		for(int j = 0; j < 1/*--heartModel_.GetNumberOfModelFrames()*/;j++)
		{
			double xi = 0.0;//--(double)j/heartModel_.GetNumberOfModelFrames();
			const std::vector<double>& prior = timeSmoother_.GetPrior(i);
			double lambda = timeSmoother_.ComputeLambda(xi, prior);
//			std::cout << "(" << xi << ", " << lambda << ") ";
			//--timeVaryingDataPoints_[i][j] = lambda;
		}
//		std::cout << std::endl;
//		std::cout << "timeVaryingDataPoints_ : " << timeVaryingDataPoints_[i]  << std::endl;
	}
	
	//--vectorOfDataPoints_.clear();
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

void Modeller::FitModel(DataPoints& dataPoints, int frameNumber)
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
	BiCubicHermiteLinearBasis basis;
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
	
	modellingModeGuidePoints_.UpdateTimeVaryingDataPoints(*x, frameNumber); //Bezier
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

std::vector<double> Modeller::ConvertToHermite(const Vector& bezierParams) const
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


} // end namespace cap
