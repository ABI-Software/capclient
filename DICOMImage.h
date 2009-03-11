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

class ImagePlane
{
public:
	Point3D tlc;            /**< top left corner */
	Point3D trc;            /**< top right corner */
	Point3D blc;            /**< bottom left corner */
	Point3D brc;            /**< bottom right corner */
	Vector3D normal;         /**< normal of the plane */
	
	Vector3D xside;          /**< vector from blc to brc */
	Vector3D yside;          /**< vector from blc to tlc */
	
//	int            imageSize;      /**< also stored in StudyInfo - image square */
//	float          fieldOfView;    /**< should match length of xside */
//	float          sliceThickness; /**< may vary between series */
//	float          sliceGap;       /**< non-zero for short axis only */
//	float          pixelSizeX;     /**< in mm, should be square */
//	float          pixelSizeY;     /**< in mm, should be square */
	float d; // the constant of the plane equation ax + by + cz = d (a,b & c are the 3 components of this->normal)
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

	int GetOrientation();
	int GetPostion();
	void SetContrast();
	void SetBrightNess();
	ImagePlane* GetImagePlaneFromDICOMHeaderInfo();
	Cmiss_texture* CreateTextureFromDICOMImage();
	
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

#endif /* DICOMIMAGE_H_ */
