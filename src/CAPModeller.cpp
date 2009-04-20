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

CAPModeller::CAPModeller(CAPModelLVPS4X4& heartModel)
:
	heartModel_(heartModel)
{
	// Read in S (smoothness matrix)
	// Read in G (global to local parameter map)
}

CAPModeller::~CAPModeller()
{
	// TODO Auto-generated destructor stub
}

void CAPModeller::FitModel()
{
	// Compute P 
	// 1. find xi coords for each data point
	// 2. evaluate basis at the xi coords
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
}
