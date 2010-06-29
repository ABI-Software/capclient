/*
 * CAPTypes.h
 *
 *  Created on: Jun 27, 2010
 *      Author: jchu014
 */

#ifndef CAPTYPES_H_
#define CAPTYPES_H_

#include <string>
#include <vector>
#include <boost/tr1/memory.hpp>
#include <boost/tuple/tuple.hpp>

class Cmiss_texture;
typedef Cmiss_texture* Cmiss_texture_id;

namespace cap
{

class DICOMImage;

typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;
typedef boost::tuple<std::string, std::vector<DICOMPtr>, std::vector<Cmiss_texture_id> > SliceInfo;
typedef std::vector<SliceInfo> SlicesWithImages;


enum SurfaceType
{
	EPICARDIUM,
	ENDOCARDIUM,
	UNDEFINED_SURFACE_TYPE,
	MAX_SURFACE_TYPE
};

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

} // end namespace cap

#endif /* CAPTYPES_H_ */
