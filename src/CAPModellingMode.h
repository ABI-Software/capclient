/*
 * CAPModellingMode.h
 *
 *  Created on: May 27, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLINGMODE_H_
#define CAPMODELLINGMODE_H_

// Implementation of FSM using the State Pattern

#include "DataPoint.h"
#include <vector>
#include <map>

class CAPModeller;

class CAPModellingMode 
{
public:
	CAPModellingMode(CAPModeller& modeller);
	virtual ~CAPModellingMode();
	
	virtual CAPModellingMode* OnAccept() = 0;
	virtual void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint) = 0;
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time) = 0;
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, float time) = 0;
protected:
	CAPModeller& modeller_;
};

class CAPModellingModeApex : public CAPModellingMode
{
public:
	CAPModellingModeApex(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
	void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
private:
	std::vector<DataPoint> apex_; // holds at most 1 item
};

class CAPModellingModeBase : public CAPModellingMode
{
public:
	CAPModellingModeBase(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
	void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
private:
	std::vector<DataPoint> base_; // holds at most 1 item
};

class CAPModellingModeRV : public CAPModellingMode
{
public:
	CAPModellingModeRV(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
	void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
private:
	std::map<Cmiss_node*, DataPoint> rvInserts_; // holds n pairs of DataPoints ( n >= 1 )
};

class CAPModellingModeBasePlane : public CAPModellingMode
{
public:
	CAPModellingModeBasePlane(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
	void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
private:
	std::vector<DataPoint> BasePlanePoints_; // holds n pairs of DataPoints ( n >= 1 )
};

class CAPModellingModeGuidePoints : public CAPModellingMode
{
public:
	CAPModellingModeGuidePoints(CAPModeller& modeller)
	: CAPModellingMode(modeller)
	{}
	
	CAPModellingMode* OnAccept();
	void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
};

#endif /* CAPMODELLINGMODE_H_ */
