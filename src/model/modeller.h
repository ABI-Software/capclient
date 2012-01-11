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

#include "datapoint.h"
#include "imodeller.h"

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
	 * Constructor.
	 *
	 * @param	mainApp	An interface to the main application that allows limited interaction.
	 */
	explicit Modeller(IModeller *mainApp);

	/**
	 * Destructor.
	 */
	~Modeller();

	/**
	 * Adds a modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void AddModellingPoint(Cmiss_region_id region, int node_id, Point3D const& position, double time);

	/**
	 * Move modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void MoveModellingPoint(Cmiss_region_id region, int node_id, Point3D const& position, double time);

	/**
	 * Removes the modelling point.
	 *
	 * @param	region 	The region.
	 * @param	node_id	Identifier for the node.
	 * @param	time   	The time.
	 */
	void RemoveModellingPoint(Cmiss_region_id region, int node_id, double time);

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
	 * Gets the modelling points.
	 *
	 * @return	The modelling points.
	 */
	std::vector<ModellingPoint> GetModellingPoints() const;

	/**
	 * Sets data points is used for feeding in data points read in from external files.
	 *
	 * @param [in,out]	allDataPoints	all data points.
	 */
	void SetDataPoints(std::vector<DataPoint>& allDataPoints);

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
	 * Initialises the Bezier lambda parameters.
	 */
	void InitialiseBezierLambdaParams();

	/**
	 * Change mode.
	 *
	 * @param	mode	The mode.
	 */
	void ChangeMode(ModellingEnum mode);

	/**
	 * Gets the current mode.
	 *
	 * @return	The current mode.
	 */
	ModellingEnum GetCurrentMode() const
	{
		ModellingEnum mode;
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

	/**
	 * Interpolate base plane.
	 *
	 * @param	planes	The planes.
	 * @param	frame 	The frame.
	 *
	 * @return	The interpolated plane.
	 */
	Plane InterpolateBasePlane(const std::map<double, Plane>& planes, double frameTime) const;


private:

	/**
	 * Updates the time varying data points.
	 *
	 * @param	x		   	The vector of data points.
	 * @param	frameNumber	The frame number.
	 */
	void UpdateTimeVaryingDataPoints(const Vector& x, int frameNumber);

	/**
	 * Fit plane to base plane points.
	 *
	 * @param	basePlanePoints	The base plane points.
	 * @param	xAxis		   	The x coordinate axis.
	 *
	 * @return	The fitted plane.
	 */
	Plane FitPlaneToBasePlanePoints(const ModellingPoints& basePlanePoints, const Vector3D& xAxis) const;

	/**
	 * Fit the model.  For the given time fit the model to the current guide points.
	 *
	 * @param	time	The time.
	 */
	void FitModel(double time);

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
	
	IModeller *mainApp_;	/**< The main application */
	ModellingModeApex modellingModeApex_;   /**< The modelling mode apex */
	ModellingModeBase modellingModeBase_;   /**< The modelling mode base */
	ModellingModeRV modellingModeRV_;   /**< The modelling mode rv */
	ModellingModeBasePlane modellingModeBasePlane_; /**< The modelling mode base plane */
	ModellingModeGuidePoints modellingModeGuidePoints_; /**< The modelling mode guide points */
	
	ModellingMode* currentModellingMode_;   /**< The current modelling mode */
	
	std::vector< std::vector<double> > timeVaryingDataPoints_;

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
