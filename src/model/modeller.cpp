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
#include "hexified/globalsmoothperframematrix.dat.h"
#include "hexified/globalmapbeziertohermite.dat.h"
#include "hexified/prior.dat.h"
#include "utils/debug.h"
#include "utils/filesystem.h"
#include "math/solverlibraryfactory.h"
#include "math/gmmfactory.h"
//#include "math/vnlfactory.h"
#include "math/algebra.h"
#include "math/geometry.h"
#include "math/basis.h"
#include "logmsg.h"

namespace cap
{

Modeller::Modeller(IModeller *mainApp)
	: mainApp_(mainApp)
	, modellingModeApex_()
	, modellingModeBase_()
	, modellingModeRV_()
	, modellingModeBasePlane_()
	, modellingModeGuidePoints_()
	, currentModellingMode_(&modellingModeApex_)
	, timeVaryingDataPoints_(134)
	, solverFactory_(new GMMFactory)
	//, solverFactory_(new VNLFactory)
	, timeSmoother_()
{
	SolverLibraryFactory& factory = *solverFactory_;

	dbg("Solver Library = " + factory.GetName());

	// Read in S (smoothness matrix)
	std::string tmpFileName = CreateTemporaryEmptyFile();
	WriteCharBufferToFile(tmpFileName, globalsmoothperframematrix_dat, globalsmoothperframematrix_dat_len);
	S_ = factory.CreateSparseMatrixFromFile(tmpFileName);
	RemoveFile(tmpFileName);
	// Read in G (global to local parameter map)
	tmpFileName = CreateTemporaryEmptyFile();
	WriteCharBufferToFile(tmpFileName, globalmapbeziertohermite_dat, globalmapbeziertohermite_dat_len);
	G_ = factory.CreateSparseMatrixFromFile(tmpFileName);
	RemoveFile(tmpFileName);

	// initialize preconditioner and GSMoothAMatrix
	preconditioner_ = factory.CreateDiagonalPreconditioner(*S_);

	aMatrix_ = factory.CreateGSmoothAMatrix(*S_, *G_);

	tmpFileName = CreateTemporaryEmptyFile();
	WriteCharBufferToFile(tmpFileName, prior_dat, prior_dat_len);
	prior_ = factory.CreateVectorFromFile(tmpFileName);
	RemoveFile(tmpFileName);
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

void Modeller::AddModellingPoint(Cmiss_region_id region, int node_id, Point3D const& position, double time)
{
	currentModellingMode_->AddModellingPoint(region, node_id, position, time);
	FitModel(time);
}

void Modeller::MoveModellingPoint(Cmiss_region_id /*region*/, int node_id, Point3D const& position, double time)
{
	currentModellingMode_->MoveModellingPoint(node_id, position, time);
	FitModel(time);
}

void Modeller::RemoveModellingPoint(Cmiss_region_id /*region*/, int node_id, double time)
{
	currentModellingMode_->RemoveModellingPoint(node_id, time);
	FitModel(time);
}

void Modeller::AttachToIfOn(int node_id, const std::string& label, const Point3D& location, const Vector3D& normal)
{
	currentModellingMode_->AttachToIfOn(node_id, label, location, normal);
}

bool Modeller::CanAccept(ModellingEnum mode) const
{
	// Run throught the modelling modes in reverse order.
	bool can = true;
	switch (mode)
	{
	case UNDEFINED_MODELLING_ENUM:
		return false;
	case GUIDEPOINT:
		can = can && modellingModeGuidePoints_.CanAccept();
	case BASEPLANE:
		can = can && modellingModeBasePlane_.CanAccept();
	case RV:
		can = can && modellingModeRV_.CanAccept();
	case BASE:
		can = can && modellingModeBase_.CanAccept();
	case APEX:
		can = can && modellingModeApex_.CanAccept();
	}

	return can;
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

void Modeller::AlignModel()
{
	if (CanAccept(BASEPLANE))
	{
		const ModellingPoints& apex = modellingModeApex_.GetModellingPoints();
		const ModellingPoints& base = modellingModeBase_.GetModellingPoints();
		const ModellingPoints& rvInserts = modellingModeRV_.GetModellingPoints();
		const ModellingPoints& basePlanePoints = modellingModeBasePlane_.GetModellingPoints();

		Point3D apexPosition = apex[0].GetPosition();
		Point3D basePosition = base[0].GetPosition();
		Vector3D xAxis= apexPosition - basePosition;
		xAxis.Normalise();

		ModellingPoints::const_iterator itr = rvInserts.begin();
		ModellingPoints::const_iterator end = rvInserts.end();
		Point3D sum;
		for (;itr!=end;++itr)
		{
			sum += itr->GetPosition();
		}

		Point3D averageOfRVInserts = sum / rvInserts.size();

		Vector3D yAxis = averageOfRVInserts - basePosition;
		Vector3D zAxis = CrossProduct(xAxis,yAxis);
		zAxis.Normalise();
		yAxis.CrossProduct(zAxis,xAxis);
		LOG_MSG(LOGINFORMATION) << "Model x axis vector" << xAxis;
		LOG_MSG(LOGINFORMATION) << "Model y axis vector" << yAxis;
		LOG_MSG(LOGINFORMATION) << "Model z axis vector" << zAxis;
		dbg("Model coord x axis vector" + ToString(xAxis));
		dbg("Model coord y axis vector" + ToString(yAxis));
		dbg("Model coord z axis vector" + ToString(zAxis));

		// Compute the position of the model coord origin. (1/3 of the way from base to apex)
		Point3D origin = basePosition + (0.3333) * (apexPosition - basePosition);

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


		mainApp_->SetHeartModelTransformation(transform);
		//--heartModel_.SetLocalToGlobalTransformation(transform);

		// TODO properly Compute FocalLength
		double lengthFromApexToBase = (apexPosition - basePosition).Length();
		dbg("Modeller::AlignModel() : lengthFromApexToBase = " + ToString(lengthFromApexToBase));
		LOG_MSG(LOGINFORMATION) << "Length from Apex to Base = " << lengthFromApexToBase;

		//double focalLength = 0.9 * (2.0 * lengthFromApexToBase / (3.0 * cosh(1.0))); // FIX
		double focalLength = (apexPosition - origin).Length()  / cosh(1.0);
		dbg("Modeller::AlignModel() : new focal length = " + ToString(focalLength));
		LOG_MSG(LOGINFORMATION) << "Focal length = " << focalLength;
		mainApp_->SetHeartModelFocalLength(focalLength);
		//--heartModel_.SetFocalLength(focalLength);

		// Construct base planes from the base plane points
		int numberOfModelFrames = mainApp_->GetNumberOfHeartModelFrames();//--heartModel_.GetNumberOfModelFrames();
		double framePeriod = 1.0/numberOfModelFrames;

		std::map<double, Plane> planes; // value_type = (frame, plane) pair
		ModellingPoints::const_iterator itrSrc = basePlanePoints.begin();

		while (itrSrc != basePlanePoints.end())
		{
			//--dbg("bp points : " + ToString(itrSrc->GetTime()) + ", " + ToString(itrSrc->GetCoordinate()));
			double frameTime = itrSrc->GetTime();
			double timeOfNextFrame = frameTime + framePeriod;//--(double)(frameNumber+1)/numberOfModelFrames;
			std::vector<Point3D> pointsInFrame;
			for (; itrSrc!=basePlanePoints.end() && itrSrc->GetTime() < timeOfNextFrame; ++itrSrc)
			{
				pointsInFrame.push_back((*itrSrc).GetPosition());
			}

			// Fit plane to the points
			Plane plane = FitPlaneThroughPoints(pointsInFrame, xAxis);
			planes.insert(std::make_pair(frameTime, plane));
		}

		// Set initial model parameters lambda, mu and theta
		// initial values for lambda come from the prior
		// theta is 1/4pi apart)
		// mu is equally spaced up to the base plane
		std::map<double, Plane>::const_iterator cit = planes.begin();
		for (; cit != planes.end(); cit++)
		{
			dbg("planes map : " + ToString(cit->first) + ", " + ToString(cit->second.position));
		}

		for(int i = 0; i < numberOfModelFrames; i++)
		{
			double time = i*framePeriod;
			//-- This call could be unnecessary as the nodes are already spaced out at regular intervals around the circle
			//--heartModel_.SetTheta(i);
			const Plane& plane = InterpolatePlanes(planes, time);

	//		std::cout << "Frame ( "<< i << ") normal = " << plane.normal << ", pos = " << plane.position << std::endl;
			//--heartModel_.SetMuFromBasePlaneForFrame(plane, time);
			mainApp_->SetHeartModelMuFromBasePlaneAtTime(plane, time);
			//heartModel_.SetLambdaForFrame(lambdaParams, i); // done in UpdateTimeVaryingModel
		}

		InitialiseBezierLambdaParams();

		// Need to fit the model across all times for the situation where the user has gone back and modified
		// any of the alignment modelling points and already has some guide points.
		if (modellingModeGuidePoints_.GetModellingPoints().size() > 0)
		{
			int numFrames = mainApp_->GetNumberOfHeartModelFrames();
			for(int i = 0; i < numFrames;i++)
			{
				double time = static_cast<double>(i)/numFrames;
				FitModel(time);
			}
		}
	}
}

void Modeller::UpdateTimeVaryingModel() //REVISE
{
	int numFrames = mainApp_->GetNumberOfHeartModelFrames();
	for(int j=0; j < numFrames;j++)
	{
		double time = static_cast<double>(j)/numFrames;
		Vector* x = solverFactory_->CreateVector(134);
		for (int i=0; i< 134; i++)
		{
			(*x)[i] = timeVaryingDataPoints_[i][j];
		}

		const std::vector<double>& hermiteLambdaParams = ConvertToHermite(*x);
		mainApp_->SetHeartModelLambdaParamsAtTime(hermiteLambdaParams, time);
		delete x;
	}
}

void Modeller::SmoothAlongTime()
{
	//--ModellingModeGuidePoints* gpMode = dynamic_cast<ModellingModeGuidePoints*>(currentModellingMode_); //REVISE
	if (CanAccept(BASEPLANE))
	{
		// For each global parameter in the per frame model
//#define PRINT_SMOOTHING_TIME
#ifdef PRINT_SMOOTHING_TIME
		clock_t before = clock();
#endif

#define SMOOTH_ALONG_TIME
#ifdef SMOOTH_ALONG_TIME
		int numFrames = mainApp_->GetNumberOfHeartModelFrames();
		//--const std::vector< std::vector<double> >& timeVaryingDataPoints = modellingModeGuidePoints_.GetTimeVaryingDataPoints();
		const std::vector<int>& framesWithDataPoints = modellingModeGuidePoints_.GetFramesWithModellingPoints(numFrames);//--GetFramesWithDataPoints();
		for (int i=0; i < 134; i++) // FIX magic number
		{
	//		std::cout << "timeVaryingDataPoints_[i] = " << timeVaryingDataPoints_[i] << std::endl;
			const std::vector<double>& lambdas = timeSmoother_.FitModel(i, timeVaryingDataPoints_[i], framesWithDataPoints);

	//		std::cout << lambdas << std::endl;

			for(int j=0; j < numFrames/*--heartModel_.GetNumberOfModelFrames()*/;j++) //FIX duplicate code
			{
				double xi = static_cast<double>(j)/numFrames;//--(double)j/heartModel_.GetNumberOfModelFrames();
				double lambda = timeSmoother_.ComputeLambda(xi, lambdas);
				timeVaryingDataPoints_[i][j] = lambda;
			}
		}
#endif
#ifdef PRINT_SMOOTHING_TIME
		clock_t after = clock();
		dbg(solverFactory_->GetName() + " Smoothing time = " + ToString((after - before) / static_cast<double>(CLOCKS_PER_SEC)));
#endif
		// feed the results back to Cmgui
		UpdateTimeVaryingModel();
	}
}

void Modeller::ChangeMode(ModellingEnum mode)
{
	ModellingMode* newMode = 0;
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
		dbg("Modeller::ChangeMode: Error (Invalid mode)");
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

std::vector<ModellingPoint> Modeller::GetModellingPoints() const
{
	ModellingPoints modellingPoints;
	ModellingPoints::const_iterator cit;

	ModellingPoints apex = modellingModeApex_.GetModellingPoints();
	cit = apex.begin();
	for (; cit != apex.end(); ++cit)
		modellingPoints.push_back(*cit);

	ModellingPoints base = modellingModeBase_.GetModellingPoints();
	cit = base.begin();
	for (; cit != base.end(); ++cit)
		modellingPoints.push_back(*cit);

	ModellingPoints rvInserts = modellingModeRV_.GetModellingPoints();
	cit = rvInserts.begin();
	for (; cit != rvInserts.end(); ++cit)
		modellingPoints.push_back(*cit);

	ModellingPoints basePlanePts = modellingModeBasePlane_.GetModellingPoints();
	cit = basePlanePts.begin();
	for (; cit != basePlanePts.end(); ++cit)
		modellingPoints.push_back(*cit);

	ModellingPoints guidePts = modellingModeGuidePoints_.GetModellingPoints();
	cit = guidePts.begin();
	for (; cit != guidePts.end(); ++cit)
		modellingPoints.push_back(*cit);

	return modellingPoints;
}

bool Modeller::ImagePlaneMoved(const std::string& label, Vector3D diff)
{
	bool model_moved = false;
	model_moved = model_moved || modellingModeApex_.ImagePlaneMoved(label, diff);
	model_moved = model_moved || modellingModeBase_.ImagePlaneMoved(label, diff);
	model_moved = model_moved || modellingModeRV_.ImagePlaneMoved(label, diff);
	model_moved = model_moved || modellingModeBasePlane_.ImagePlaneMoved(label, diff);
	model_moved = model_moved || modellingModeGuidePoints_.ImagePlaneMoved(label, diff);
	if (model_moved)
	{
		AlignModel();
		SmoothAlongTime();
	}

	return model_moved;
}

void Modeller::InitialiseBezierLambdaParams()
{
	//Initialise bezier global params for each model
	int numFrames = mainApp_->GetNumberOfHeartModelFrames();
	for (int i=0; i<134;i++)
	{
		timeVaryingDataPoints_[i].resize(numFrames);
		const std::vector<double>& prior = timeSmoother_.GetPrior(i);
		for(int j = 0; j < numFrames;j++)
		{
			double xi = static_cast<double>(j)/numFrames;

			double lambda = timeSmoother_.ComputeLambda(xi, prior);
			timeVaryingDataPoints_[i][j] = lambda;
		}
	}
}

void Modeller::FitModel(double time)
{
//#define PRINT_FIT_TIMINGS  // Uncomment or define elsewhere to print fit times
#ifdef PRINT_FIT_TIMINGS
	clock_t beforeTotal = clock();
	dbgn("Fit Model times [");
#endif
	// 0. Get the modelling points for the current time
	ModellingPoints currentModellingPoints = modellingModeGuidePoints_.GetModellingPointsAtTime(time);

	if (currentModellingPoints.size() == 0)
		return;

	// Compute P
	// 1. find xi coords for each data point
	ModellingPoints::iterator itr = currentModellingPoints.begin();
	int numFrames = mainApp_->GetNumberOfHeartModelFrames();
	int frameNumber = time*numFrames + 0.5;
	std::vector<Point3D> xi_vector;
	std::vector<int> element_id_vector;
	// For rhs
	Vector* guidePointLambda = solverFactory_->CreateVector(currentModellingPoints.size());

	for (int i = 0; itr!=currentModellingPoints.end(); ++itr, ++i)
	{
		Point3D xi;
		int elem_id = mainApp_->ComputeHeartModelXi(itr->GetPosition(), time, xi);
		switch(itr->GetHeartSurfaceType())
		{
		case ENDOCARDIUM:
		{
			xi.z = 0.0f;
			break;
		}
		case EPICARDIUM:
		{
			xi.z = 1.0f;
			break;
		}
		case UNDEFINED_HEART_SURFACE_TYPE:
		{
			if (xi.z < 0.5)
			{
				xi.z = 0.0f; // projected on endocardium
				modellingModeGuidePoints_.SetHeartSurfaceType(itr->GetNodeIdentifier(), ENDOCARDIUM);
			}
			else
			{
				xi.z = 1.0f; // projected on epicardium
				modellingModeGuidePoints_.SetHeartSurfaceType(itr->GetNodeIdentifier(), EPICARDIUM);
			}
			break;
		}
		}

		xi_vector.push_back(xi);
		element_id_vector.push_back(elem_id - 1); // element id starts at 1!!

		Point3D modellingPointPS = mainApp_->ConvertToHeartModelProlateSpheriodalCoordinate(itr->GetNodeIdentifier(), itr->GetModellingPointTypeString());
		(*guidePointLambda)[i] = modellingPointPS.x; // x = lambda, y = mu, z = theta
	}

	// 2. evaluate basis at the xi coords
	double psi[32]; //FIX 32?
	std::vector<Entry> entries;
	BiCubicHermiteLinearBasis basis;
	std::vector<Point3D>::iterator itr_xi = xi_vector.begin();
	std::vector<Point3D>::const_iterator end_xi = xi_vector.end();

	for (int xiIndex = 0; itr_xi != end_xi; ++itr_xi, ++xiIndex)
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
	SparseMatrix* P = solverFactory_->CreateSparseMatrix(currentModellingPoints.size(), 512, entries); //FIX

	aMatrix_->UpdateData(*P);

	// Compute RHS - GtPt(dataLamba - priorLambda)

	Vector* lambda = G_->mult(*prior_);

	// p = P * lambda : prior at projected data points
	Vector* p = P->mult(*lambda);

	// guidePointLambda = dataPoints in the same order as P (* weight) TODO : implement weight!

	// guidePointLambda = guidePointLambda - p
	*guidePointLambda -= *p;
	// rhs = GtPt p
	Vector* temp = P->trans_mult(*guidePointLambda);
	Vector* rhs = G_->trans_mult(*temp);

	// Solve Normal equation
	const double tolerance = 1.0e-3;
	const int maximumIteration = 100;

	Vector* x = solverFactory_->CreateVector(134); //FIX magic number

#ifdef PRINT_FIT_TIMINGS
	clock_t before = clock();
#endif

	solverFactory_->CG(*aMatrix_, *x, *rhs, *preconditioner_, maximumIteration, tolerance);

#ifdef PRINT_FIT_TIMINGS
	clock_t after = clock();
	dbgn(" CG : " + ToString((after - before) / static_cast<double>(CLOCKS_PER_SEC)));
#endif
	*x += *prior_;

	const std::vector<double>& hermiteLambdaParams = ConvertToHermite(*x);
#ifdef PRINT_FIT_TIMINGS
	clock_t beforeZn = clock();
#endif
	mainApp_->SetHeartModelLambdaParamsAtTime(hermiteLambdaParams, time);

#ifdef PRINT_FIT_TIMINGS
	clock_t afterZn = clock();
	dbgn(", ZN : " + ToString((afterZn - beforeZn) / static_cast<double>(CLOCKS_PER_SEC)));
#endif

	UpdateTimeVaryingDataPoints(*x, frameNumber); //Bezier

	delete P;
	delete lambda;
	delete p;
	delete guidePointLambda;
	delete temp;
	delete rhs;
	delete x;

#ifdef PRINT_FIT_TIMINGS
	clock_t afterTotal = clock();
	dbg(", Tot : " + ToString((afterTotal - beforeTotal) / static_cast<double>(CLOCKS_PER_SEC)) + " ]");
#endif
}

void Modeller::UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber)
{
	// Update the (Bezier) parameters for the newly fitted frame
	// This is in turn used as data points for the time varying model in the smoothing step

	for (int i = 0; i < 134; i++)
	{
		timeVaryingDataPoints_[i][frameNumber] = x[i];
	}
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
