/*
 * ImageSlice.cpp
 *
 *  Created on: May 19, 2009
 *      Author: jchu014
 */

#include "ImageSlice.h"

#include "DICOMImage.h"
#include "ImageSliceGraphics.h"

#include <iostream>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

using namespace std;

namespace cap
{

ImageSlice::ImageSlice(std::vector<DICOMPtr> const& images, ImageSliceGraphics* graphics)
	:
	oldIndex_(-1),
	isVisible_(true),
	images_(images),
	graphics_(graphics)
{
	size_t numberOfFrames = images_.size();
	for (size_t frame = 0; frame < numberOfFrames; ++ frame)
	{
		std::vector<ContourPtr>& contours = images_.at(frame)->GetContours();
		BOOST_FOREACH(ContourPtr& capContour, contours)
		{
//			capContour->ReadFromExFile(cmguiManager.GetCmissContext());
			double startTime = (double)frame / (double) numberOfFrames;
			double duration = (double)1.0 / numberOfFrames;
			double endTime = startTime + duration;
			capContour->SetValidPeriod(startTime, endTime);
			capContour->SetVisibility(true);
		}
	}

	this->TransformImagePlane();
}

ImageSlice::~ImageSlice()
{
	delete graphics_;
}

namespace {
	size_t MapTimeToIndex(double time, size_t totalNumberOfFrames)
	{
		size_t index = static_cast<int>(time * totalNumberOfFrames); // -1
		//boundary checks
		if (index >= totalNumberOfFrames)
		{
			index = totalNumberOfFrames - 1;
		}
		else if (index < 0)
		{
			index = 0;
		}
		
		return index;
	}
}

void ImageSlice::SetVisible(bool visibility)
{
	if (visibility)
	{
		isVisible_ = true;
		oldIndex_ = -1; //this forces rebinding & redraw of the texture
		SetTime(time_);
	}
	else
	{
		isVisible_ = false;
	}
	graphics_->SetVisible(visibility);
	
	//TEST
	// Set visibility of contours
	std::for_each(images_.begin(), images_.end(), 
			boost::bind(&DICOMImage::SetContourVisibility,_1,visibility));
	
	return;
}

void ImageSlice::SetTime(double time)
{
	time_ = time; // store time for later use

	size_t index = MapTimeToIndex(time, images_.size());
	
	//update texture only when it is necessary
	if (index == oldIndex_|| !isVisible_)
	{
		return; 
	}
	oldIndex_ = index;
	
	//DEBUG
	//cout << "ImageSlice::setTime index = " << index << endl;
		
	graphics_->ChangeTexture(index);
	return ;
}

void ImageSlice::SetBrightness(float brightness)
{
	graphics_->SetBrightness(brightness);
}

void ImageSlice::SetContrast(float contrast)
{
	graphics_->SetContrast(contrast);
}

void ImageSlice::TransformImagePlane()
{
	// Now get the necessary info from the DICOM header
	
	assert(!images_.empty());
	DICOMImage& dicomImage = *images_[0]; //just use the first image in the slice
	ImagePlane* plane = dicomImage.GetImagePlaneFromDICOMHeaderInfo();
	if (!plane)
	{
		cout << "ERROR !! plane is null"<<endl;
	}
	else
	{
		cout << plane->tlc << endl;
		imagePlane_ = plane;
	}
	
	graphics_->TransformTo(imagePlane_);
}

const ImagePlane& ImageSlice::GetImagePlane() const
{
	return *(imagePlane_);
}

void ImageSlice::SetShiftedImagePosition()
{	
	Point3D tlc = graphics_->GetTopLeftCornerPosition(); // TODO error handling
	std::for_each(images_.begin(), images_.end(), boost::bind(&DICOMImage::SetImagePlaneTLC, _1, tlc));
}

} // end namespace cap
