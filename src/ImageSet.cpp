/*
 * ImageSet.cpp
 *
 *  Created on: Feb 27, 2009
 *      Author: jchu014
 */

#include "ImageSet.h"
#include "ImageSlice.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

namespace cap
{

using namespace std;

ImageSet::ImageSet(SlicesWithImages const& slices, 
		std::vector<std::string> const& imageSliceNames,
		ImageSlicesMap const& map)
:
	slicesWithImages_(slices),
	imageSliceNames_(imageSliceNames),
	imageSlicesMap_(map)
{
	SetBrightness(0.5);
	SetContrast(0.5);
	SetTime(0.0);
}

void ImageSet::SetTime(double time)
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.begin();
	ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
	for (;itr != end; ++itr)
	{
		itr->second->SetTime(time);
	}
	return;
}

void ImageSet::SetBrightness(float brightness)
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.begin();
	ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
	for (;itr != end; ++itr)
	{
		itr->second->SetBrightness(brightness);
	}
	return;
}

void ImageSet::SetContrast(float contrast)
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.begin();
	ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
	for (;itr != end; ++itr)
	{
		itr->second->SetContrast(contrast);
	}
	return;
}

void ImageSet::SetVisible(bool visible, const std::string& sliceName)
{
	if (sliceName.length())
	{
		ImageSlicesMap::iterator itr = imageSlicesMap_.find(sliceName);
		if (itr == imageSlicesMap_.end())
		{
			//error should probably throw exception
			std::cout << __func__ << ": No such name in the imageSliceMap_ : " << sliceName << '\n' ;
			
			throw std::exception();
		}
		else
		{
			itr->second->SetVisible(visible);
		}
	}
	else //zero length name string:: set visibility for the whole set
	{
		ImageSlicesMap::iterator itr = imageSlicesMap_.begin();
		ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
		for (;itr!=end;++itr)
		{
			itr->second->SetVisible(visible);
		}
	}
}

void ImageSet::SetVisible(bool visible, int index)
{
	if (imageSlicesMap_.size() <= index || index < 0)
	{
		assert(!"Index out of bound: imageSliceMap_");
	}
	else
	{
		const std::string& name = imageSliceNames_[index];
		imageSlicesMap_[name]->SetVisible(visible);
	}
}
bool ImageSet::IsVisible(const std::string& sliceName) const
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.find(sliceName);
	if (itr == imageSlicesMap_.end())
	{
		//error should probably throw exception
		std::cout << __func__ << ": No such name in the imageSliceMap_ : " << sliceName << '\n' ;
		throw std::exception();
	}
	else
	{
		return itr->second->IsVisible();
	} 
}

const ImagePlane& ImageSet::GetImagePlane(const std::string& sliceName) const
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.find(sliceName);
	if (itr == imageSlicesMap_.end())
	{
		//error should probably throw exception
		std::cout << __func__ << ": No such name in the imageSliceMap_ : " << sliceName << '\n' ;
		
		throw std::exception();
	}
	else
	{
		return itr->second->GetImagePlane();
	} 
}

int ImageSet::GetNumberOfFrames() const
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.begin();
	int numberOfFrames = itr->second->GetNumberOfFrames();
	++itr;
	for(;itr!=imageSlicesMap_.end();++itr)
	{
		if (numberOfFrames > itr->second->GetNumberOfFrames())
		{
			numberOfFrames = itr->second->GetNumberOfFrames();
		}
	}
	return numberOfFrames;
}

void ImageSet::SetShiftedImagePosition()
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.begin();
	ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
	for (;itr != end; ++itr)
	{
		itr->second->SetShiftedImagePosition();
	}
}

} // end namespace cap
