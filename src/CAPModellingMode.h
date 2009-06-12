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
	CAPModellingMode();
	virtual ~CAPModellingMode();
	
	virtual CAPModellingMode* OnAccept(CAPModeller& modeller) = 0;
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time) = 0;
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time) = 0;
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, float time) = 0;
	
	virtual void PerformEntryAction() = 0;
	virtual void PerformExitAction() = 0;
};

class CAPModellingModeApex : public CAPModellingMode
{
public:
	CAPModellingModeApex()
	{}
	
	CAPModellingMode* OnAccept(CAPModeller& modeller);
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const DataPoint& GetApex() const; //REVISE design

	void PerformEntryAction();
	void PerformExitAction();
	
private:
	std::vector<DataPoint> apex_; // holds at most 1 item
};

class CAPModellingModeBase : public CAPModellingMode
{
public:
	CAPModellingModeBase()
	{}
	
	CAPModellingMode* OnAccept(CAPModeller& modeller);
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const DataPoint& GetBase() const;
	
	void PerformEntryAction();
	void PerformExitAction();
	
private:
	std::vector<DataPoint> base_; // holds at most 1 item
};

//class CAPModelLVPS4X4;
#include "CAPModelLVPS4X4.h"

class CAPModellingModeRV : public CAPModellingMode
{
public:
	CAPModellingModeRV(CAPModelLVPS4X4& heartModel)
	:heartModel_(heartModel)
	{}
	
	CAPModellingMode* OnAccept(CAPModeller& modeller);
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const std::map<Cmiss_node*, DataPoint>& GetRVInsertPoints() const;
	
	void PerformEntryAction();
	void PerformExitAction();
	
private:
	std::map<Cmiss_node*, DataPoint> rvInserts_; // holds n pairs of DataPoints ( n >= 1 )
	
	CAPModelLVPS4X4& heartModel_;
};

class CAPModellingModeBasePlane : public CAPModellingMode
{
public:
	CAPModellingModeBasePlane(CAPModelLVPS4X4& heartModel)
	:heartModel_(heartModel)
	{}
	
	CAPModellingMode* OnAccept(CAPModeller& modeller);
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const std::vector<DataPoint>& GetBasePlanePoints() const;
	
	void PerformEntryAction();
	void PerformExitAction();
	
private:
	std::vector<DataPoint> basePlanePoints_; // holds n pairs of DataPoints ( n >= 1 )
	
	CAPModelLVPS4X4& heartModel_;
};

#include "CAPTimeSmoother.h"

class Matrix;
class Vector;
class Preconditioner;
class GSmoothAMatrix;
class SolverLibraryFactory;

class CAPModellingModeGuidePoints : public CAPModellingMode
{
public:
	typedef std::map<Cmiss_node*, DataPoint> DataPoints;
	
	CAPModellingModeGuidePoints(CAPModelLVPS4X4& heartModel);
	~CAPModellingModeGuidePoints();
	
	CAPModellingMode* OnAccept(CAPModeller& modeller);
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const std::vector<DataPoint>& GetDataPoints() const;
	
	void PerformEntryAction();
	void PerformExitAction();
	
	void InitialiseModel(const DataPoint& apex,
			const DataPoint& base,
			const std::map<Cmiss_node*, DataPoint>& rvInserts,
			const std::vector<DataPoint>& basePlanePoints);
	
	void InitialiseModelLambdaParams();
	
	void ReadModelFromFile(std::string& filename);
	void UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber);
	void UpdateTimeVaryingModel();
	void SmoothAlongTime();
	
private:
	void FitModel(DataPoints& dataPoints, int frameNumber);
	
	Plane InterpolateBasePlane(const std::map<int, Plane>& planes, int frame);
	
	Plane FitPlaneToBasePlanePoints(const std::vector<DataPoint>& basePlanePoints, const Vector3D& xAxis);
	
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
	std::vector<int> framesWithDataPoints_;
};

#endif /* CAPMODELLINGMODE_H_ */
