
#ifndef _STANDARDHEARTDEFINITIONS_H
#define _STANDARDHEARTDEFINITIONS_H

namespace cap
{

/**
 * Enum describing the external surfaces of the heart.
 */
enum SurfaceType
{
	EPICARDIUM,
	ENDOCARDIUM,
	UNDEFINED_SURFACE_TYPE,
	MAX_SURFACE_TYPE
};

/**
 * Enum describing points on the heart.
 */
enum DataPointType
{
	APEX,
	BASE,
	RV,
	BASEPLANE,
	GUIDEPOINT,
	UNDEFINED_DATA_POINT_TYPE,
	MAX_PATA_POINT_TYPE
};

}

#endif /* _STANDARDHEARTDEFINITIONS_H */

