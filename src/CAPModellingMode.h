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

namespace cap
{

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
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const DataPoint& GetApex() const; //REVISE design:
	// Probably its more extensible to provide a uniform interface (virtual GetDataPoints) on all Modes.
	// However it is unlikely that new modes will be added in the future so probably its ok.

	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	std::vector<DataPoint> apex_; // holds at most 1 item
};

class CAPModellingModeBase : public CAPModellingMode
{
public:
	CAPModellingModeBase()
	{}
	
	CAPModellingMode* OnAccept(CAPModeller& modeller);
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const DataPoint& GetBase() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	std::vector<DataPoint> base_; // holds at most 1 item
};

} // end namespace cap

//class CAPModelLVPS4X4;
#include "CAPModelLVPS4X4.h"

namespace cap
{

class CAPModellingModeRV : public CAPModellingMode
{
public:
	CAPModellingModeRV(CAPModelLVPS4X4& heartModel)
	:heartModel_(heartModel)
	{}
	
	CAPModellingMode* OnAccept(CAPModeller& modeller);
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const std::map<Cmiss_node*, DataPoint>& GetRVInsertPoints() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
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
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	const std::vector<DataPoint>& GetBasePlanePoints() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	std::vector<DataPoint> basePlanePoints_; // holds n pairs of DataPoints ( n >= 1 )
	
	CAPModelLVPS4X4& heartModel_;
};

} // end namespace cap

#include "CAPTimeSmoother.h"

namespace cap
{

class SparseMatrix;
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
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	std::vector<DataPoint> GetDataPoints() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
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
	
	Plane InterpolateBasePlane(const std::map<int, Plane>& planes, int frame) const;
	
	Plane FitPlaneToBasePlanePoints(const std::vector<DataPoint>& basePlanePoints, const Vector3D& xAxis) const;
	
	std::vector<float> ConvertToHermite(const Vector&) const;
	
	CAPModelLVPS4X4& heartModel_;
	
	std::vector<DataPoints> vectorOfDataPoints_;
	
	std::vector< std::vector<float> > timeVaryingDataPoints_;
	
	SolverLibraryFactory* solverFactory_;
	SparseMatrix* S_;
	SparseMatrix* G_;
	//SparseMatrix* P_;
	Preconditioner* preconditioner_;
	GSmoothAMatrix* aMatrix_;
	Vector* prior_;
	
	SparseMatrix* bezierToHermiteTransform_; // Temporary
	
	CAPTimeSmoother timeSmoother_;
	std::vector<int> framesWithDataPoints_;
};

} // end namespace cap

#endif /* CAPMODELLINGMODE_H_ */
