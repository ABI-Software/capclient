 /*
 * Modeller.h
 *
 *  Created on: Apr 15, 2009
 *      Author: jchu014
 */

#ifndef CAPMODELLER_H_
#define CAPMODELLER_H_

#include "model/modellingmode.h"
//#include <boost/ptr_container/ptr_vector.hpp>
#include <string> // no easy way to forward declare std::string cf) <iosfwd>

#include "DataPoint.h"

struct Cmiss_node; //REVISE

namespace cap
{

class CAPClient;
class SparseMatrix;
class Vector;
class Preconditioner;
class GSmoothAMatrix;
class SolverLibraryFactory;

/**
 * This Modeller class controls the modelling of the heart model.  It holds the current modelling mode
 * and holds the data points used in the modelling process.
 */
class Modeller {
public:

	/**
	 * Values that represent ModellingModeEnum.
	 */
	enum ModellingModeEnum
	{
		APEX,
		BASE,
		RV,
		BASEPLANE,
		GUIDEPOINT
	};

	/**
	 * Defines an alias representing the modelling mode enum map.
	 */
	typedef std::map<Modeller::ModellingModeEnum, std::string> ModellingModeEnumMap;

	static const ModellingModeEnumMap ModellingModeStrings; /**< The modelling mode strings */

	/**
	 * Constructor.
	 *
	 * @param	mainApp	The main application.
	 */
	explicit Modeller(CAPClient *mainApp);

	/**
	 * Destructor.
	 */
	~Modeller();

	/**
	 * Adds a data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	coord			   	The coordinate.
	 * @param	time			   	The time.
	 */
	void AddDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);

	/**
	 * Move data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	coord			   	The coordinate.
	 * @param	time			   	The time.
	 */
	void MoveDataPoint(Cmiss_node* dataPointID, const Point3D& coord, double time);

	/**
	 * Removes the data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	time			   	The time.
	 */
	void RemoveDataPoint(Cmiss_node* dataPointID, double time);

	/**
	 * Executes the accept action.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool OnAccept();

	/**
	 * Gets the modelling mode apex.
	 *
	 * @return	null if it fails, else the modelling mode apex.
	 */
	ModellingMode* GetModellingModeApex();

	/**
	 * Gets the modelling mode base.
	 *
	 * @return	null if it fails, else the modelling mode base.
	 */
	ModellingMode* GetModellingModeBase();

	/**
	 * Gets the modelling mode rv.
	 *
	 * @return	null if it fails, else the modelling mode rv.
	 */
	ModellingMode* GetModellingModeRV();

	/**
	 * Gets the modelling mode base plane.
	 *
	 * @return	null if it fails, else the modelling mode base plane.
	 */
	ModellingMode* GetModellingModeBasePlane();

	/**
	 * Gets the modelling mode guide points.
	 *
	 * @return	null if it fails, else the modelling mode guide points.
	 */
	ModellingModeGuidePoints* GetModellingModeGuidePoints();

	/**
	 * Gets the data points.
	 *
	 * @return	The data points.
	 */
	std::vector<DataPoint> GetDataPoints() const;

	/**
	 * Sets a data points.
	 *
	 * @param [in,out]	allDataPoints	all data points.
	 */
	void SetDataPoints(std::vector<DataPoint>& allDataPoints); // this is used for feeding in data points read in from external files

//	void SetApex(const std::vector<DataPoint>& apex);
//	
//	void SetBase(const std::vector<DataPoint>& base);
//	
//	void SetRVInsertionPoints(const std::map<Cmiss_node*, DataPoint>& rvInserts);
//	
//	void SetBasePlanePoints(const std::map<Cmiss_node*, DataPoint>& rvInserts);

	/**
	 * Align model.
	 */
	void AlignModel();

	/**
	 * Updates the time varying model.
	 */
	void UpdateTimeVaryingModel();

	/**
	 * Smooth along time.
	 */
	void SmoothAlongTime();

	/**
	 * Initialises the model lambda parameters.
	 */
	void InitialiseModelLambdaParams();

	/**
	 * Change mode.
	 *
	 * @param	mode	The mode.
	 */
	void ChangeMode(ModellingModeEnum mode);

	/**
	 * Gets the current mode.
	 *
	 * @return	The current mode.
	 */
	ModellingModeEnum GetCurrentMode() const
	{
		ModellingModeEnum mode;
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
	/**
	 * Initialises the modelling mode strings.
	 *
	 * @return	A Modelling mode enum map of enums to strings.
	 */
	static ModellingModeEnumMap InitModellingModeStrings()
	{
		ModellingModeEnumMap m;
		m[APEX] = std::string("APEX");
		m[BASE] = std::string("BASE");
		m[RV] = std::string("RV");
		m[BASEPLANE] = std::string("BASEPLANE");
		m[GUIDEPOINT] = std::string("GUIDEPOINT");
		return m;
	}

	/**
	 * Fit plane to base plane points.
	 *
	 * @param	basePlanePoints	The base plane points.
	 * @param	xAxis		   	The x coordinate axis.
	 *
	 * @return	The fitted plane.
	 */
	Plane FitPlaneToBasePlanePoints(const std::vector<DataPoint>& basePlanePoints, const Vector3D& xAxis) const;

	/**
	 * Interpolate base plane.
	 *
	 * @param	planes	The planes.
	 * @param	frame 	The frame.
	 *
	 * @return	The interpolated plane.
	 */
	Plane InterpolateBasePlane(const std::map<int, Plane>& planes, int frame) const;

	/**
	 * Fit model.
	 *
	 * @param [in,out]	dataPoints	The data points.
	 * @param	frameNumber		  	The frame number.
	 */
	void FitModel(DataPoints& dataPoints, int frameNumber);

	/**
	 * Change modelling mode.
	 *
	 * @param [in,out]	newMode	If non-null, the new mode.
	 */
	void ChangeMode(ModellingMode* newMode);

	/**
	 * Converts a  to hermite parameters.
	 *
	 * @param	bezierParams	vector to convert.
	 *
	 * @return	The hermite paramter vector.
	 */
	std::vector<double> ConvertToHermite(const Vector& bezierParams) const;
	
	CAPClient *mainApp_;	/**< The main application */
	ModellingModeApex modellingModeApex_;   /**< The modelling mode apex */
	ModellingModeBase modellingModeBase_;   /**< The modelling mode base */
	ModellingModeRV modellingModeRV_;   /**< The modelling mode rv */
	ModellingModeBasePlane modellingModeBasePlane_; /**< The modelling mode base plane */
	ModellingModeGuidePoints modellingModeGuidePoints_; /**< The modelling mode guide points */
	
	ModellingMode* currentModellingMode_;   /**< The current modelling mode */
	
	SolverLibraryFactory* solverFactory_;   /**< The solver factory */
	SparseMatrix* S_;   /**< The s */
	SparseMatrix* G_;   /**< The g */
	//SparseMatrix* P_;
	Preconditioner* preconditioner_;	/**< The preconditioner */
	GSmoothAMatrix* aMatrix_;   /**< The matrix */
	Vector* prior_; /**< The prior */
	
	SparseMatrix* bezierToHermiteTransform_;	/**< The bezier to hermite transform, temporary? */
	
	TimeSmoother timeSmoother_;  /**< The time smoother */
};

} //end namespace cap
#endif /* CAPMODELLER_H_ */
