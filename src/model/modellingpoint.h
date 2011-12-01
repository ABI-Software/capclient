

#ifndef MODELLINGPOINT_H_
#define MODELLINGPOINT_H_

#include <vector>
#include <map>

extern "C"
{
#include <api/cmiss_region.h>
#include <api/cmiss_node.h>
}

#include "math/algebra.h"
#include "standardheartdefinitions.h"

namespace cap
{
	/**
	 * Modelling point.  This class encapsultates the points for modelling.  It holds a region
	 * handle to the region of the modelling point as well as a node id in that region from which
	 * the position is derived from.
	 */
	class ModellingPoint
	{
	public:

		/**
		 * Default constructor.
		 */
		ModellingPoint();

		/**
		 * Constructor.
		 *
		 * @param	modellingPointType	Type of the modelling point.
		 * @param	region			  	The region.
		 * @param	node_id			  	Identifier for the node.
		 * @param	position		  	The position.
		 * @param	time			  	(optional) the time.
		 */
		ModellingPoint(ModellingEnum modellingPointType, Cmiss_region_id region, int node_id, const Point3D& position, double time = -1.0);

		/**
		 * Destructor.
		 */
		virtual ~ModellingPoint(void);

		/**
		 * Copy constructor.
		 *
		 * @param	other	The other.
		 */
		ModellingPoint(const ModellingPoint& other);

		/**
		 * Assignment operator.
		 *
		 * @param	rhs	The right hand side.
		 *
		 * @return	A shallow copy of this object.
		 */
		ModellingPoint& operator=(const ModellingPoint& rhs);

		/**
		 * Sets the modelling point visible if visibility is true and invisible otherwise.
		 *
		 * @param	visibility	true to make the modelling point visible.
		 */
		void SetVisible(bool visibility);

		/**
		 * Removes this node from the node group.  This function will destroy the node and make any
		 * references to it invalid.  Use this function just before deleting but not in the destructor
		 * as the destructor will be called when creating copies and then the node will be removed prematurely.
		 */
		void Remove();

		/**
		 * Sets a position.
		 *
		 * @param	position	The position.
		 */
		void SetPosition(const Point3D& position) { position_ = position; }

		/**
		 * Gets the position.
		 *
		 * @return	The position.
		 */
		const Point3D& GetPosition() const { return position_; }

		/**
		 * Gets the time.
		 *
		 * @return	The time.
		 */
		double GetTime() const { return time_; }

		/**
		 * Gets the modelling point type.
		 *
		 * @return	The modelling point type.
		 */
		ModellingEnum GetModellingPointType() const { return modellingPointType_; }

	protected:
		ModellingEnum modellingPointType_; /**< Type of the modelling point */
		Cmiss_region_id region_;	/**< The region */
		int node_id_;   /**< Identifier for the node */
		Point3D position_;  /**< The position */
		double weight_; /**< The weight */
		double time_;   /**< The time */
	};

	/**
	 * Defines an alias representing the modelling points.
	 */
	typedef std::map<int, ModellingPoint> ModellingPointsMap;

	/**
	 * Defines an alias representing the modelling points.
	 */
	typedef std::vector<ModellingPoint> ModellingPoints;

	/**
	 * Modelling point time less than.  Used for sorting ModellingPoints with respect to time.
	 */
	struct ModellingPointTimeLessThan
	{
		/**
		 * bool casting operator.
		 */
		bool operator() (const ModellingPoint& i, const ModellingPoint& j)  const
		{
			return (i.GetTime() < j.GetTime());
		}
	};


}

#endif /* MODELLINGPOINT_H_ */
