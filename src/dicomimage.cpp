/*
 * DICOMImage.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */
#include "capclientconfig.h"

#include "dicomimage.h"

#ifdef NDEBUG //HACK!
#undef NDEBUG // HACK to get around the fact that gdcm only gets compiled without NDEBUG during the cmgui build process
#include "gdcmReader.h"
#include "gdcmAttribute.h"
#define NDEBUG
#else
#include "gdcmReader.h"
#include "gdcmAttribute.h"
#endif

#include "math.h"

#include <string>
#include <ostream>
#include <boost/algorithm/string.hpp>

#include "utils/debug.h"
#include "logmsg.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

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
	dbg("DICOMImage::~DICOMImage()");
	if (plane_)
	{
		delete plane_;
	}
}

void DICOMImage::ReadFile()
{
	// study instance uid (0020,000d)
	// sop instance uid (0008,0018) 
	// rows (0028,0010)
	// columns (0028,0011)
	// slice thickness_ (0018,0050)
	// image position (0020,0032) - (0020,0030)(old)
	// image orientation (0020,0037) - (0020,0035)(old)
	// pixel spacing (0028,0030)
	// series description (0008,103e) 
	// (0018,1090) IS [28]         # 2,1 Cardiac Number of Images

	gdcm::Reader r;
	r.SetFileName( filename_.c_str() );
	//cout << "DICOM filename = " << filename_ << '\n';
	if( !r.Read() )
	{
		LOG_MSG(LOGERROR) << "DICOM file read failed : " << filename_;
		throw std::exception();
	}
	
	gdcm::DataSet const& ds = r.GetFile().GetDataSet();
	
	{
		// SOP instance UID (0008,0018) 
		const gdcm::DataElement& sopiuid = ds.GetDataElement(gdcm::Tag(0x0008,0x0018));
		gdcm::Attribute<0x0008,0x0018> at_sopiuid;
		at_sopiuid.SetFromDataElement(sopiuid);
		sopInstanceUID_ = at_sopiuid.GetValue();
		// gdcm leaves some non alpha numeric characters at the back
		// get rid of them here
		boost::trim_right_if(sopInstanceUID_, !boost::is_digit());
	//	cout << "UID: " << sopInstanceUID_;
	//	cout << endl;
	}
	
	{
		// Study Instance UID (0020,000d)
		const gdcm::DataElement& studyiuid = ds.GetDataElement(gdcm::Tag(0x0020,0x000d));
		gdcm::Attribute<0x0020,0x000d> at_studyiuid;
		at_studyiuid.SetFromDataElement(studyiuid);
		studyInstanceUID_ = at_studyiuid.GetValue();
		boost::trim_right_if(studyInstanceUID_, !boost::is_digit());
	}
	
	{
		// Series Instance UID (0020,000E)
		const gdcm::DataElement& seriesiuid = ds.GetDataElement(gdcm::Tag(0x0020,0x000e));
		gdcm::Attribute<0x0020,0x000e> at_seriesiuid;
		at_seriesiuid.SetFromDataElement(seriesiuid);
		seriesInstanceUID_ = at_seriesiuid.GetValue();
		boost::trim_right_if(seriesInstanceUID_, !boost::is_digit());
	}
	
	// series number (0020,0011)
	if (ds.FindDataElement(gdcm::Tag(0x0020,0x0011)))
	{
		const gdcm::DataElement& seriesNum = ds.GetDataElement(gdcm::Tag(0x0020,0x0011));
		gdcm::Attribute<0x0020,0x0011> at_sn;
		at_sn.SetFromDataElement(seriesNum);
		seriesNumber_ = at_sn.GetValue();
	//	cout << "Series Number: " << seriesNumber_;
	//	cout << endl;
	}
	else
	{
		LOG_MSG(LOGERROR) << "Series number not found: " << filename_;
		throw std::exception();
	}
	
	// series description (0008,103e)
	if (ds.FindDataElement(gdcm::Tag(0x0008,0x103E)))
	{
		const gdcm::DataElement& seriesDesc = ds.GetDataElement(gdcm::Tag(0x0008,0x103E));
		gdcm::Attribute<0x0008,0x103E> at_sd;
		at_sd.SetFromDataElement(seriesDesc);
		seriesDescription_ = at_sd.GetValue();
	//	cout << "Series Description: " << seriesDescription_;
	//	cout << endl;
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Series description not found: " << filename_;
		seriesDescription_ = "";
	}
	
	// sequence name (0018,0024)
	if (ds.FindDataElement(gdcm::Tag(0x0018,0x0024)))
	{
		const gdcm::DataElement& seqName = ds.GetDataElement(gdcm::Tag(0x0018,0x0024));
		gdcm::Attribute<0x0018,0x0024> at_seqName;
		at_seqName.SetFromDataElement(seqName);
		sequenceName_ = at_seqName.GetValue();
//		cout << "sequenceName_: " << sequenceName_ << '\n';
	//	cout << endl;
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Sequence Name not found: " << filename_;
		sequenceName_ = "";
	}
	
	if (ds.FindDataElement(gdcm::Tag(0x0028,0x0010)))
	{
		const gdcm::DataElement& rows = ds.GetDataElement(gdcm::Tag(0x0028,0x0010));
		gdcm::Attribute<0x0028,0x0010> at_rows;
		at_rows.SetFromDataElement(rows);
		height_ = at_rows.GetValue();
//		cout << "Rows: " << height_;
//		cout << endl;
	}
	else
	{
		LOG_MSG(LOGERROR) << "Rows not found: " << filename_;
		throw std::exception();
	}

	if (ds.FindDataElement(gdcm::Tag(0x0028,0x0011)))
	{
		const gdcm::DataElement& cols = ds.GetDataElement(gdcm::Tag(0x0028,0x0011));
		gdcm::Attribute<0x0028,0x0011> at_cols;
		at_cols.SetFromDataElement(cols);
		width_ = at_cols.GetValue();
//		cout << "Columns: " << width_;
//		cout << endl;
	}

//	if (ds.FindDataElement(gdcm::Tag(0x0018,0x0050)))
//	{
//		const gdcm::DataElement& thick = ds.GetDataElement(gdcm::Tag(0x0018,0x0050));
//		gdcm::Attribute<0x0018,0x0050> at_thick;
//		at_thick.SetFromDataElement(thick);
//		thickness_ = at_thick.GetValue();
//	}
//	else
//	{
//		cout << "Slice Thickness not found.\n";
//		thickness_ = 0.0;
//	}

	if (ds.FindDataElement(gdcm::Tag(0x0020,0x0032)))
	{
		const gdcm::DataElement& position = ds.GetDataElement(gdcm::Tag(0x0020,0x0032));
		gdcm::Attribute<0x0020,0x0032> at;
		at.SetFromDataElement(position);
		position3D_ = Point3D(at[0],at[1],at[2]);
	}
	else
	{
		LOG_MSG(LOGERROR) << "Image Position not found: " << filename_;
		throw std::exception();
	}

	if (ds.FindDataElement(gdcm::Tag(0x0020,0x0037)))
	{
		const gdcm::DataElement& orientation = ds.GetDataElement(gdcm::Tag(0x0020,0x0037));
		gdcm::Attribute<0x0020,0x0037> at_ori;
		at_ori.SetFromDataElement(orientation);
		orientation1_ = Vector3D(at_ori[0],at_ori[1],at_ori[2]);
		orientation2_ = Vector3D(at_ori[3],at_ori[4],at_ori[5]);
	}
	else if (ds.FindDataElement(gdcm::Tag(0x0020,0x0035)))
	{
		LOG_MSG(LOGINFORMATION) << "(0x20,0x37) Image Orientation - Patient is missing. We will compute the orientation from (0x20,0x35) instead.";

		const gdcm::DataElement& orientation = ds.GetDataElement(gdcm::Tag(0x0020,0x0035));
		gdcm::Attribute<0x0020,0x0035> at_ori; //test
		at_ori.SetFromDataElement(orientation);
		orientation1_ = Vector3D(-at_ori[0],at_ori[1],-at_ori[2]);
		orientation2_ = Vector3D(-at_ori[3],at_ori[4],-at_ori[5]);
	}
	else
	{
		LOG_MSG(LOGERROR) << "Image Orientation not found: " << filename_;
		throw std::exception();
	}
	
	if (ds.FindDataElement(gdcm::Tag(0x0028,0x0030)))
	{
		const gdcm::DataElement& spacing = ds.GetDataElement(gdcm::Tag(0x0028,0x0030));
		gdcm::Attribute<0x0028,0x0030> at_spc;
		at_spc.SetFromDataElement(spacing);
		pixelSizeX_ = at_spc[0];
		pixelSizeY_ = at_spc[1];
//		cout << "pixelSize\n";
	}
	else
	{
		LOG_MSG(LOGERROR) << "Pixel Spacing not found: " << filename_;
		throw std::exception();
	}
	
	//patient name (0010,0010) 
	if (ds.FindDataElement(gdcm::Tag(0x0010,0x0010)))
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0010));
		gdcm::Attribute<0x0010,0x0010> at;
		at.SetFromDataElement(de);
		patientName_ = at.GetValue();
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Patient Name not found: " << filename_;
		patientName_ = "N/A";
	}
	
	//patient id (0010,0020)
	if (ds.FindDataElement(gdcm::Tag(0x0010,0x0020)))
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0020));
		gdcm::Attribute<0x0010,0x0020> at;
		at.SetFromDataElement(de);
		patientId_ = at.GetValue();
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Patient ID not found: " << filename_;
		patientId_ = "N/A";
	}
	
	//acquisition date (0008,0022) 
	if (ds.FindDataElement(gdcm::Tag(0x0008,0x0022)))
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0008,0x0022));
		gdcm::Attribute<0x0008,0x0022> at;
		at.SetFromDataElement(de);
		scanDate_ = at.GetValue();
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Scan Date not found: " << filename_;
		scanDate_ = "N/A";
	}
	
