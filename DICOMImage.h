/*
 * DICOMImage.h
 *
 *  Created on: Feb 11, 2009
 *      Author: jchu014
 */

#ifndef DICOMIMAGE_H_
#define DICOMIMAGE_H_

#include <string>
#include <ostream>
#include <vector>

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

class DICOMImage
{
public:
	DICOMImage(const std::string& filename);
	~DICOMImage()
	{
		delete plane;
	}

	int getOrientation();
	int getPostion();
	void setContrast();
	void setBrightNess();
	ImagePlane* getImagePlaneFromDICOMHeaderInfo();
	
private:
	std::string name;
	unsigned int width;
	unsigned int height;
	float thickness;
	float floats[14];
	float pixelSizeX, pixelSizeY;
	
	double timeInCardiacCycle;
	
	ImagePlane* plane;
};

class ImageSlice //should contain info about imagePlane, exnode and exelem (ie. node and element)
{
public:
	void setTime(double time); //actually switch the texture in the material if applicable.
	void transformImagePlane(); //need region& nodenumber
private:
	std::string sliceName;
	unsigned int sliceNumber;
	unsigned int numberOfFrames;
	std::vector<DICOMImage*> images;
};

class ImageGroup // a bunch of image slices i.e LA & SA
{
	unsigned int numberOfImageSlices; //Redundant?
	std::vector<ImageSlice*> imageSlices;
	std::string groupName; //either SA or LA
};

#include <map>

class ImageSet // The whole lot ImageManager??
{
public:
	
	/** Sets time for the whole image set
	 * @param time in a cardiac cycle in second
	 */ 
	void setTime(double time);
	
	/** Sets the visibility for a slice.
	 * @param sliceName name of the slice e.g LA1, SA2, ...
	 * @paran visible visibility default = true
	 */
	void setVisible(const std::string& sliceName, bool visible = true); 
	
	std::vector<ImageGroup*> imageGroups;

	//or
	ImageGroup LA;
	ImageGroup SA;
	
	// or
	std::vector<ImageSlice*> imageSlices;
	
	//or
	std::map<std::string, ImageSlice*> imageSliceMap;
};

//#include <algorithm>

//#define FIND_VECTOR(ans, from, to) \
//(ans).x = (to).x - (from).x; \
//(ans).y = (to).y - (from).y; \
//(ans).z = (to).z - (from).z

template <typename T>
inline void FIND_VECTOR(T& ans, T& from, T& to)
{
	ans.x = to.x - from.x;
	ans.y = to.y - from.y;
	ans.z = to.z - from.z;
}

//#define DOT(a,b) ( (a).x*(b).x + (a).y*(b).y + (a).z*(b).z )
template <typename T>
inline float DOT(T& a, T& b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

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

//ImagePlane* getImagePlaneFromDICOMHeaderInfo(const std::string& filename);

#endif /* DICOMIMAGE_H_ */
