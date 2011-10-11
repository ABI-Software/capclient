/*
 * UnitTestImageSet.cpp
 *
 *  Created on: Jan 13, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#include "ImageSet.h"
#include "ImageSlice.h"
#include "ImageSliceGraphics.h"

class TestImageSliceGraphics : public cap::ImageSliceGraphics
{
	virtual void SetVisible(bool visibility)
	{
	}
	
	virtual void ChangeTexture(size_t index)
	{
	}
	
	virtual void SetBrightness(float brightness)
	{
	}
	
	virtual void SetContrast(float contrast)
	{
	}
	
	virtual cap::Point3D GetTopLeftCornerPosition()
	{
		return cap::Point3D();
	}
	
	virtual void TransformTo(cap::ImagePlane* plane)
	{
	}
};

TEST(ImageSetTest, Create)
{
	cap::ImageSliceGraphics* graphics = new TestImageSliceGraphics;
}
