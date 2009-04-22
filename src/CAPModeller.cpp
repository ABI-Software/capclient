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
const static char* rhsfile = "/Users/jchu014/Dev/Solver/Data/RHS_test_frame_9.txt";

CAPModeller::CAPModeller(CAPModelLVPS4X4& heartModel)
:
	heartModel_(heartModel),
	solverFactory_(new GMMFactory)
{
	SolverLibraryFactory& factory = *solverFactory_;
	
	std::cout << "Library = " << factory.GetName() << std::endl;

	// Read in S (smoothness matrix)
	S_ = factory.CreateMatrixFromFile(Sfile);
	// Read in G (global to local parameter map)
	G_ = factory.CreateMatrixFromFile(Gfile);

	// initialize preconditioner and GSMoothAMatrix
	
	preconditioner_ = factory.CreateDiagonalPreconditioner(*S_);
	
	aMatrix_ = factory.CreateGSmoothAMatrix(*S_, *G_);

	return;
}

CAPModeller::~CAPModeller()
{
	delete aMatrix_;
	delete preconditioner_;
	delete P_;
	delete S_;
	delete G_;
}

void CAPModeller::FitModel()
{		
	// Compute P 
	// 1. find xi coords for each data point
	DataPoints::const_iterator itr = dataPoints_.begin();
	DataPoints::const_iterator end = dataPoints_.end();
	std::vector<Point3D> xi_vector;
	std::vector<int> element_id_vector;
	for (;itr!=end;++itr)
	{
		Point3D xi;
		int elem_id = heartModel_.ComputeXi(itr->GetCoordinate(), xi);
		xi_vector.push_back(xi);
		element_id_vector.push_back(elem_id);
	}
	
	// 2. evaluate basis at the xi coords
	//    use this function as a temporary soln until Cmgui supports this
	double psi[32];
	CimBiCubicHermiteLinearBasis basis;
	std::vector<Point3D>::iterator itr_xi = xi_vector.begin();
	std::vector<Point3D>::const_iterator end_xi = xi_vector.end();
	for (;itr_xi!=end_xi;++itr_xi)
	{
		double temp[3];
		temp[0] = itr_xi->x;
		temp[1] = itr_xi->y;
		temp[2] = itr_xi->z;
		basis.evaluateBasis(psi, temp);
	}
	
	
	// 3. construct P
	
	// Compute RHS - GtPt(dataLamba - priorLambda)
	
	// TODO Smooth along time
	return;
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
}
