/*
 * DataPoint.h
 *
 *  Created on: Apr 1, 2009
 *      Author: jchu014
 */

#ifndef DATAPOINT_H_
#define DATAPOINT_H_

#include "CAPMath.h"

extern "C" {
#include "api/cmiss_node.h"
}

class DataPoint
{
public:
	
	DataPoint(Cmiss_node* node, const Point3D& coord, float time = 0, float weight = 1.0f);
	
	DataPoint(const DataPoint& other);
		
	~DataPoint();
	
	const Cmiss_node* GetCmissNode() const;
	
	Cmiss_node* GetCmissNode();
	
	const Point3D& GetCoordinate() const;
	
	void SetCoordinate(const Point3D& coord);
	
	float GetTime() const;
	
	void SetValidPeriod(float startTime, float endTime);
	
	void SetVisible(bool visibility);
	
	//HACK
	int GetSurfaceType() const;
	
	void SetSurfaceType(int type);
	
	// assignment operator
	DataPoint& operator=(const DataPoint& rhs);
	
private:
	Cmiss_node* cmissNode_;
	Point3D coordinate_;
	float weight_;
	float time_;
	int surfaceType_;
	float startTime_, endTime_; // these can't be inferred from time only 
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
#endif /* DATAPOINT_H_ */
