/*
 * ModellingMode.h
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

class Modeller;

/**
 * Modelling mode.  This is an abstract base class for the modelling modes.
 */
class ModellingMode
{
public:

	/**
	 * Default constructor.
	 */
	ModellingMode();

	/**
	 * Destructor.
	 */
	virtual ~ModellingMode();

	virtual ModellingMode* OnAccept(Modeller& modeller) = 0;
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time) = 0;
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time) = 0;
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time) = 0;
	
	virtual void PerformEntryAction() = 0;
	virtual void PerformExitAction() = 0;
};

class ModellingModeApex : public ModellingMode
{
public:
	ModellingModeApex() {}
	
	ModellingMode* OnAccept(Modeller& modeller);
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	const DataPoint& GetApex() const; //REVISE design:
	// Probably its more extensible to provide a uniform interface (virtual GetDataPoints) on all Modes.
	// However it is unlikely that new modes will be added in the future so probably its ok.

	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	std::vector<DataPoint> apex_; // holds at most 1 item
};

class ModellingModeBase : public ModellingMode
{
public:
	ModellingModeBase() {}
	
	ModellingMode* OnAccept(Modeller& modeller);
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	const DataPoint& GetBase() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	std::vector<DataPoint> base_; // holds at most 1 item
};

class HeartModel;

class ModellingModeRV : public ModellingMode
{
public:
	ModellingModeRV() {}
	
	ModellingMode* OnAccept(Modeller& modeller);
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	const std::map<Cmiss_node*, DataPoint>& GetRVInsertPoints() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	std::map<Cmiss_node*, DataPoint> rvInserts_; // holds n pairs of DataPoints ( n >= 1 )
	
	//HeartModel& heartModel_;
};

class ModellingModeBasePlane : public ModellingMode
{
public:
	ModellingModeBasePlane() {}
	
	ModellingMode* OnAccept(Modeller& modeller);
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	const std::vector<DataPoint>& GetBasePlanePoints() const;
	
	virtual void PerformEntryAction();
	virtual void PerformExitAction();
	
private:
	std::vector<DataPoint> basePlanePoints_; // holds n pairs of DataPoints ( n >= 1 )
	
	//HeartModel& heartModel_;
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

class ModellingModeGuidePoints : public ModellingMode
{
public:
	typedef std::map<Cmiss_node*, DataPoint> DataPoints;
	
	ModellingModeGuidePoints();
	~ModellingModeGuidePoints();
	
	ModellingMode* OnAccept(Modeller& modeller);
	virtual void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	virtual void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
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
	
	std::vector<double> ConvertToHermite(const Vector&) const;
	
	//HeartModel& heartModel_;
	
	std::vector<DataPoints> vectorOfDataPoints_;
	
	std::vector< std::vector<double> > timeVaryingDataPoints_;
	
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
