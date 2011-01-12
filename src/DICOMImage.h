/*
 * DICOMImage.h
 *
 *  Created on: Feb 11, 2009
 *      Author: jchu014
 */

#ifndef DICOMIMAGE_H_
#define DICOMIMAGE_H_

#include "CAPMath.h"
#include "CAPContour.h"

#include <string>
#include <vector>
#include <boost/tr1/memory.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>

//struct Cmiss_texture;

namespace cap
{

class DICOMImage : boost::noncopyable
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
	
	std::pair<Vector3D,Vector3D> GetImageOrientation() const
	{
		return std::make_pair(orientation1_, orientation2_);
	}
	
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
	
	std::string const& GetStudyInstanceUID() const
	{
		return studyInstanceUID_;
	}
	
	std::string const& GetSopInstanceUID() const
	{
		return sopInstanceUID_;
	}
	
	std::string const& GetSeriesInstanceUID() const
	{
		return seriesInstanceUID_;
	}
	
	int GetInstanceNumber() const
	{
		return instanceNumber_;
	}
	
	bool IsShifted() const
	{
		return isShifted_;
	}

	bool IsRotated() const
	{
		return isRotated_;
	}
	
//	void SetShifted(bool shifted)
//	{
//		isShifted_ = shifted;
//	}

	Point3D const& GetShiftedImagePosition() const
	{
		return shiftedPosition_;
	}

	void SetShiftedImagePosition(Point3D const& p)
	{
		isShifted_ = true;
		shiftedPosition_ = p;
		ComputeImagePlane();
	}
	
	void SetImagePlaneTLC(Point3D const& p)
	{
		//covert tlc to ImagePosition
		Point3D pos = p + 0.5 * pixelSizeX_ * orientation1_ + 0.5f * pixelSizeY_ * orientation2_;
		SetShiftedImagePosition(pos);
	}

	void ComputeImagePlane();
	
	std::pair<Vector3D,Vector3D> GetShiftedImageOrientation() const
	{
		return std::make_pair(shiftedOrientation1_, shiftedOrientation2_);
	}

	void SetShiftedImageOrientation(Vector3D const& v1, Vector3D const& v2)
	{
		isRotated_ = true;
		// cosine vectors
		shiftedOrientation1_ = v1;
		shiftedOrientation2_ = v2;
	}

	//FIXME
	std::vector<ContourPtr>& GetContours()
	{
		return contours_;
	}
//	
	void AddContour(ContourPtr const& con)
	{
		contours_.push_back(con);
	}
	
	void SetContourVisibility(bool visibility)
	{
		std::for_each(contours_.begin(), contours_.end(), 
				boost::bind(&CAPContour::SetVisibility, _1, visibility));
	}
	
private:
	void ReadDICOMFile();
	
	std::string filename_;
	unsigned int width_;
	unsigned int height_;
//	double thickness_;
	double pixelSizeX_, pixelSizeY_;
	
//	double timeInCardiacCycle;
	
	std::string studyInstanceUID_;
	std::string sopInstanceUID_;
	std::string seriesInstanceUID_;
	std::string seriesDescription_;
	std::string sequenceName_;
	double triggerTime_;
	int seriesNumber_;
	Point3D position3D_;
	Vector3D orientation1_, orientation2_;
	
	Point3D shiftedPosition_;
	Vector3D shiftedOrientation1_, shiftedOrientation2_;

	int instanceNumber_;
	
	std::string patientName_;
	std::string patientId_;
	std::string scanDate_;
	std::string dateOfBirth_;
	std::string gender_;
	std::string age_;
	
	ImagePlane* plane_;
//	Cmiss_texture* texture_;
	std::vector<ContourPtr> contours_; //FIXME
	
	bool isShifted_;
	bool isRotated_;
};

//class DICOMImage;

typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;

} // end namespace cap
#endif /* DICOMIMAGE_H_ */
