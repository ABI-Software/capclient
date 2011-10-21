/*
 * ImageSliceGraphics.h
 *
 *  Created on: Jan 12, 2011
 *      Author: jchu014
 */

#ifndef IMAGESLICEGRAPHICS_H_
#define IMAGESLICEGRAPHICS_H_

#include <stddef.h>

namespace cap
{

class ImagePlane;
class Point3D;

/**
 * Pure abstract class that is an interface for image slice graphics.
 */
class ImageSliceGraphics
{
public:
	virtual ~ImageSliceGraphics() {};
	virtual void SetVisible(bool visibility) = 0;
	virtual void ChangeTexture(size_t index) = 0;
	virtual void SetBrightness(float brightness) = 0;
	virtual void SetContrast(float contrast) = 0;
	virtual Point3D GetTopLeftCornerPosition() = 0;
	virtual void TransformTo(ImagePlane* plane) = 0;
};
} // namespace cap

#endif /* IMAGESLICEGRAPHICS_H_ */
