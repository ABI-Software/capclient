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
#include "model/modellingmode.h"
#include "model/modeller.h"
#include "math/solverlibraryfactory.h"
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
	: timeVaryingDataPoints_(134)
{
}

ModellingModeGuidePoints::~ModellingModeGuidePoints()
{
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
	//FitModel(vectorOfDataPoints_[frameNumber], frameNumber);
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

void ModellingModeGuidePoints::UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber)
{
	// Update the (Bezier) parameters for the newly fitted frame
	// This is in turn used as data points for the time varying model in the smoothing step
	
	for (int i = 0; i < 134; i++)
	{
		timeVaryingDataPoints_[i][frameNumber] = x[i];
	}
}

std::vector<DataPoint> ModellingModeGuidePoints::GetGuidePoints() const
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
