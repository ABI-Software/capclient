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

struct Cmiss_node; //REVISE

namespace cap
{

class HeartModel;
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
	
	explicit CAPModeller(HeartModel& heartModel);
	~CAPModeller(){}
	
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);
	
	void RemoveDataPoint(Cmiss_node* dataPointID, double time);
	
	bool OnAccept();
	
	CAPModellingMode* GetModellingModeApex();
	
	CAPModellingMode* GetModellingModeBase();
	
	CAPModellingMode* GetModellingModeRV();
	
	CAPModellingMode* GetModellingModeBasePlane();
	
	CAPModellingModeGuidePoints* GetModellingModeGuidePoints();
	
	std::vector<DataPoint> GetDataPoints() const;
	
	void SetDataPoints(std::vector<DataPoint>& allDataPoints); // this is used for feeding in data points read in from external files

//	void SetApex(const std::vector<DataPoint>& apex);
//	
//	void SetBase(const std::vector<DataPoint>& base);
//	
//	void SetRVInsertionPoints(const std::map<Cmiss_node*, DataPoint>& rvInserts);
//	
//	void SetBasePlanePoints(const std::map<Cmiss_node*, DataPoint>& rvInserts);
	
	
	void InitialiseModel();
	
	void UpdateTimeVaryingModel();
	
	void SmoothAlongTime();
	
	void ChangeMode(ModellingMode mode);
	
	ModellingMode GetCurrentMode() const
	{
		ModellingMode mode;
		if (currentModellingMode_ == &modellingModeApex_)
		{
			mode = APEX;
		}
		else if (currentModellingMode_ == &modellingModeBase_)
		{
			mode = BASE;
		}
		else if (currentModellingMode_ == &modellingModeRV_)
		{
			mode = RV;
		}
		else if (currentModellingMode_ == &modellingModeBasePlane_)
		{
			mode = BASEPLANE;
		}
		else
		{
			mode = GUIDEPOINT;
		}
		return mode;
	}
	
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
