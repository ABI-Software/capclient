/*
 * DICOMImage.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */
#include "capclientconfig.h"

#include "dicomimage.h"


#include "utils/debug.h"
#include "logmsg.h"

#ifdef NDEBUG //HACK!
#undef NDEBUG // HACK to get around the fact that gdcm only gets compiled without NDEBUG during the cmgui build process
#include "gdcmReader.h"
#include "gdcmAttribute.h"
#define NDEBUG
#else
#include "gdcmReader.h"
#include "gdcmAttribute.h"
#endif

extern "C" {
#include <zn/cmiss_status.h>
#include <zn/cmiss_core.h>
}

#include "math.h"

#include <string>
#include <ostream>
#include <boost/algorithm/string.hpp>

namespace cap
{

using namespace std;

DICOMImage::DICOMImage(const string& filename)
    : filename_(filename)
    , plane_(0)
    , isShifted_(false)
    , isRotated_(false)
{
}

DICOMImage::~DICOMImage()
{
    if (plane_)
    {
        delete plane_;
    }
}

bool DICOMImage::IsTagRequired(const std::string& tag)
{
    static const std::string required_tags[] = {"dcm:SOPInstanceUID", "dcm:StudyInstanceUID", "dcm:SeriesInstanceUID"
        , "dcm:SeriesNumber", "dcm:Rows", "dcm:Columns", "dcm:ImagePosition(Patient)"
        , "dcm:ImageOrientation(Patient)", "dcm:PixelSpacing"};
    int numRequiredTags = sizeof(required_tags) / sizeof(std::string);
    for(int i = 0; i < numRequiredTags; i++)
    {
        if (required_tags[i] == tag)
            return true;
    }

    return false;
}

std::string DICOMImage::GetDefaultTagValue(const std::string& tag)
{
    std::string value = "N/A";
    if (tag == "dcm:SeriesDescription")
        value = "";
    else if (tag == "dcm:SequenceName")
        value = "";
    else if (tag == "dcm:InstanceNumber")
        value = "-1";

    return value;
}

void DICOMImage::AssignTagValue(const std::string& tag, const std::string& value)
{
    if (tag == "dcm:SOPInstanceUID")
        sopInstanceUID_ = value;
    else if (tag == "dcm:StudyInstanceUID")
        studyInstanceUID_ = value;
    else if (tag == "dcm:SeriesInstanceUID")
        seriesInstanceUID_ = value;
    else if (tag == "dcm:SeriesDescription")
        seriesDescription_ = value;
    else if (tag == "dcm:SeriesNumber")
        seriesNumber_ = FromString<int>(value);
    else if (tag == "dcm:SequenceName")
        sequenceName_ = value;
    else if (tag == "dcm:Rows")
        height_ = FromString<int>(value);
    else if (tag == "dcm:Columns")
        width_ = FromString<int>(value);
    else if (tag == "dcm:ImagePosition(Patient)")
    {
        std::vector<double> values = FromString<double>(value, '\\');
        position3D_ = Point3D(values[0], values[1], values[2]);
    }
    else if (tag == "dcm:ImageOrientation(Patient)")
    {
        std::vector<double> values = FromString<double>(value, '\\');
        orientation1_ = Vector3D(values[0], values[1], values[2]);
        orientation2_ = Vector3D(values[3], values[4], values[5]);
    }
    else if (tag == "dcm:PixelSpacing")
    {
        std::vector<double> values = FromString<double>(value, '\\');
        pixelSizeX_ = values[0];
        pixelSizeY_ = values[1];
    }
    else if (tag == "dcm:InstanceNumber")
        instanceNumber_ = FromString<int>(value);
    else if (tag == "dcm:Patient'sName")
        patientName_ = value;
    else if (tag == "dcm:PatientID")
        patientId_ = value;
    else if (tag == "dcm:AcquisitionDate")
        scanDate_ = value;
    else if (tag == "dcm:Patient'sBirthDate")
        dateOfBirth_ = value;
    else if (tag == "dcm:Patient'sSex")
        gender_ = value;
    else if (tag == "dcm:Patient'sAge")
        age_ = value;
    else
    {
        LOG_MSG(LOGERROR) << "Unknown dcm tag: '" << tag << "' - not assigned";
    }
}

bool DICOMImage::Analyze(Cmiss_field_image_id field_image)
{
    bool success = true;
    // SOP instance UID (0x0008,0x0018)
    // Study Instance UID (0x0020,0x000d)
    // Series Instance UID (0x0020,0x000e)
    // Series Description (0x0008,0x103e)
    // Series Number (0x0020,0x0011)
    // Sequence Name (0x0018,0x0024)
    // Rows (0x0028,0x0010)
    // Columns (0x0028,0x0011)
    // Image Position (Patient) (0x0020,0x0032)
    // Image Orientation (Patient) (0x0020,0x0037)
    // Image Orientation (0x0020,0x0035) <- old, used if newer value is not present
    // Pixel Spacing (0x0028,0x0030)
    // Instance Number (0x0020,0x0013)
    // Patient's Name (0x0010,0x0010)
    // Patient ID (0x0010,0x0020)
    // Acquisition Date (0x0008,0x0022)
    // Patient's Birth Date (0x0010,0x0030)
    // Patient's Sex (0x0010,0x0040)
    // Patient's Age (0x0010,0x1010)
    static const std::string tags[] = {"dcm:SOPInstanceUID", "dcm:StudyInstanceUID", "dcm:SeriesInstanceUID", "dcm:SeriesDescription"
        , "dcm:SeriesNumber", "dcm:SequenceName", "dcm:Rows", "dcm:Columns", "dcm:ImagePosition(Patient)"
        , "dcm:ImageOrientation(Patient)", "dcm:PixelSpacing", "dcm:InstanceNumber", "dcm:Patient'sName"
        , "dcm:PatientID", "dcm:AcquisitionDate", "dcm:Patient'sBirthDate", "dcm:Patient'sSex", "dcm:Patient'sAge"};
    int numTags = sizeof(tags) / sizeof(std::string);

    for (int i = 0; i < numTags && success; i++)
    {
        char *prop = Cmiss_field_image_get_property(field_image, tags[i].c_str());
        if (prop == 0)
        {
            if (tags[i] == "dcm:ImageOrientation(Patient)")
            {
                prop = Cmiss_field_image_get_property(field_image, "dcm:ImageOrientation");
                if (prop == 0)
                {
                    success = false;
                    LOG_MSG(LOGERROR) << "DICOM header analysis failed : " << filename_;
                    LOG_MSG(LOGERROR) << "DICOM header missing required tag : dcm:ImageOrientation";
                }
                else
                {
                    std::string value = prop;
                    Cmiss_deallocate(prop);
                    AssignTagValue(tags[i], value);
                }
            }
            else
            {
                if (IsTagRequired(tags[i]))
                {
                    success = false;
                    LOG_MSG(LOGERROR) << "DICOM header analysis failed : " << filename_;
                    LOG_MSG(LOGERROR) << "DICOM header missing required tag : " << tags[i];
                }
                else
                {
                    std::string value = GetDefaultTagValue(tags[i]);
                    AssignTagValue(tags[i], value);
                    LOG_MSG(LOGINFORMATION) << "DICOM header '" << filename_ << "' missing tag : " << tags[i];
                }
            }
        }
        else
        {
            std::string value = prop;
            Cmiss_deallocate(prop);
            AssignTagValue(tags[i], value);
        }
    }

    return success;
}

void DICOMImage::ComputeImagePlane()
{
    if (!plane_)
    {
        plane_ = new ImagePlane();
    }
    // plane_'s tlc starts from the edge of the first voxel
    // rather than centre; (0020, 0032) is the centre of the first voxel

    Point3D pos;
    if (IsShifted())
    {
        pos = shiftedPosition_;
    }
    else
    {
        pos = position3D_;
    }

    plane_->tlc = pos - 0.5 * pixelSizeX_ * orientation1_ -  0.5f * pixelSizeY_ * orientation2_;

    double fieldOfViewX = width_ * pixelSizeX_;
//	cout << "width in mm = " << fieldOfViewX ;

    plane_->trc = plane_->tlc + fieldOfViewX * orientation1_;

    double fieldOfViewY = height_ * pixelSizeY_;//JDCHUNG
//	cout << ", height in mm = " << fieldOfViewY << endl ;

    plane_->blc = plane_->tlc + fieldOfViewY * orientation2_;

    plane_->xside = plane_->trc - plane_->tlc;
    plane_->yside = plane_->blc - plane_->tlc;
    plane_->normal.CrossProduct(plane_->xside, plane_->yside);
    plane_->normal.Normalise();

    plane_->brc = plane_->blc + plane_->xside;

//    dbg(ToString(plane_->trc));
//    dbg(ToString(plane_->tlc));
//    dbg(ToString(plane_->brc));
//    dbg(ToString(plane_->blc));

    plane_->d = DotProduct((plane_->tlc - Point3D(0,0,0)) ,plane_->normal);
}

ImagePlane* DICOMImage::GetImagePlane()
{
    if (!plane_)
        ComputeImagePlane();

    return plane_;
}

} // end namespace cap

