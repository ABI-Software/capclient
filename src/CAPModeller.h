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
//#include <map>
//#include <vector>

#include "DataPoint.h" // needed for DataPoints. consider Pimpl?
//#include "CAPTimeSmoother.h"
#include "CAPModellingMode.h"

class CAPModelLVPS4X4;
//class Matrix;
//class Vector;
//class Preconditioner;
//class GSmoothAMatrix;
//class SolverLibraryFactory;
class Cmiss_node; //REVISE

class CAPModeller {
public:
	enum ModellingMode
	{
		APEX,
		BASE,
		RV,
		BASEPLANE,
		GUIDEPOINT
	};
	
	CAPModeller(CAPModelLVPS4X4& heartModel);
	~CAPModeller(){}
	
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, float time);
	
	void RemoveDataPoint(Cmiss_node* dataPointID, float time);
	
	bool OnAccept();
	
	CAPModellingMode* GetModellingModeApex();
	
	CAPModellingMode* GetModellingModeBase();
	
	CAPModellingMode* GetModellingModeRV();
	
	CAPModellingMode* GetModellingModeBasePlane();
	
	CAPModellingModeGuidePoints* GetModellingModeGuidePoints();
	
	
	void SetApex(const std::vector<DataPoint>& apex);
	
	void SetBase(const std::vector<DataPoint>& base);
	
	void SetRVInsertionPoints(const std::map<Cmiss_node*, DataPoint>& rvInserts);
	
	void SetBasePlanePoints(const std::map<Cmiss_node*, DataPoint>& rvInserts);
	
	
	void InitialiseModel();
	
	void UpdateTimeVaryingModel();
	
	void SmoothAlongTime();
	
	void ChangeMode(ModellingMode mode);
	
private:
	void CAPModeller::ChangeMode(CAPModellingMode* newMode);
	
	CAPModellingModeApex modellingModeApex_;
	CAPModellingModeBase modellingModeBase_;
	CAPModellingModeRV modellingModeRV_;
	CAPModellingModeBasePlane modellingModeBasePlane_;
	CAPModellingModeGuidePoints modellingModeGuidePoints_;
	
	CAPModellingMode* currentModellingMode_;
	
	
};

#endif /* CAPMODELLER_H_ */
