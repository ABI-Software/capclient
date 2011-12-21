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
#include "utils/filesystem.h"
#include "utils/debug.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

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

ModellingModeApex::~ModellingModeApex()
{
	ModellingPointsMap::iterator itr = apex_.begin();
	for (;itr != apex_.end(); ++itr)
	{
		itr->second.Remove();
	}
}

void ModellingModeApex::PerformEntryAction()
{
	ModellingPointsMap::iterator itr = apex_.begin();
	for (;itr != apex_.end(); ++itr)
	{
		itr->second.SetVisible(true);
	}
}

void ModellingModeApex::PerformExitAction()
{
	ModellingPointsMap::iterator itr = apex_.begin();
	for (;itr != apex_.end(); ++itr)
	{
		itr->second.SetVisible(false);
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

void ModellingModeApex::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
{
	dbg("Add = " + toString(node_id));
	if (!apex_.empty())
	{
		ModellingPointsMap::iterator itr = apex_.find(node_id);//begin();
		if (itr == apex_.end())
		{
			for (itr = apex_.begin();itr != apex_.end(); ++itr)
			{
				itr->second.Remove();
			}
			apex_.clear();
		}
		else
		{
			MoveModellingPoint(node_id, position, time);
			return;
		}
	}

	ModellingPoint modellingPoint(APEX, region, node_id, position);
	modellingPoint.SetVisible(true);

	apex_[node_id] = modellingPoint;
}

void ModellingModeApex::MoveModellingPoint(int node_id, const Point3D& position, double time)
{
	ModellingPointsMap::iterator itr = apex_.find(node_id);
	if (itr != apex_.end())
		itr->second.SetPosition(position);
}

void ModellingModeApex::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	assert(apex_.size() == 1);
	//--assert(apex_[0].GetCmissNode() == dataPointID);
	apex_[0].SetPosition(coord);
}

void ModellingModeApex::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	assert(!apex_.empty());
	apex_.clear();
}

ModellingPoints ModellingModeApex::GetModellingPoints() const
{
	ModellingPoints mps;
	std::transform(apex_.begin(), apex_.end(), std::back_inserter(mps),
		boost::bind(&ModellingPointsMap::value_type::second, _1));

	return mps;
}

const ModellingPoint& ModellingModeApex::GetApex() const
{
	assert(apex_.size() == 1);
	ModellingPointsMap::const_iterator itr = apex_.begin();

	return itr->second;
}

// ModellingModeBase

void ModellingModeBase::PerformEntryAction()
{
	ModellingPointsMap::iterator itr = base_.begin();
	for (;itr != base_.end(); ++itr)
	{
		itr->second.SetVisible(true);
	}
}

void ModellingModeBase::PerformExitAction()
{
	ModellingPointsMap::iterator itr = base_.begin();
	for (;itr != base_.end(); ++itr)
	{
		itr->second.SetVisible(false);
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

void ModellingModeBase::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
{
	if (!base_.empty())
	{
		base_[0].Remove();
		base_.clear();
	}

	ModellingPoint modellingPoint(BASE, region, node_id, position);
	modellingPoint.SetVisible(true);

	base_[node_id] = modellingPoint;
}

//void ModellingModeBase::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
//{
//	if (!base_.empty())
//	{
//		base_.clear();
//	}
//
//	DataPoint dataPoint(dataPointID, coord, BASE, time);
//	dataPoint.SetValidPeriod(0,1); //REVISE
//	dataPoint.SetVisible(true);
//	base_.push_back(dataPoint);
//}

void ModellingModeBase::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& position, double time)
{
	assert(base_.size() == 1);
	//assert(base_[0].GetCmissNode() == dataPointID);
	base_[0].SetPosition(position);
}

void ModellingModeBase::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	assert(!base_.empty());
	base_.clear();
}

ModellingPoints ModellingModeBase::GetModellingPoints() const
{
	ModellingPoints mps;
	std::transform(base_.begin(), base_.end(), std::back_inserter(mps),
		boost::bind(&ModellingPointsMap::value_type::second, _1));

	return mps;
}

const ModellingPoint& ModellingModeBase::GetBase() const
{
	assert(base_.size()==1);
	ModellingPointsMap::const_iterator itr = base_.begin();

	return itr->second;
}

// ModellingModeRV

