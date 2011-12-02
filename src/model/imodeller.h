

#ifndef IMODELLER_H_
#define IMODELLER_H_

namespace cap
{
	class Point3D;

	/**
	 * This class is an interface class for the modeller to communicate to the main application with
	 * a limited api.
	 */
	class IModeller
	{
	public:

		/**
		 * Destructor.
		 */
		virtual ~IModeller() {}

		/**
		 * Sets a heart model transformation.
		 *
		 * @param	m	The m.
		 */
		virtual void SetHeartModelTransformation(const gtMatrix& m) = 0;

		/**
		 * Sets a heart model focal length.
		 *
		 * @param	focalLength	Length of the focal.
		 */
		virtual void SetHeartModelFocalLength(double focalLength) = 0;

		/**
		 * Sets a heart model mu from base plane at time.
		 *
		 * @param	plane	The plane.
		 * @param	time 	The time.
		 */
		virtual void SetHeartModelMuFromBasePlaneAtTime(const Plane& plane, double time) = 0;

		/**
		 * Sets a heart model lambda parameters at time.
		 *
		 * @param	lambdaParams	Options for controlling the lambda.
		 * @param	time			The time.
		 */
		virtual void SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time) = 0;

		/**
		 * Gets the number of heart model frames.
		 *
		 * @return	The number of heart model frames.
		 */
		virtual int GetNumberOfHeartModelFrames() const = 0;

		/**
		 * Converts a nodes rc position into a heart model prolate spheriodal coordinate.
		 *
		 * @param	node_id	   	The node identifier.
		 * @param	region_name	Name of the region.
		 *
		 * @return	The position in a prolate shperiodal coordinate system.
		 */
		virtual Point3D ConvertToHeartModelProlateSpheriodalCoordinate(int node_id, const std::string& region_name) const = 0;

		/**
		 * Calculates the heart model xi.
		 *
		 * @param	position  	The position.
		 * @param	time	  	The time.
		 * @param [in,out]	xi	The xi.
		 *
		 * @return	The calculated heart model xi.
		 */
		virtual int ComputeHeartModelXi(const Point3D& position, double time, Point3D& xi) const = 0;

	};

}

#endif /* IMODELLER_H_ */

