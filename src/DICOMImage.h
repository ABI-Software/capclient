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
	explicit DICOMImage(const std::string& filename);
	~DICOMImage()
	{
		if (plane_)
		{
			delete plane_;
		}
	}

	std::string const& GetFilename()
	{
		return filename_;
	}
	
	std::pair<Vector3D,Vector3D> GetImageOrientation() const;
	
	Point3D const& GetImagePosition() const
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
	
	std::string const& GetPatientName() const
	{
		return patientName_;
	}
	
	std::string const& GetPatientID() const
	{
		return patientId_;
	}
	
	std::string const& GetScanDate() const
	{
		return scanDate_;
	}
	
	std::string const& GetDateOfBirth() const
	{
		return dateOfBirth_;
	}
	
	std::string const& GetGender() const
	{
		return gender_;
	}
	
	std::string const& GetAge() const
	{
		return age_;
	}
	
//	void SetContrast();
//	void SetBrightNess();
	ImagePlane* GetImagePlaneFromDICOMHeaderInfo() const;
//	Cmiss_texture* CreateTextureFromDICOMImage();
	
	int GetSeriesNumber() const
	{
		return seriesNumber_;
	}
	
	size_t GetImageWidth() const
	{
		return width_;
	}
	
	size_t GetImageHeight() const
	{
		return height_;
	}
	
	bool operator<(DICOMImage const& other) const // Better to make this a non member ?
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
	
//	double timeInCardiacCycle;
	
	std::string studyInstanceUID_;
	std::string sopInstanceUID_;
	std::string seriesDescription_;
	std::string sequenceName_;
	double triggerTime_;
	int seriesNumber_;
	Point3D position3D_;
	Vector3D orientation1_, orientation2_;
	
	std::string patientName_;
	std::string patientId_;
	std::string scanDate_;
	std::string dateOfBirth_;
	std::string gender_;
	std::string age_;
	
	mutable ImagePlane* plane_;
//	Cmiss_texture* texture;
};

} // end namespace cap
#endif /* DICOMIMAGE_H_ */
