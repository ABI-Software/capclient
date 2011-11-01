/*
 * SliceInfo.h
 *
 *  Created on: Jun 27, 2010
 *      Author: jchu014
 */

#ifndef SLICEINFO_H_
#define SLICEINFO_H_

#include <string>
#include <vector>
#include <boost/tr1/memory.hpp>
#include <boost/bind.hpp>

#include <api/cmiss_graphics_material.h>

#include "capclientconfig.h"
#include "standardheartdefinitions.h"
#include "DICOMImage.h"

extern "C"
{
}

namespace cap
{

/**
 * Contains information to fully describe the slice; a label, a vector of dicom images and
 * a vector of textures.
 */
class SliceInfo
{
public:
	SliceInfo(std::string const& label, std::vector<DICOMPtr> const& dicomImages, std::vector< Cmiss_field_image_id > textures)
		: label_(label)
		, dicomImages_(dicomImages)
		, textures_(textures)
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
	
	std::vector<Cmiss_field_image_id> GetTextures() const
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
	std::vector<Cmiss_field_image_id> textures_;
};

typedef std::vector<SliceInfo> SlicesWithImages; /**< A std::vector of SliceInfo */

/**
 * Slice info sort order overrides the operator() so that 
 * short axis slices come first.
 */
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

} // end namespace cap

#endif /* SLICEINFO_H_ */
