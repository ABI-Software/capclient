/*
 * ImageSetBuilder.h
 *
 *  Created on: Jan 11, 2011
 *      Author: jchu014
 */

#ifndef IMAGESETBUILDER_H_
#define IMAGESETBUILDER_H_

#include "SliceInfo.h"
#include "ImageSet.h"
#include "ImageSlice.h"
#include "CmguiImageSliceGraphics.h"
#include <boost/make_shared.hpp>

namespace cap
{

class CmguiManager;
class ImageSet;

// This is the class responsible for creating the image set
// along with the image slices that the image set owns.

class ImageSetBuilder
{
public:
	ImageSetBuilder(SlicesWithImages const& slices, CmguiManager const& cmguiManager)
	:
		slices_(slices),
		cmguiManager_(cmguiManager)
	{
	}
	
	ImageSet* build()
	{
		ImageSlicesMap imageSlicesMap;
		std::vector<std::string> imageSliceNames;
		
		SlicesWithImages::const_iterator itr = slices_.begin();
		SlicesWithImages::const_iterator end = slices_.end();
		for (;itr != end; ++itr)
		{
			std::string const& name(itr->GetLabel());
			ImageSliceGraphics* graphics = new CmguiImageSliceGraphics(cmguiManager_, name);
			imageSlicesMap[name] = boost::make_shared<ImageSlice>(*itr, graphics);
			imageSliceNames.push_back(name);
		}
		
		return new ImageSet(slices_, imageSliceNames, imageSlicesMap);
	}
	
private:
	SlicesWithImages const& slices_;
	CmguiManager const& cmguiManager_;
};


} // namespace cap

#endif /* IMAGESETBUILDER_H_ */
