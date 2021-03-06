

#ifndef MODELLINGPOINT_H_
#define MODELLINGPOINT_H_

#include <vector>
#include <map>

extern "C"
{
#include <zn/cmiss_region.h>
#include <zn/cmiss_node.h>
}

#include "math/algebra.h"
#include "standardheartdefinitions.h"

namespace cap
{
	struct ModellingPointDetail
	{
		ModellingPointDetail(ModellingEnum type, Point3D position, double time)
			: modellingPointType_(type)
			, position_(position)
			, time_(time)
		{
		}

		std::string  GetModellingPointTypeString() const
		{
			return ModellingEnumStrings.find(modellingPointType_)->second;
		}

		ModellingEnum modellingPointType_; /**< Type of the modelling point */
		Point3D position_;  /**< The position */
		double time_;   /**< The time */
	};

	/**
	 * @brief Modelling point.  This class encapsultates the points for modelling.  It holds a region
	 * handle to the region of the modelling point as well as a node id in that region from which
	 * the position is derived from.  We use an external Remove() function to delete the node from
	 * Cmgui rather than the destructor for this class.  If Remove() was put into the destructor then
	 * we would have to add a create to the copy constructor and assignment operator.
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
		 * Set the coordinates of the node to the given position, also update the position_.
		 *
		 * @param location	The location to set the coordinates to.
		 */
		void SetCoordinates(const Point3D& location);

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

		/**
		 * Gets the modelling point type as a string.
		 *
		 * @return	The modelling point type string.
		 */
		std::string GetModellingPointTypeString() const;

		/**
		 * Gets the heart surface type.
		 *
		 * @return	The heart surface type.
		 */
		HeartSurfaceEnum GetHeartSurfaceType() const { return heartSurfaceType_; }

		/**
		 * Sets the heart surface type.
		 *
		 * @param	type	The type.
		 */
		void SetHeartSurfaceType(HeartSurfaceEnum type);

		/**
		 * Gets the node identifier.
		 *
		 * @return	The node identifier.
		 */
		int GetNodeIdentifier() const { return node_id_; }

		/**
		 * If the modelling point is on the given plane, then add the given label to the list of planes
		 * this modelling point is attached to.
		 *
		 * @param label	The label to add the the list of attached to planes.
		 * @param location	The location of the plane.
		 * @param normal	The normal of the plane.
		 */
		void AttachToIfOn(const std::string& label, const Point3D& location, const Vector3D& normal);

		/**
		 * Test to see if the given label is in the list of attachedTo_ planes.
		 *
		 * @param label	The label to test.
		 * @return true if the label is in the list of attachedTo_ planes, false otherwise.
		 */
		bool IsAttachedTo(const std::string& label);

		/**
		 * Clear attachedTo_ list.
		 */
		void ClearAttachedToList() { attachedTo_.clear(); }

		/**
		 * Get the list of attached to strings for this modelling point.
		 *
		 * @return A vector of strings .
		 */
		std::vector<std::string> GetAttachedTo() const { return attachedTo_; }

	private:
		/**
		 * Set the node visibility.
		 *
		 * @param visibility    the visibility of the node to set.
		 */
		void SetNodeVisibility(bool visibility);

		/**
		 * Set the spectrum value into the spectrum_value_field.
		 *
		 * @param spectrumValue the value for the spectrum value field.
		 */
		void SetSpectrumValue(double spectrumValue);

		/**
		 * Get the value represented by heartSurfaceType_ as a double
		 * for the node spectrum.  Only the guidepoint type of modelling
		 * point cares about this value.
		 *
		 * @return the value for the heartSurfaceType_.
		 */
		double GetSpectrumValue() const;

	protected:
		ModellingEnum modellingPointType_; /**< Type of the modelling point */
		HeartSurfaceEnum heartSurfaceType_; /**< Type of the heart surface */
		std::vector<std::string> attachedTo_; /**< A list of surfaces the modelling point is attached to */
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
	 * Defines an alias representing a vector of modelling point details.
	 */
	typedef std::vector<ModellingPointDetail> ModellingPointDetails;

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
