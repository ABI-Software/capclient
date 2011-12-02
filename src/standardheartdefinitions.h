
#ifndef _STANDARDHEARTDEFINITIONS_H
#define _STANDARDHEARTDEFINITIONS_H

#include <map>
#include <string>

namespace cap
{

/**
 * Enum describing the external surfaces of the heart.
 */
enum HeartSurfaceEnum
{
	EPICARDIUM,
	ENDOCARDIUM,
	UNDEFINED_HEART_SURFACE_TYPE
};

/**
 * Enum describing points on the heart.
 */
enum ModellingEnum
{
	APEX,
	BASE,
	RV,
	BASEPLANE,
	GUIDEPOINT,
	UNDEFINED_MODELLING_ENUM
};

/**
 * Defines an alias representing the modelling enum map.
 */
typedef std::map<ModellingEnum, std::string> ModellingEnumMap;

/**
 * Initialises the modelling enum strings.
 *
 * @return	A map of modelling enum to string.
 */
ModellingEnumMap InitModellingEnumStrings();

static const ModellingEnumMap ModellingEnumStrings = InitModellingEnumStrings(); /**< The modelling enum strings */

}

#endif /* _STANDARDHEARTDEFINITIONS_H */

