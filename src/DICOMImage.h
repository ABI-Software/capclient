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

struct Cmiss_texture;

namespace cap
{

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
//	float          pixelSizeX_;     /**< in mm, should be square */
//	float          pixelSizeY_;     /**< in mm, should be square */
	float d; // the constant of the plane equation ax + by + cz = d (a,b & c are the 3 components of this->normal)
};

class DICOMImage
{
public:
	DICOMImage(const std::string& filename);
	~DICOMImage()
	{
		delete plane_;
	}

	std::pair<Vector3D,Vector3D> GetOrientation() const;
	Point3D const& GetPosition() const
	{
		return position3D_;
	}
	
	std::string const& GetSeriesDescription() const
	{
		return seriesDescription_;
	}
	
	std::string const& GetSequenceName() const
	{
		return sequenceName_;
	}
	
	double GetTriggerTime() const
	{
		return triggerTime_;
	}
	
//	void SetContrast();
//	void SetBrightNess();
	ImagePlane* GetImagePlaneFromDICOMHeaderInfo();
//	Cmiss_texture* CreateTextureFromDICOMImage();
	
	int GetSeriesNumber() const
	{
		return seriesNumber_;
	}
	
	bool operator<(DICOMImage const& other) const
	{
		if (triggerTime_ >= 0.0 && other.triggerTime_ >= 0.0)
		{
			return triggerTime_ < other.triggerTime_;
		}
		else
		{
			return filename_ < other.filename_;
		}
	}
	
private:
	void ReadDICOMFile();
	
	std::string filename_;
	unsigned int width_;
	unsigned int height_;
	float thickness_;
	float pixelSizeX_, pixelSizeY_;
	
	double timeInCardiacCycle;
	
	std::string studyInstanceUID_;
	std::string sopInstanceUID_;
	std::string seriesDescription_;
	std::string sequenceName_;
	double triggerTime_;
	int seriesNumber_;
	Point3D position3D_;
	Vector3D orientation1_, orientation2_;
	
	ImagePlane* plane_;
//	Cmiss_texture* texture;
};

} // end namespace cap
#endif /* DICOMIMAGE_H_ */
