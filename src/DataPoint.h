/*
 * DataPoint.h
 *
 *  Created on: Apr 1, 2009
 *      Author: jchu014
 */

#ifndef DATAPOINT_H_
#define DATAPOINT_H_

#include "CAPTypes.h"
#include "CAPMath.h"

extern "C" {
#include "api/cmiss_node.h"
}

namespace cap
{

class DataPoint
{
public:
	
	DataPoint(Cmiss_node* node, const Point3D& coord, DataPointType dataPointType, double time, double weight = 1.0f);
	
	DataPoint(const DataPoint& other);
		
	~DataPoint();
	
	const Cmiss_node* GetCmissNode() const;
	
	Cmiss_node* GetCmissNode();
	
	const Point3D& GetCoordinate() const;
	
	void SetCoordinate(const Point3D& coord);
	
	double GetTime() const;
	
	void SetValidPeriod(double startTime, double endTime);
	
	void SetVisible(bool visibility);
	
	DataPointType GetDataPointType() const;
	
	void SetDataPointType(DataPointType type);
	
	//HACK
	SurfaceType GetSurfaceType() const;
	
	void SetSurfaceType(SurfaceType type);
	
	std::string GetSliceName() const; // returns the name of the slice this date point belongs to
	
	// assignment operator
	DataPoint& operator=(const DataPoint& rhs);
	
private:

	void DecrementCmissNodeObjectCount();

	Cmiss_node* cmissNode_;
	Point3D coordinate_;
	double weight_;
	double time_;
	SurfaceType surfaceType_;
	DataPointType dataPointType_;
	double startTime_, endTime_; // these can't be inferred from time only
};

struct DataPointTimeLessThan // used for sorting DataPoints with respect to time
{
	bool operator() (const DataPoint& i, const DataPoint& j)  const
	{
		return (i.GetTime()<j.GetTime());
	}
};

class DataPointCmissNodeEqualTo
{
public:
	DataPointCmissNodeEqualTo(const Cmiss_node* id)
	: id_(id)
	{}
	
	bool operator() (const DataPoint& point) const
	{
		return (id_ == point.GetCmissNode());
	}
	
private:
	const Cmiss_node* id_;
};

} // end namespace cap
#endif /* DATAPOINT_H_ */
