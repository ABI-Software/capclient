/*
 * DICOMImage.h
 *
 *  Created on: Feb 11, 2009
 *      Author: jchu014
 */

#ifndef DICOMIMAGE_H_
#define DICOMIMAGE_H_

#include <string>
#include <vector>
#include <boost/tr1/memory.hpp>
#include <boost/utility.hpp>
#include <boost/bind.hpp>

#include "math/algebra.h"
#include "CAPContour.h"

//struct Cmiss_texture;

namespace cap
{
/**
 * Dicom image. 
 */
class DICOMImage : boost::noncopyable
{
public:

	/**
	 * Constructor.
	 *
	 * @param	filename	Filename of the file.
	 */
	explicit DICOMImage(const std::string& filename);
	~DICOMImage()
	{
		if (plane_)
		{
			delete plane_;
		}
	}

	/**
	 * Gets the filename.
	 *
	 * @return	The filename.
	 */
	std::string const& GetFilename()
	{
		return filename_;
	}

	/**
	 * Gets the image orientation.
	 *
	 * @param	original	(optional) return the original image orientation.
	 *
	 * @return	The image orientation.
	 */
	std::pair<Vector3D,Vector3D> GetImageOrientation(bool originalOrientation = false) const
	{
		if (isRotated_ && !originalOrientation)
			return std::make_pair(modifiedOrientation1_, modifiedOrientation2_);

		return std::make_pair(orientation1_, orientation2_);
	}

	/**
	 * Gets the image position.
	 *
	 * @param	originalPosition	(optional) return the original position of the image.
	 *
	 * @return	The image position.
	 */
	Point3D const& GetImagePosition(bool originalPosition = false) const
	{
		if (isShifted_ && !originalPosition)
			return shiftedPosition_;

		return position3D_;
	}

	/**
	 * Gets the series description.
	 *
	 * @return	The series description.
	 */
	std::string const& GetSeriesDescription() const
	{
		return seriesDescription_;
	}

	/**
	 * Gets the sequence name.
	 *
	 * @return	The sequence name.
	 */
	std::string const& GetSequenceName() const
	{
		return sequenceName_;
	}

	/**
	 * Gets the trigger time.
	 *
	 * @return	The trigger time.
	 */
	double GetTriggerTime() const
	{
		return triggerTime_;
	}

	/**
	 * Gets the patient name.
	 *
	 * @return	The patient name.
	 */
	std::string const& GetPatientName() const
	{
		return patientName_;
	}

	/**
	 * Gets the patient identifier.
	 *
	 * @return	The patient identifier.
	 */
	std::string const& GetPatientID() const
	{
		return patientId_;
	}

	/**
	 * Gets the scan date.
	 *
	 * @return	The scan date.
	 */
	std::string const& GetScanDate() const
	{
		return scanDate_;
	}

	/**
	 * Gets the date of birth.
	 *
	 * @return	The date of birth.
	 */
	std::string const& GetDateOfBirth() const
	{
		return dateOfBirth_;
	}

	/**
	 * Gets the gender.
	 *
	 * @return	The gender.
	 */
	std::string const& GetGender() const
	{
		return gender_;
	}

	/**
	 * Gets the age.
	 *
	 * @return	The age.
	 */
	std::string const& GetAge() const
	{
		return age_;
	}
	
	/**
	 * Gets the image plane, computes the image plane
	 * if it hasn't been computed.
	 *
	 * @return	null if it fails, else the image plane from dicom header information.
	 */
	ImagePlane* GetImagePlane();
	
	int GetSeriesNumber() const
	{
		return seriesNumber_;
	}
	
	size_t GetImageWidthPx() const
	{
		return width_;
	}
	
	/**
	 * Get the image width in millimeters.
	 */
	double GetImageWidthMm(void);

	/**
	 * Get the image height in millimeters.
	 */
	double GetImageHeightMm(void);

	size_t GetImageHeightPx() const
	{
		return height_;
	}

	/**
	 * Gets the study instance uid.
	 *
	 * @return	The study instance uid.
	 */
	std::string const& GetStudyInstanceUID() const
	{
		return studyInstanceUID_;
	}

	/**
	 * Gets the sop instance uid.
	 *
	 * @return	The sop instance uid.
	 */
	std::string const& GetSopInstanceUID() const
	{
		return sopInstanceUID_;
	}

	/**
	 * Gets the series instance uid.
	 *
	 * @return	The series instance uid.
	 */
	std::string const& GetSeriesInstanceUID() const
	{
		return seriesInstanceUID_;
	}

