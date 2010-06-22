/*
 * CAPModeller.h
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLER_H_
#define CAPMODELLER_H_

#include "CAPModellingMode.h"
//#include <boost/ptr_container/ptr_vector.hpp>
#include <string> // no easy way to forward declare std::string cf) <iosfwd>

class Cmiss_node; //REVISE

namespace cap
{

class CAPModelLVPS4X4;
//class SparseMatrix;
//class Vector;
//class Preconditioner;
//class GSmoothAMatrix;
//class SolverLibraryFactory;

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
	
	explicit CAPModeller(CAPModelLVPS4X4& heartModel);
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
	void ChangeMode(CAPModellingMode* newMode);
	
	CAPModellingModeApex modellingModeApex_;
	CAPModellingModeBase modellingModeBase_;
	CAPModellingModeRV modellingModeRV_;
	CAPModellingModeBasePlane modellingModeBasePlane_;
	CAPModellingModeGuidePoints modellingModeGuidePoints_;
	
	CAPModellingMode* currentModellingMode_;
	
	
};

} //end namespace cap
#endif /* CAPMODELLER_H_ */
