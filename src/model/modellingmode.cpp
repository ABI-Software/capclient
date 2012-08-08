/*
 * ModellingMode.cpp
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#include "model/modellingmode.h"

#include "capclientconfig.h"
#include "model/modeller.h"
#include "math/solverlibraryfactory.h"
#include "utils/filesystem.h"
#include "utils/misc.h"
#include "utils/debug.h"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <algorithm>
#include <assert.h>
#include <ctime>

namespace cap
{

	ModellingMode::ModellingMode()
	{
	}

	ModellingMode::~ModellingMode()
	{
		ModellingPointsMap::iterator itr = modellingPoints_.begin();
		for (;itr != modellingPoints_.end(); ++itr)
		{
			itr->second.Remove();
		}
	}

	void ModellingMode::PerformEntryAction()
	{
		ModellingPointsMap::iterator itr = modellingPoints_.begin();
		for (;itr != modellingPoints_.end(); ++itr)
		{
			itr->second.SetVisible(true);
		}
	}

	void ModellingMode::PerformExitAction()
	{
		ModellingPointsMap::iterator itr = modellingPoints_.begin();
		for (;itr != modellingPoints_.end(); ++itr)
		{
			itr->second.SetVisible(false);
		}
	}

	bool ModellingMode::DoesModellingPointExist(int node_id)
	{
		return (modellingPoints_.find(node_id) != modellingPoints_.end());
	}

	void ModellingMode::MoveModellingPoint(int node_id, const Point3D& position, double /*time*/)
	{
		ModellingPointsMap::iterator itr = modellingPoints_.find(node_id);
		if (itr != modellingPoints_.end())
		{
			itr->second.SetPosition(position);
			itr->second.ClearAttachedToList();
		}
	}

	void ModellingMode::RemoveModellingPoint(int node_id, double /*time*/)
	{
		ModellingPointsMap::iterator itr = modellingPoints_.find(node_id);
		if (itr != modellingPoints_.end())
		{
			itr->second.Remove();
			modellingPoints_.erase(itr);
		}
	}

	ModellingPoints ModellingMode::GetModellingPoints() const
	{
		ModellingPoints mps;
		std::transform(modellingPoints_.begin(), modellingPoints_.end(), std::back_inserter(mps),
			boost::bind(&ModellingPointsMap::value_type::second, _1));

		ModellingPointTimeLessThan lessThan;
		std::sort(mps.begin(), mps.end(), lessThan);

		return mps;
	}

	void ModellingMode::AttachToIfOn(int node_id, const std::string& label, const Point3D& location, const Vector3D& normal)
	{
		ModellingPointsMap::iterator itr = modellingPoints_.find(node_id);
		if (itr != modellingPoints_.end())
		{
			ModellingPoint& mp = itr->second;
			mp.AttachToIfOn(label, location, normal);
		}

	}

	bool ModellingMode::ImagePlaneMoved(const std::string& label, Vector3D diff)
	{
		bool moved = false;
		ModellingPointsMap::iterator itr = modellingPoints_.begin();
		while (itr != modellingPoints_.end())
		{
			ModellingPoint& mp = itr->second;
			Point3D pos = mp.GetPosition();
			if (mp.IsAttachedTo(label))
			{
				mp.SetCoordinates(pos-diff);
				moved = true;
			}

			++itr;
		}

		return moved;
	}

	// ModellingModeApex

	ModellingMode* ModellingModeApex::OnAccept(Modeller& modeller)
	{
		if (CanAccept())
			return modeller.GetModellingModeBase();

		return 0;
	}

	bool ModellingModeApex::CanAccept() const
	{
		return modellingPoints_.size() == 1;
	}

	void ModellingModeApex::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double /*time*/)
	{
		if (!modellingPoints_.empty())
		{
			ModellingPointsMap::iterator itr = modellingPoints_.begin();
			while(itr != modellingPoints_.end())
			{
				// If the node id is the same we are really moving it so in this case
				// we don't delete it from Cmgui.
				if (node_id != itr->second.GetNodeIdentifier())
					itr->second.Remove();
				itr++;
			}
			modellingPoints_.clear();
		}

		ModellingPoint modellingPoint(APEX, region, node_id, position);
		modellingPoint.SetVisible(true);

		modellingPoints_[node_id] = modellingPoint;
	}

	// ModellingModeBase

	ModellingMode* ModellingModeBase::OnAccept(Modeller& modeller)
	{
		if (CanAccept())
			return modeller.GetModellingModeRV();

		return 0;
	}

	bool ModellingModeBase::CanAccept() const
	{
		return modellingPoints_.size() == 1;
	}

	void ModellingModeBase::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double /*time*/)
	{
		if (!modellingPoints_.empty())
		{
			ModellingPointsMap::iterator itr = modellingPoints_.begin();
			while(itr != modellingPoints_.end())
			{
				// If the node id is the same we are really moving it so in this case
				// we don't delete it from Cmgui.
				if (node_id != itr->second.GetNodeIdentifier())
					itr->second.Remove();
				itr++;
			}
			modellingPoints_.clear();
		}

		ModellingPoint modellingPoint(BASE, region, node_id, position);
		modellingPoint.SetVisible(true);

		modellingPoints_[node_id] = modellingPoint;
	}

	// ModellingModeRV

	ModellingMode* ModellingModeRV::OnAccept(Modeller& modeller)
	{
		if (CanAccept())
			return modeller.GetModellingModeBasePlane();

		return 0;
	}

	bool ModellingModeRV::CanAccept() const
	{
		// If we don't have any modelling points or not an even number of modelling points
		// then return fail (0).
		if ((modellingPoints_.size() % 2) || modellingPoints_.empty())
			return false;

		// Create a map of times and time counts
		ModellingPoints mps = GetModellingPoints();
		ModellingPoints::const_iterator cit = mps.begin();
		std::map<int, int> timeLayoutMap;
		while (cit != mps.end())
		{
			// Move from dealing with double comparisons to ints, so that we can index the map
			// with an int version of the time. 1e-06 time resolution should be good enough.
			int time = static_cast<int>(100000 * cit->GetTime());
			timeLayoutMap[time]++;
			++cit;
		}

		std::map<int, int>::const_iterator const_it = timeLayoutMap.begin();
		for (; const_it != timeLayoutMap.end(); ++const_it)
		{
			// Each time value should have exactly two modelling points
			if (const_it->second != 2)
				return false;
		}

		return true;
	}

	void ModellingModeRV::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
	{
		ModellingPoint modellingPoint(RV, region, node_id, position, time);
		modellingPoints_[node_id] = modellingPoint;
	}

	// ModellingModeBasePlane

	ModellingMode* ModellingModeBasePlane::OnAccept(Modeller& modeller)
	{
		if (CanAccept())
			return modeller.GetModellingModeGuidePoints();

		return 0;
	}

	bool ModellingModeBasePlane::CanAccept() const
	{
		// If we don't have any modelling points then fail (0).
		if (modellingPoints_.empty())
			return false;

		// Create a map of times and time counts
		ModellingPoints mps = GetModellingPoints();
		ModellingPoints::const_iterator cit = mps.begin();
		std::map<int, ModellingPoints> timeLayoutMap;
		while (cit != mps.end())
		{
			// Move from dealing with double comparisons to ints, so that we can index the map
			// with an int version of the time. 1e-06 time resolution should be good enough.
			int time = static_cast<int>(100000 * cit->GetTime());
			timeLayoutMap[time].push_back(*cit);
			++cit;
		}

		std::map<int, ModellingPoints>::const_iterator const_it = timeLayoutMap.begin();
		for (; const_it != timeLayoutMap.end(); ++const_it)
		{
			// If a time value has more than 2 modelling points then they cannot all be on the
			// same image plane.
			if ((const_it->second).size() > 2)
			{
				ModellingPoint reference = (const_it->second).at(0);
				std::vector<std::string> intersection = reference.GetAttachedTo();
				ModellingPoints::const_iterator cit = (const_it->second).begin();
				while (cit != (const_it->second).end())
				{
					std::vector<std::string> current = cit->GetAttachedTo();
					std::vector<std::string> current_intersection;
					std::set_intersection(intersection.begin(), intersection.end(), current.begin(), current.end(), std::back_inserter(current_intersection));
					intersection = current_intersection;

					++cit;
				}
				if (intersection.size() > 0)
					return false;
			}
		}

		return true;
	}

	void ModellingModeBasePlane::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
	{
		ModellingPoint modellingPoint(BASEPLANE, region, node_id, position, time);
		modellingPoints_[node_id] = modellingPoint;
	}

	// ModellingModeGuidePoints

	ModellingMode* ModellingModeGuidePoints::OnAccept(Modeller& /*modeller*/)
	{
		return 0;
	}

	void ModellingModeGuidePoints::AddModellingPoint(Cmiss_region_id region, int node_id, const Point3D& position, double time)
	{
		ModellingPoint modellingPoint(GUIDEPOINT, region, node_id, position, time);
		modellingPoints_[node_id] = modellingPoint;
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
			double time = static_cast<double>(i)/numFrames;
			if (GetModellingPointsAtTime(time).size() > 0)
				frames[i] = 1;
			else
				frames[i] = 0;
		}

		return frames;
	}

	void ModellingModeGuidePoints::SetHeartSurfaceType(int node_id, HeartSurfaceEnum surface)
	{
		modellingPoints_[node_id].SetHeartSurfaceType(surface);
	}

} // end namespace cap
