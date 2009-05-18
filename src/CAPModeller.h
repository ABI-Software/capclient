/*
 * CAPModeller.h
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLER_H_
#define CAPMODELLER_H_

//#include <boost/ptr_container/ptr_vector.hpp>
#include <string> // no easy way to forward declare std::string cf) <iosfwd>
#include <map>
#include <vector>

#include "DataPoint.h" // needed for DataPoints. consider Pimpl?
#include "CAPTimeSmoother.h"

class CAPModelLVPS4X4;
class Matrix;
class Vector;
class Preconditioner;
class GSmoothAMatrix;
class SolverLibraryFactory;
class Cmiss_node; //REVISE

class CAPModeller {
public:
	typedef std::map<Cmiss_node*, DataPoint> DataPoints;
	
	CAPModeller(CAPModelLVPS4X4& heartModel);
	~CAPModeller();
	
	void FitModel(DataPoints& dataPoints, int frameNumber);
	
	void SmoothAlongTime();
	
	void AddDataPoint(Cmiss_node* dataPointID, const DataPoint& dataPoint);
	
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	
	void InitialiseModel();
	
	void ReadModelFromFile(std::string& filename);
	
	void UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber);
	
	void UpdateTimeVaryingModel();
	
private:
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

#endif /* CAPMODELLER_H_ */