void ModellingModeRV::PerformEntryAction()
{
	ModellingPointsMap::iterator itr = rvInserts_.begin();
	for (;itr != rvInserts_.end(); ++itr)
	{
		itr->second.SetVisible(true);
	}
}

void ModellingModeRV::PerformExitAction()
{
	ModellingPointsMap::iterator itr = rvInserts_.begin();
	for (;itr != rvInserts_.end(); ++itr)
	{
		itr->second.SetVisible(false);
	}
}

ModellingMode* ModellingModeRV::OnAccept(Modeller& modeller)
{
	if ((rvInserts_.size() % 2) || rvInserts_.empty())
	{
		dbg("ModellingModeRV::OnAccept: Need n pairs of rv insertion points");
		return 0;
	}
	return modeller.GetModellingModeBasePlane();
}

void ModellingModeRV::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
{
	ModellingPoint modellingPoint(RV, region, node_id, position, time);
	modellingPoint.SetVisible(true);

	rvInserts_[node_id] = modellingPoint;
}

//void ModellingModeRV::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
//{
//	DataPoint dataPoint(dataPointID, coord, RV, time);
//	//double startTime = heartModel_.MapToModelFrameTime(time);
//	//double duration = (double)1.0f / heartModel_.GetNumberOfModelFrames();
//	//double endTime = startTime + duration;
//	
//	//std::cout << __func__ << ": time = " << time << ", startTime = " << startTime << ", endTime = " << endTime << std::endl;
//	//dataPoint.SetValidPeriod(startTime,endTime); //REVISE
//	dataPoint.SetVisible(true);
//	rvInserts_.insert(std::pair<Cmiss_node* ,DataPoint>(dataPointID,dataPoint));
//}

void ModellingModeRV::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	ModellingPointsMap::iterator itr = rvInserts_.find(1);
	assert(itr != rvInserts_.end());
	itr->second.SetPosition(coord);
}

void ModellingModeRV::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	ModellingPointsMap::iterator itr = rvInserts_.find(1);
	assert(itr != rvInserts_.end());
	rvInserts_.erase(itr);
}

ModellingPoints ModellingModeRV::GetModellingPoints() const
{
	ModellingPoints mps;
	std::transform(rvInserts_.begin(), rvInserts_.end(), std::back_inserter(mps),
		boost::bind(&ModellingPointsMap::value_type::second, _1));

	ModellingPointTimeLessThan lessThan;
	std::sort(mps.begin(), mps.end(), lessThan);

	return mps;
}

const ModellingPointsMap& ModellingModeRV::GetRVInsertPoints() const
{
	return rvInserts_;
}

// ModellingModeBasePlane

void ModellingModeBasePlane::PerformEntryAction()
{
	ModellingPointsMap::iterator itr = basePlanePoints_.begin();
	for (;itr != basePlanePoints_.end(); ++itr)
	{
		itr->second.SetVisible(true);
	}
}

void ModellingModeBasePlane::PerformExitAction()
{
	ModellingPointsMap::iterator itr = basePlanePoints_.begin();
	for (;itr != basePlanePoints_.end(); ++itr)
	{
		itr->second.SetVisible(false);
	}
}

ModellingMode* ModellingModeBasePlane::OnAccept(Modeller& modeller)
{
	if ((basePlanePoints_.size() % 2) || basePlanePoints_.empty())
	{
		dbg("ModellingModeBasePlane::OnAccept: Need n pairs of base plane points");
		return 0;
	}
	
	dbg("Warning: ModellingModeBasePlane::OnAccept : need to implement sorting for modelling points.");
	ModellingPoints mps = GetModellingPoints();
	ModellingPoints::const_iterator cit = mps.begin();
	std::map<int, int> timeLayoutMap;
	while (cit != mps.end())
	{
		int time = static_cast<int>(100000 * cit->GetTime());
		timeLayoutMap[time]++;
		++cit;
	}

	std::map<int, int>::const_iterator const_it = timeLayoutMap.begin();
	for (; const_it != timeLayoutMap.end(); ++const_it)
	{
		if (const_it->second < 2)
		{
			dbg("ModellingModeBasePlane::OnAccept: Each time value needs at least two base plane points.");
			return 0;
		}
	}
	//--DataPointTimeLessThan lessThan; // need lambda functions !
	//--std::sort(basePlanePoints_.begin(), basePlanePoints_.end(), lessThan);

	//modeller.AlignModel();
	//modeller.UpdateTimeVaryingModel();
	return modeller.GetModellingModeGuidePoints();
}

