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

namespace cap
{

using namespace std;

ImageSet::ImageSet(const vector<string>& sliceNames)
:
imageSliceNames_(sliceNames)
{
	vector<string>::const_iterator itr = sliceNames.begin();
	for (;itr != sliceNames.end();++itr)
	{
		const string& name = *itr;
		
		ImageSlice* imageSlice = new ImageSlice(name);
		imageSlicesMap_[name] = imageSlice; // use exception safe container or smartpointers
	}
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
			assert(!"No such name in the imageSliceMap_");
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
	else //zero length name string:: set visibility for the whole set
	{
		const std::string& name = imageSliceNames_[index];
		imageSlicesMap_[name]->SetVisible(visible);
	}
}

const ImagePlane& ImageSet::GetImagePlane(const std::string& sliceName) const
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.find(sliceName);
	if (itr == imageSlicesMap_.end())
	{
		//error should probably throw exception
		assert(!"No such name in the imageSliceMap_");
		
		throw std::exception();
	}
	else
	{
		return itr->second->GetImagePlane();
	} 
}

int ImageSet::GetNumberOfFrames() const
{
	std::map<std::string, ImageSlice*>::const_iterator itr = imageSlicesMap_.begin();
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

void ImageSet::WritePlaneInfoToFiles() const
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.begin();
	ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
	for (;itr != end; ++itr)
	{
		std::string filename("Data/images/");
		filename.append(itr->first);
		filename.append(".txt");
		itr->second->WritePlaneInfoToFile(filename);//fix
	}
}

} // end namespace cap
