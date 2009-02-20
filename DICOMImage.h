/*
 * DICOMImage.h
 *
 *  Created on: Feb 11, 2009
 *      Author: jchu014
 */

#ifndef DICOMIMAGE_H_
#define DICOMIMAGE_H_

#include "CAPMath.h"

#include <string>
#include <ostream>
#include <vector>

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

struct Cmiss_texture;

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
	Cmiss_texture* createTextureFromDICOMImage();
	
private:
	std::string filename;
	unsigned int width;
	unsigned int height;
	float thickness;
	float floats[14];
	float pixelSizeX, pixelSizeY;
	
	double timeInCardiacCycle;
	
	ImagePlane* plane;
	Cmiss_texture* texture;
};


struct Graphical_material;

class ImageSlice //should contain info about imagePlane, exnode and exelem (ie. node and element)
{
public:
	ImageSlice(const std::string& name);
	
	void setTime(double time); //actually switch the texture in the material if applicable.
	
private:
	void loadImagePlaneModel();
	
	void transformImagePlane(); //need region& nodenumber
	
	void loadTextures();  // should go to DICOImage??

	std::string sliceName;
	//unsigned int sliceNumber;
	//unsigned int numberOfFrames; //redundant
	
	std::vector<DICOMImage*> images;
	
	Graphical_material* material;
	std::vector<Cmiss_texture*> textures; // should go to DICOMImage?? or might consider having a Texture manager class
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
	
	/** Constructs an image set from a vector of slice names 
	 * @param vector of slice names
	 */
	ImageSet(const std::vector<std::string>& sliceNames_);
	
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

#endif /* DICOMIMAGE_H_ */