void ModellingModeBasePlane::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
{
	ModellingPoint modellingPoint(BASEPLANE, region, node_id, position, time);
	modellingPoint.SetVisible(true);

	basePlanePoints_[node_id] = modellingPoint;
}

//void ModellingModeBasePlane::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
//{
//	DataPoint dataPoint(dataPointID, coord, BASEPLANE, time);
//	//double startTime = heartModel_.MapToModelFrameTime(time);
//	//double duration = (double)1.0f / heartModel_.GetNumberOfModelFrames();
//	//double endTime = startTime + duration;
//	//dataPoint.SetValidPeriod(startTime,endTime); //REVISE
//	dataPoint.SetVisible(true);
//	basePlanePoints_.push_back(dataPoint);
//}

void ModellingModeBasePlane::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	dbg("Warning: ModellingModeBasePlane::MoveDataPoint : not complete for modelling points");
	//DataPointCmissNodeEqualTo equalTo(dataPointID);
	//std::vector<ModellingPoint>::iterator itr = std::find_if(basePlanePoints_.begin(), basePlanePoints_.end(), equalTo);
	//assert(itr != basePlanePoints_.end());
	//
	//itr->SetPosition(coord);
}

void ModellingModeBasePlane::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	dbg("Warning: ModellingModeBasePlane::MoveDataPoint : not complete for modelling points");
	//DataPointCmissNodeEqualTo equalTo(dataPointID);
	//std::vector<ModellingPoint>::iterator itr = std::find_if(basePlanePoints_.begin(), basePlanePoints_.end(), equalTo);
	//assert(itr != basePlanePoints_.end());
	////--std::cout << "Removing BP : " << itr->GetSliceName() << ", time = " << itr->GetTime() << '\n';//debug
	//basePlanePoints_.erase(itr);
}

ModellingPoints ModellingModeBasePlane::GetModellingPoints() const
{
	ModellingPoints mps;
	std::transform(basePlanePoints_.begin(), basePlanePoints_.end(), std::back_inserter(mps),
		boost::bind(&ModellingPointsMap::value_type::second, _1));

	ModellingPointTimeLessThan lessThan;
	std::sort(mps.begin(), mps.end(), lessThan);

	return mps;
}

const ModellingPointsMap& ModellingModeBasePlane::GetBasePlanePoints() const
{
	return basePlanePoints_;
}

// ModellingModeGuidePoints

ModellingModeGuidePoints::ModellingModeGuidePoints()
{
}

ModellingModeGuidePoints::~ModellingModeGuidePoints()
{
}

void ModellingModeGuidePoints::PerformEntryAction()
{
	ModellingPointsMap::iterator it = guidePoints_.begin();
	for (; it != guidePoints_.end(); ++it)
	{
		it->second.SetVisible(true);
	}

	std::vector<ModellingPointsMap>::iterator vectorIter = vectorOfModellingPoints_.begin();
	for (;vectorIter != vectorOfModellingPoints_.end(); ++vectorIter)
	{
		ModellingPointsMap::iterator itr = vectorIter->begin();
		for (;itr != vectorIter->end(); ++itr)
		{
			itr->second.SetVisible(true);
		}
	}
}