	/**
	 * Gets the instance number.
	 *
	 * @return	The instance number.
	 */
	int GetInstanceNumber() const
	{
		return instanceNumber_;
	}

	/**
	 * Gets the content time.
	 *
	 * @return	The content time.
	 */
	const std::string& GetContentTime() const
	{
		return contentTime_;
	}

	/**
	 * Query if this image is shifted.
	 *
	 * @return	true if shifted, false if not.
	 */
	bool IsShifted() const
	{
		return isShifted_;
	}

	/**
	 * Query if this image is rotated.
	 *
	 * @return	true if rotated, false if not.
	 */
	bool IsRotated() const
	{
		return isRotated_;
	}
	
	/**
	 * Shifts the image position to the given point.
	 *
	 * @param	p	The point to shift the image to.
	 */
	void SetImagePosition(Point3D const& p)
	{
		isShifted_ = true;
		shiftedPosition_ = p;
		ComputeImagePlane();
	}

	/**
	 * Sets the image plane tlc.
	 *
	 * @param	p	The tlc to set the image plane to.
	 */
	void SetImagePlaneTLC(Point3D const& p)
	{
		//covert tlc to ImagePosition
		Point3D pos = p + 0.5 * pixelSizeX_ * orientation1_ + 0.5f * pixelSizeY_ * orientation2_;
		SetImagePosition(pos);
	}

	/**
	 * Sets a rotated image orientation.
	 *
	 * @param	v1	The row cosine Vector3D const&.
	 * @param	v2	The column cosine Vector3D const&.
	 */
	void SetImageOrientation(Vector3D const& v1, Vector3D const& v2)
	{
		isRotated_ = true;
		// cosine vectors
		modifiedOrientation1_ = v1;
		modifiedOrientation2_ = v2;
	}


	/**
	 * Gets the contours.
	 *
	 * @return	The contours.
	 */
	//-- FIXME What needs fixing here??
	std::vector<ContourPtr>& GetContours()
	{
		return contours_;
	}

	/**
	 * Adds a contour. 
	 *
	 * @param	con	The contour.
	 */
	void AddContour(const ContourPtr& con)
	{
		contours_.push_back(con);
	}
	
//	void SetContourVisibility(bool visibility)
//	{
//		std::for_each(contours_.begin(), contours_.end(), 
//				boost::bind(&CAPContour::SetVisibility, _1, visibility));
//	}

	/**
	 * Gets the pixel size x coordinate.
	 *
	 * @return	The pixel size x coordinate.
	 */
	double GetPixelSizeX() const
	{
		return pixelSizeX_;
	}

	/**
	 * Gets the pixel size y coordinate.
	 *
	 * @return	The pixel size y coordinate.
	 */
	double GetPixelSizeY() const
	{
		return pixelSizeY_;
	}
	
private:
	/**
	 * Compute the image found from information extracted from the 
	 * DICOM header.
	 */
	void ComputeImagePlane();
	
	/**
	 * Read in the DICOM image extracting information from the image
	 * attributes.
	 */
	void ReadDICOMFile();
	
	std::string filename_;  /**< Filename of the file */
	unsigned int width_;	/**< The width */
	unsigned int height_;   /**< The height */
//	double thickness_;
	double pixelSizeX_; /**< The pixel size x coordinate */
	double pixelSizeY_;	/**< The pixel size y coordinate */
	
//	double timeInCardiacCycle;
	
	std::string studyInstanceUID_;
	std::string sopInstanceUID_;
	std::string seriesInstanceUID_;
	std::string seriesDescription_;
	std::string sequenceName_;
	std::string contentTime_;
	double triggerTime_;
	int seriesNumber_;
	Point3D position3D_; /**< image position, location in mm from the origin of the RCS */
	Vector3D orientation1_; /**< values from the row (X) direction cosine */
	Vector3D orientation2_; /**< values from the column (Y) direction cosine */
	
	Point3D shiftedPosition_;
	Vector3D modifiedOrientation1_;
	Vector3D modifiedOrientation2_;

	int instanceNumber_;
	
	std::string patientName_;
	std::string patientId_;
	std::string scanDate_;
	std::string dateOfBirth_;
	std::string gender_;
	std::string age_;
	
	ImagePlane* plane_;
//	Cmiss_texture* texture_;
	std::vector<ContourPtr> contours_; //FIXME fix what???
	
	bool isShifted_;
	bool isRotated_;
};

//class DICOMImage;

typedef boost::shared_ptr<DICOMImage> DICOMPtr;

} // end namespace cap
#endif /* DICOMIMAGE_H_ */
