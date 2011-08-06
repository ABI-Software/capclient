/*
 * ImageSlice.h
 *
 *  Created on: May 19, 2009
 *      Author: jchu014
 */

#ifndef IMAGESLICE_H_
#define IMAGESLICE_H_

#include "DICOMImage.h"
#include <vector>
#include <boost/noncopyable.hpp>

namespace cap
{

class ImagePlane;
class ImageSliceGraphics;

// move Textures to another class??
class ImageSlice : boost::noncopyable
{
public:	
	ImageSlice(std::vector<DICOMPtr> const& images, ImageSliceGraphics* graphics);
	
	~ImageSlice();
	
	void SetTime(double time); //actually switch the texture in the material if applicable.
	
	void SetVisible(bool visibility);
	
	bool IsVisible() const
	{
		return isVisible_;
	};
	
	void SetBrightness(float brightness);
	
	void SetContrast(float contrast);
	
	const ImagePlane& GetImagePlane() const;
	
	size_t GetNumberOfFrames() const
	{
		return images_.size();
	}
	
	void SetShiftedImagePosition();
	
	std::vector<DICOMPtr> const& GetImages() const
	{
		return images_;
	}
	
private:
	void TransformImagePlane(); //need region& nodenumber
	
//	std::string sliceName_;
	//unsigned int sliceNumber;
	//unsigned int numberOfFrames; //redundant
	
	bool isVisible_;
	double time_;
	
	std::vector<DICOMPtr> images_;
		
	ImagePlane* imagePlane_; //Redundant?? its in DICOMImage
	
	size_t oldIndex_; //used to check if texture switch is needed
	
	ImageSliceGraphics* graphics_; // graphics component
};

} // end namespace cap
#endif /* IMAGESLICE_H_ */