void ModellingModeGuidePoints::PerformExitAction()
{
	ModellingPointsMap::iterator it = guidePoints_.begin();
	for (; it != guidePoints_.end(); ++it)
	{
		it->second.SetVisible(false);
	}

	std::vector<ModellingPointsMap>::iterator vectorIter = vectorOfModellingPoints_.begin();
	for (;vectorIter != vectorOfModellingPoints_.end(); ++vectorIter)
	{
		ModellingPointsMap::iterator itr = vectorIter->begin();
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

void ModellingModeGuidePoints::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
{
	ModellingPoint modellingPoint(GUIDEPOINT, region, node_id, position, time);
	modellingPoint.SetVisible(true);

	guidePoints_[node_id] = modellingPoint;
}

//void ModellingModeGuidePoints::AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
//{
//#if defined(NDEBUG)
//	std::cout << "NDEBUG" << std::endl;
//#endif
//	
//	DataPoint dataPoint(dataPointID, coord, GUIDEPOINT, time);
//	//double startTime = heartModel_.MapToModelFrameTime(time);
//	//double duration = (double)1.0f / heartModel_.GetNumberOfModelFrames();
//	//double endTime = startTime + duration;
//	//dataPoint.SetValidPeriod(startTime,endTime); //REVISE
//	dataPoint.SetVisible(true);
//		
//	int frameNumber = 0;//heartModel_.MapToModelFrameNumber(dataPoint.GetTime());
//	
//	dbg(std::string("**** Change me **** ModellingModeGuidePoints::AddDataPoint time = ") + toString(dataPoint.GetTime()) + ", frame number = " + toString(frameNumber));
//	
//	vectorOfModellingPoints_[frameNumber].insert(std::pair<Cmiss_node* ,DataPoint>(dataPointID,dataPoint));
//	framesWithDataPoints_[frameNumber]++;
//	
////	Point3D xi;
////	int elementNumber = heartModel_.ComputeXi(dataPoint->GetCoordinate(), xi);
//	//FitModel(vectorOfModellingPoints_[frameNumber], frameNumber);
////	
////	std::vector<double> test(160);
////	
////	heartModel_.SetLambda(test); //Test
//}

void ModellingModeGuidePoints::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time)
{
	dbg("**** FIX, move to modeller class ****");
	//int frameNumber = heartModel_.MapToModelFrameNumber(time);
	//DataPoints::iterator itr = vectorOfModellingPoints_[frameNumber].find(dataPointID);
	//assert(itr != vectorOfModellingPoints_[frameNumber].end());
	//itr->second.SetCoordinate(coord);
	//FitModel(vectorOfModellingPoints_[frameNumber], frameNumber);
}

void ModellingModeGuidePoints::RemoveDataPoint(Cmiss_node* dataPointID, double time)
{
	dbg("**** FIX, move to modeller class ****");
	//int frameNumber = heartModel_.MapToModelFrameNumber(time);
	//DataPoints::iterator itr = vectorOfModellingPoints_[frameNumber].find(dataPointID);
	//assert(itr != vectorOfModellingPoints_[frameNumber].end());
	//vectorOfModellingPoints_[frameNumber].erase(itr);
	//framesWithDataPoints_[frameNumber]--;
	//FitModel(vectorOfModellingPoints_[frameNumber], frameNumber);
}

ModellingPoints ModellingModeGuidePoints::GetModellingPoints() const
{
	ModellingPoints mps;
	std::transform(guidePoints_.begin(), guidePoints_.end(), std::back_inserter(mps),
		boost::bind(&ModellingPointsMap::value_type::second, _1));

	ModellingPointTimeLessThan lessThan;
	std::sort(mps.begin(), mps.end(), lessThan);

	return mps;
}

ModellingPoints ModellingModeGuidePoints::GetModellingPointsAtTime(double time) const
{
	ModellingPoints mps = GetModellingPoints();
	ModellingPoints::const_iterator cit = mps.begin();
	ModellingPoints modellingPointsAtTime;
	while (cit != mps.end() && ((cit->GetTime() - time) < 1e-06))
	{
		if (fabs(cit->GetTime() - time) < 1e-06)
		{
			modellingPointsAtTime.push_back(*cit);
		}
		++cit;
	}

	return modellingPointsAtTime;
}

std::vector<int> ModellingModeGuidePoints::GetFramesWithModellingPoints(int numFrames) const
{
	std::vector<int> frames;
	frames.resize(numFrames);
	for (int i = 0; i < numFrames; i++)
	{
		if (GetModellingPointsAtTime(i/numFrames).size() > 0)
			frames[i] = 1;
		else
			frames[i] = 0;
	}

	return frames;
}

std::vector<ModellingPointsMap> ModellingModeGuidePoints::GetGuidePoints() const
{
	using boost::bind;
	std::vector<ModellingPointsMap> v;
	//for each map:
	//--typedef std::map<Cmiss_node*, ModellingPoint> Map;
	//--BOOST_FOREACH(const ModellingPointsMap& map, vectorOfModellingPoints_)
	//--{
	//--	std::transform(map.begin(), map.end(), std::back_inserter(v), bind(&ModellingPointsMap::value_type::second, _1));
	//--}
	return v;
}

void ModellingModeGuidePoints::Reset(unsigned int numFrames)
{
}

} // end namespace cap
