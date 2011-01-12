/*
 * ImageSliceGraphics.h
 *
 *  Created on: Jan 12, 2011
 *      Author: jchu014
 */

#ifndef IMAGESLICEGRAPHICS_H_
#define IMAGESLICEGRAPHICS_H_

struct Cmiss_texture;

namespace cap
{

class ImagePlane;
class Point3D;

class ImageSliceGraphics
{
public:
	virtual ~ImageSliceGraphics() {};
	virtual void SetVisible(bool visibility) = 0;
	virtual void ChangeTexture(Cmiss_texture* tex) = 0;
	virtual void SetBrightness(float brightness) = 0;
	virtual void SetContrast(float contrast) = 0;
	virtual Point3D GetTopLeftCornerPosition() = 0;
	virtual void TransformTo(ImagePlane* plane) = 0;
};
} // namespace cap

#endif /* IMAGESLICEGRAPHICS_H_ */