//	cout << "acquisition time\n";
	
	//date of birth (0010,0030)
	if (ds.FindDataElement(gdcm::Tag(0x0010,0x0030)))
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0030));
		gdcm::Attribute<0x0010,0x0030> at;
		at.SetFromDataElement(de);
		dateOfBirth_ = at.GetValue();
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Date of Birth not found: " << filename_;
		dateOfBirth_ = "N/A";
	}
	
	//gender (0010,0040)
	if (ds.FindDataElement(gdcm::Tag(0x0010,0x0040)))
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0040));
		gdcm::Attribute<0x0010,0x0040> at;
		at.SetFromDataElement(de);
		gender_ = at.GetValue();
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Gender not found: " << filename_;
		gender_ = "N/A";
	}
	
	//age (0010,1010)
	if (ds.FindDataElement(gdcm::Tag(0x0010,0x1010)))
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x1010));
		gdcm::Attribute<0x0010,0x1010> at;
		at.SetFromDataElement(de);
		age_ = at.GetValue();
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Patient's age (0010,1010) not found: " << filename_;
		age_ = "N/A";
	}
	
	//instanceNumber_ (0020,0013)
	if (ds.FindDataElement(gdcm::Tag(0x0020,0x0013)))
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0020,0x0013));
		gdcm::Attribute<0x0020,0x0013> at;
		at.SetFromDataElement(de);
		instanceNumber_ = at.GetValue();
	}
	else
	{
		LOG_MSG(LOGWARNING) << "Instance Number (0020,0013) not found: " << filename_;
		instanceNumber_ = -1;
	}

	//dbg("dicom info: " + filename_ + ", " + sopInstanceUID_ + ", " + seriesInstanceUID_);
	
//	cout << "Exiting " << __func__ << '\n';
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

#ifdef DEBUG
	std::cout << plane_->trc << endl;
	std::cout << plane_->tlc << endl;
	std::cout << plane_->brc << endl;
	std::cout << plane_->blc << endl;
#endif
	
	plane_->d = DotProduct((plane_->tlc - Point3D(0,0,0)) ,plane_->normal);
}

ImagePlane* DICOMImage::GetImagePlane()
{
	if (!plane_)
		ComputeImagePlane();

	return plane_;
}

} // end namespace cap

