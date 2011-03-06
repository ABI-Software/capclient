/*
 * SliceInfo.h
 *
 *  Created on: Jun 27, 2010
 *      Author: jchu014
 */

#ifndef SLICEINFO_H_
#define SLICEINFO_H_

#include "DICOMImage.h"

#include <string>
#include <vector>
#include <boost/tr1/memory.hpp>
#include <boost/bind.hpp>

extern "C"
{
#include "general/value.h"
}

class Cmiss_texture;
typedef Cmiss_texture* Cmiss_texture_id;

namespace cap
{

class SliceInfo
{
public:
	SliceInfo(std::string const& label, std::vector<DICOMPtr> const& dicomImages, std::vector<Cmiss_texture_id> const& textures)
	:
		label_(label),
		dicomImages_(dicomImages),
		textures_(textures)
	{		
	}
	
	std::string const& GetLabel() const
	{
		return label_;
	}
	
	std::vector<DICOMPtr> const& GetDICOMImages() const
	{
		return dicomImages_;
	}
	
	std::vector<Cmiss_texture_id> const& GetTextures() const
	{
		return textures_;
	}
	
	bool ContainsDICOMImage(std::string const& sopiuid) const
	{
		std::vector<DICOMPtr>::const_iterator itr = 
				std::find_if(dicomImages_.begin(), dicomImages_.end(),
						boost::bind(&DICOMImage::GetSopInstanceUID,_1) == sopiuid);
		return itr != dicomImages_.end();
	}
	
private:
	std::string label_;
	std::vector<DICOMPtr> dicomImages_;
	std::vector<Cmiss_texture_id> textures_;
};

typedef std::vector<SliceInfo> SlicesWithImages;

struct SliceInfoSortOrder
// for sorting SliceInfo's
{
	SliceInfoSortOrder()
	{
//		std::cout << __func__ << '\n';
	}

	bool operator()(const SliceInfo& a, const SliceInfo& b) const
	{
		std::string const& x = a.GetLabel();
		std::string const& y = b.GetLabel();
		if (x[0] != y[0])
		{
			// this makes sure short axis slices come first
			return x[0] > y[0];
		}

		return std::make_pair(x.length(), x) < std::make_pair(y.length(), y);
	}
};

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

#endif /* SLICEINFO_H_ */
