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
#include <boost/foreach.hpp>

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
			
			// Create the cmgui specific component that handles the graphical representation
			// of the image slice.
			// The cmgui region which contains the image plane, data points and contour nodes
			// is created by CmguiImageSliceGraphics.
			CmguiImageSliceGraphics* graphics = new CmguiImageSliceGraphics(cmguiManager_, name, itr->GetTextures());
			
			// Create the cmgui nodes which represent contour lines on the images.
			size_t numberOfFrames = itr->GetDICOMImages().size();
			size_t frame = 0;
			BOOST_FOREACH(DICOMPtr const& dicom, itr->GetDICOMImages())
			{
				BOOST_FOREACH(ContourPtr const& contour, dicom->GetContours())
				{
					std::cout << "Creating contour:: Frame numbr = " << frame << '\n';
					double startTime = (double)frame / (double) numberOfFrames;
					double duration = (double)1.0 / numberOfFrames;
					double endTime = startTime + duration;
					
					graphics->CreateContour(contour->GetContourNumber(),
							contour->GetCoordinates(),
							std::make_pair(startTime, endTime),
							contour->GetTransformation());
				}
				frame ++;
			}
			imageSlicesMap[name] = boost::make_shared<ImageSlice>(itr->GetDICOMImages(), graphics);
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
