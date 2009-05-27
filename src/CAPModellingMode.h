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

#include "CAPTimeSmoother.h"

class CAPModelLVPS4X4;
class Matrix;
class Vector;
class Preconditioner;
class GSmoothAMatrix;
class SolverLibraryFactory;

class CAPModellingModeGuidePoints : public CAPModellingMode
{
public:
	typedef std::map<Cmiss_node*, DataPoint> DataPoints;
	
	CAPModellingModeGuidePoints(CAPModeller& modeller, CAPModelLVPS4X4& heartModel);
	~CAPModellingModeGuidePoints();
	
	CAPModellingMode* OnAccept();
	void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	void InitialiseModel();
	void ReadModelFromFile(std::string& filename);
	void UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber);
	void UpdateTimeVaryingModel();
	void SmoothAlongTime();
	
private:
	void FitModel(DataPoints& dataPoints, int frameNumber);
	
	std::vector<float> ConvertToHermite(const Vector&);
	
	CAPModelLVPS4X4& heartModel_;
	
	std::vector<DataPoints> vectorOfDataPoints_;
	
	std::vector< std::vector<float> > timeVaryingDataPoints_;
	
	SolverLibraryFactory* solverFactory_;
	Matrix* S_;
	Matrix* G_;
	Matrix* P_;
	Preconditioner* preconditioner_;
	GSmoothAMatrix* aMatrix_;
	Vector* prior_;
	
	Matrix* bezierToHermiteTransform_; // Temporary
	
	CAPTimeSmoother timeSmoother_;
};

#endif /* CAPMODELLINGMODE_H_ */
