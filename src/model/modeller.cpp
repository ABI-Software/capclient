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
#include "utils/debug.h"
#include "math/totalleastsquares.h"

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
{
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
	}
	
	modellingModeGuidePoints_.InitialiseModelLambdaParams();
}

void Modeller::UpdateTimeVaryingModel() //REVISE
{
//	ModellingModeGuidePoints* gpMode = dynamic_cast<ModellingModeGuidePoints*>(currentModellingMode_); //REVISE
//	if (gpMode)
//	{
//		gpMode->UpdateTimeVaryingModel();
//	}
	modellingModeGuidePoints_.UpdateTimeVaryingModel();
}

void Modeller::SmoothAlongTime()
{
	ModellingModeGuidePoints* gpMode = dynamic_cast<ModellingModeGuidePoints*>(currentModellingMode_); //REVISE
	if (gpMode)
	{
		gpMode->SmoothAlongTime();
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

void Modeller::SetDataPoints(std::vector<DataPoint>& dataPoints)
{
	if (dataPoints.empty()) //FIXME 
	{
		// This handles the case where no data points are defined
		// e.g model files converted from CIM models
		// FIXME - this does not work in cases where neither data points nor
		//         model files are defined in the xml file.
		modellingModeGuidePoints_.InitialiseModelLambdaParams();
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

} // end namespace cap
