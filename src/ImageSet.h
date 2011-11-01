/*
 * ImageSet.h
 *
 *  Created on: Feb 27, 2009
 *      Author: jchu014
 */

#ifndef IMAGESET_H_
#define IMAGESET_H_

//class ImageGroup // a bunch of image slices i.e LA & SA
//{
//	unsigned int numberOfImageSlices_; //Redundant?
//	std::vector<ImageSlice*> imageSlices_;
//	std::string groupName_; //either SA or LA
//};
#include "SliceInfo.h"

#include <map>
#include <vector>
#include <string>
#include <boost/tr1/memory.hpp>

namespace cap
{

class ImagePlane;
class ImageSlice;
class CmguiPanel;

typedef std::map<std::string, boost::shared_ptr<ImageSlice> > ImageSlicesMap;

/**
 * 
 */
class ImageSet // The whole lot : ImageManager??
{
public:
	
	/** 
	 * Constructs an image set from a data structure of type SlicesWithImages.
	 * 
	 * \param slices slices with images to be used as textures.
	 * \param imageSliceNames vector of slice names.
	 * \param map an image slice map using a key of image name.
	 */
	
	ImageSet(SlicesWithImages const& slices, 
			std::vector<std::string> const& imageSliceNames,
			ImageSlicesMap const& map);
	
	/** 
	 * Sets time for the whole image set.
	 * 
	 * \param time in a cardiac cycle in seconds.
	 */ 
	void SetTime(double time);
	
	/** 
	 * Sets the visibility for a slice.
	 * 
	 * \param visible visibility 
	 * \param sliceName name of the slice e.g LA1, SA2, ... default = all
	 */
	void SetVisible(bool visible, const std::string& sliceName = std::string() ); 
	
	/** 
	 * Sets the visibility for a slice.
	 * 
	 * \param visible visibility default 
	 * \param index index of the slice
	 */
	void SetVisible(bool visible, unsigned int index ); 
	
	bool IsVisible(const std::string& sliceName) const;
	
	void SetBrightness(float brightness);
	
	void SetContrast(float contrast);
	
	/** 
	 * Get the position and orientation of the image slice by name.
	 * 
	 * \param name
	 */
	const ImagePlane& GetImagePlane(const std::string& sliceName) const;
	
	unsigned int GetNumberOfSlices() const
	{
		return imageSlicesMap_.size();
	}
	
	const std::vector<std::string>& GetSliceNames() const
	{
		return imageSliceNames_;
	}
	
	/** 
	 * Get the number of logical frames. 
	 * Currently this is just the smallest number of frames of all slices.
	 */
	int GetNumberOfFrames() const;
	
	void SetShiftedImagePosition();
	
	SlicesWithImages const& GetSlicesWithImages() const
	{
		return slicesWithImages_;
	}
	
	std::string const& GetPatientID() const
	{
		assert(slicesWithImages_.at(0).GetDICOMImages().at(0));
		return slicesWithImages_.at(0).GetDICOMImages().at(0)->GetPatientID();;
	}
	
private:
	std::vector<std::string> imageSliceNames_; /**< so we can access the slices by indices */
//	std::vector<ImageGroup*> imageGroups;
//
//	//or
//	ImageGroup LA;
//	ImageGroup SA;
	
	// or
	//std::vector<ImageSlice*> imageSlices_;
	
	//or
	ImageSlicesMap imageSlicesMap_;

	SlicesWithImages slicesWithImages_;
	
};

} // end namespace cap
#endif /* IMAGESET_H_ */
