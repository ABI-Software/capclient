/*
 * DICOMImage.h
 *
 *  Created on: Feb 11, 2009
 *      Author: jchu014
 */

#ifndef DICOMIMAGE_H_
#define DICOMIMAGE_H_

#include <string>;

class DICOMImage
{
public:
	DICOMImage(const std::string& filename);

	int getOrientation();
	int getPostion();
	void setContrast();
	void setBrightNess();
//private:
	std::string name;
	unsigned int width;
	unsigned int height;
	float thickness;
	float floats[14];
	float pixelSizeX, pixelSizeY;
};

#include <vector>
#include <ostream>

class ImageSequence
{
	unsigned int numberOfFrames;
	std::vector<DICOMImage*> images;
};

class ImageSlice
{
	unsigned int sliceNumber;
	ImageSequence imageSequence;
};

class ImageGroup // a bunch of image slices i.e LA & SA
{
	unsigned int numberOfImageSlices; //redendant?
	std::vector<ImageSlice*> imageSlices;
	std::string groupName; //either SA or LA
};

class ImageSet // The whole lot ImageManager??
{
	std::vector<ImageGroup*> imageGroups;

	//or
	ImageGroup LA;
	ImageGroup SA;
};

struct Point3D
{
	float x,y,z;
	Point3D(float x_, float y_, float z_)
	: x(x_),y(y_),z(z_)
	{};
	Point3D()
	{};
	friend std::ostream& operator<<(std::ostream &_os, const Point3D &val);
};

inline std::ostream& operator<<(std::ostream &os, const Point3D &val)
{
  os << "( " << val.x << ", " << val.y << ", " << val.z << ") ";
  return os;
};

struct ImagePlane
{
  int registered;

  Point3D tlc;            /**< top left corner */
  Point3D trc;            /**< top right corner */
  Point3D blc;            /**< bottom left corner */
  Point3D brc;            /**< bottom right corner */
  Point3D normal;         /**< normal of the plane */

  Point3D xside;          /**< vector from blc to brc */
  Point3D yside;          /**< vector from blc to tlc */

  int            imageSize;      /**< also stored in StudyInfo - image square */
  float          fieldOfView;    /**< should match length of xside */
  float          sliceThickness; /**< may vary between series */
  float          sliceGap;       /**< non-zero for short axis only */
  float          pixelSizeX;     /**< in mm, should be square */
  float          pixelSizeY;     /**< in mm, should be square */
};

//#include <algorithm>

#define FIND_VECTOR(ans, from, to) \
(ans).x = (to).x - (from).x; \
(ans).y = (to).y - (from).y; \
(ans).z = (to).z - (from).z

#define DOT(a,b) ( (a).x*(b).x + (a).y*(b).y + (a).z*(b).z )

#define CROSS_PRODUCT(ans, vec1, vec2) \
(ans).x = (vec1).y*(vec2).z - (vec1).z*(vec2).y; \
(ans).y = (vec1).z*(vec2).x - (vec1).x*(vec2).z; \
(ans).z = (vec1).x*(vec2).y - (vec1).y*(vec2).x

#define NORMALISE(vec) \
{ \
  double length = sqrt(DOT((vec), (vec))); \
  (vec).x = (vec).x / length; \
  (vec).y = (vec).y / length; \
  (vec).z = (vec).z / length; \
}

ImagePlane* getImagePlaneFromDICOMHeaderInfo(const std::string& filename);

#endif /* DICOMIMAGE_H_ */
