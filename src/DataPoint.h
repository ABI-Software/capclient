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
	DataPoint(Cmiss_node_id node, const Point3D& coord, float weight = 1.0f)
	:
		cmissNode_(node),
		coordinate_(coord),
		weight_(weight)
	{
	};
	
	const Point3D& GetCoordinate() const
	{
		return coordinate_;
	}
	
private:
	Cmiss_node_id cmissNode_;
	Point3D coordinate_;
	float weight_;
	float time_;
	
};
#endif /* DATAPOINT_H_ */
