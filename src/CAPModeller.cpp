/*
 * CAPModeller.cpp
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#include "CAPModeller.h"

#include "SolverLibraryFactory.h"
#include "GMMFactory.h"
#include "CAPMath.h"
#include "CAPModelLVPS4X4.h"

#include "CimBiCubicHermiteLinearBasis.h"

#include <iostream>

const static char* Sfile = "/Users/jchu014/Dev/Solver/Data/CimModelGlobalSmooth.dat_gpts";
const static char* Gfile = "/Users/jchu014/Dev/Solver/Data/CimModelLVPS4x4_0.map";
const static char* P9file = "/Users/jchu014/Dev/Solver/Data/PMatrix_test_frame_9.mat_noCR";
const static char* priorFile = "Data/templates/prior.dat";

CAPModeller::CAPModeller(CAPModelLVPS4X4& heartModel)
:
	heartModel_(heartModel),
	solverFactory_(new GMMFactory)
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

CAPModeller::~CAPModeller()
{
	delete aMatrix_;
	delete preconditioner_;
	delete P_;
	delete S_;
	delete G_;
	delete prior_;
}

void CAPModeller::FitModel()
{		
	// Compute P 
	// 1. find xi coords for each data point
	DataPoints::const_iterator itr = dataPoints_.begin();
	DataPoints::const_iterator end = dataPoints_.end();
	std::vector<Point3D> xi_vector;
	std::vector<int> element_id_vector;
	Vector* dataLambda = solverFactory_->CreateVector(dataPoints_.size()); // for rhs

	for (int i = 0; itr!=end; ++itr, ++i)
	{
		Point3D xi;
		int elem_id = heartModel_.ComputeXi(itr->GetCoordinate(), xi);
		xi_vector.push_back(xi);
		element_id_vector.push_back(elem_id);
		
		const Point3D dataPointLocal = heartModel_.TransformToLocalCoordinateRC(itr->GetCoordinate());
		const Point3D dataPointPS = heartModel_.TransformToProlateSheroidal(dataPointLocal);
		(*dataLambda)[i] = dataPointPS.x; // x = lambda, y = mu, z = theta 
	}
	
	//debug
	//std::cout << "dataLambda = " << *dataLambda << std::endl;
	
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
	Matrix* P = solverFactory_->CreateMatrix(dataPoints_.size(), 512, entries); //FIX
	
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
	solverFactory_->CG(*aMatrix_, *x, *rhs, *preconditioner_, maximumIteration, tolerance);

	*x += *prior_;
//	std::cout << "x = " << *x << std::endl;
//	std::cout << "prior_ = " << *prior_ << endl;
	
	const std::vector<float>& hermiteLambdaParams = ConvertToHermite(*x);
	
	// Model should have the notion of frames
	heartModel_.SetLambda(hermiteLambdaParams);
	
	// TODO Smooth along time
	
	delete P;
	delete lambda;
	delete p;
	delete dataLambda;
	delete temp;
	delete rhs;
	delete x;
	
	return;
}

std::vector<float> CAPModeller::ConvertToHermite(const Vector& bezierParams)
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

void CAPModeller::InitialiseModel()
{
	// Compute model coordinate axes from Apex, Base and RV insert points
	Point3D apex = dataPoints_[0].GetCoordinate();
	Point3D base = dataPoints_[1].GetCoordinate();
	Vector3D x = apex - base;
	x.Normalise();

	Point3D averageOfRVInserts = dataPoints_[3].GetCoordinate() + 
					(dataPoints_[2].GetCoordinate() - dataPoints_[3].GetCoordinate()) * 0.5;
	Vector3D y = averageOfRVInserts - base;
	Vector3D z = CrossProduct(x,y);
	z.Normalise();
	y.CrossProduct(z,x);
	cout << "Model coord x axis vector" << x << endl;
	cout << "Model coord y axis vector" << y << endl;
	cout << "Model coord z axis vector" << z << endl;
	
	// Compute the position of the model coord origin. (1/3 of the way from base to apex)
	Point3D origin = base + 1/3 * (base - apex);
	
	// Set focal length
	float focalLength = 42.0;
	
	// Set initial model parameters lambda, mu and theta
	// initial values for lambda come from the prior
	// theta is 1/2pi apart)
	// mu is equally spaced up to the base value

	return;
}

void CAPModeller::AddDataPoint(DataPoint* dataPoint)
{
	dataPoints_.push_back(dataPoint);
//	Point3D xi;
//	int elementNumber = heartModel_.ComputeXi(dataPoint->GetCoordinate(), xi);
	FitModel();
//	
//	std::vector<float> test(160);
//	
//	heartModel_.SetLambda(test); //Test
}
