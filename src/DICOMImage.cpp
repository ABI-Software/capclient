/*
 * DICOMImage.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */
#include "Config.h"

#include "DICOMImage.h"

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

namespace cap
{

using namespace std;

DICOMImage::DICOMImage(const string& filename)
	: filename_(filename),
	  plane_(0),
	  isShifted_(false)
{
	ReadDICOMFile();
}

void DICOMImage::ReadDICOMFile()
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
	// trigger time (0018,1060) 
	// (0018,1090) IS [28]         # 2,1 Cardiac Number of Images

	gdcm::Reader r;
	r.SetFileName( filename_.c_str() );
	if( !r.Read() )
	{
		cout << "Can't read file: " << filename_ << endl;
		throw std::exception();
	}
	
	gdcm::DataSet const& ds = r.GetFile().GetDataSet();
	
	// Study Instance UID (0020,000d)
	const gdcm::DataElement& studyiuid = ds.GetDataElement(gdcm::Tag(0x0020,0x000d));
	gdcm::Attribute<0x0020,0x000d> at_studyiuid;
	at_studyiuid.SetFromDataElement(studyiuid);
	studyInstanceUID_ = at_studyiuid.GetValue();
	
	// SOP instance UID (0008,0018) 
	const gdcm::DataElement& sopiuid = ds.GetDataElement(gdcm::Tag(0x0008,0x0018));
	gdcm::Attribute<0x0008,0x0018> at_sopiuid;
	at_sopiuid.SetFromDataElement(sopiuid);
	sopInstanceUID_ = at_sopiuid.GetValue();
	cout << "UID: " << sopInstanceUID_;
	cout << endl;
	
	// series number (0020,0011)
	const gdcm::DataElement& seriesNum = ds.GetDataElement(gdcm::Tag(0x0020,0x0011));
	gdcm::Attribute<0x0020,0x0011> at_sn;
	at_sn.SetFromDataElement(seriesNum);
	seriesNumber_ = at_sn.GetValue();
	cout << "Series Number: " << seriesNumber_;
	cout << endl;
	
	// series description (0008,103e)
	const gdcm::DataElement& seriesDesc = ds.GetDataElement(gdcm::Tag(0x0008,0x103E));
	gdcm::Attribute<0x0008,0x103E> at_sd;
	at_sd.SetFromDataElement(seriesDesc);
	seriesDescription_ = at_sd.GetValue();
	cout << "Series Description: " << seriesDescription_;
	cout << endl;
	
	// sequence name (0018,0024)
	const gdcm::DataElement& seqName = ds.GetDataElement(gdcm::Tag(0x0018,0x0024));
	gdcm::Attribute<0x0018,0x0024> at_seqName;
	at_seqName.SetFromDataElement(seqName);
	sequenceName_ = at_seqName.GetValue();
	cout << "sequenceName_: " << sequenceName_;
	cout << endl;
	
	// trigger time trigger time (0018,1060)
	if (ds.FindDataElement(gdcm::Tag(0x0018,0x1060)))
	{
		const gdcm::DataElement& triggerTime = ds.GetDataElement(gdcm::Tag(0x0018,0x1060));
		gdcm::Attribute<0x0018,0x1060> at_tt;
		at_tt.SetFromDataElement(triggerTime);
		triggerTime_ = at_tt.GetValue();
		cout << "Trigger Time : " << triggerTime_;
		cout << endl;
	}
	else
	{
		cout << "Trigger Time not found in the DICOM header \n";
		triggerTime_ = -1;
	}

	const gdcm::DataElement& rows = ds.GetDataElement(gdcm::Tag(0x0028,0x0010));
	gdcm::Attribute<0x0028,0x0010> at_rows;
	at_rows.SetFromDataElement(rows);
	height_ = at_rows.GetValue();
	cout << "Rows: " << height_;
	cout << endl;

	const gdcm::DataElement& cols = ds.GetDataElement(gdcm::Tag(0x0028,0x0011));
	gdcm::Attribute<0x0028,0x0011> at_cols;
	at_cols.SetFromDataElement(cols);
	width_ = at_cols.GetValue();
	cout << "Columns: " << width_;
	cout << endl;

	const gdcm::DataElement& thick = ds.GetDataElement(gdcm::Tag(0x0018,0x0050));
	gdcm::Attribute<0x0018,0x0050> at_thick;
	at_thick.SetFromDataElement(thick);
	thickness_ = at_thick.GetValue();

	const gdcm::DataElement& position = ds.GetDataElement(gdcm::Tag(0x0020,0x0032));
	gdcm::Attribute<0x0020,0x0032> at;
	at.SetFromDataElement(position);
	position3D_ = Point3D(at[0],at[1],at[2]);

	if (ds.FindDataElement(gdcm::Tag(0x0020,0x0037)))
	{
		const gdcm::DataElement& orientation = ds.GetDataElement(gdcm::Tag(0x0020,0x0037));
		gdcm::Attribute<0x0020,0x0037> at_ori;
		at_ori.SetFromDataElement(orientation);
		orientation1_ = Vector3D(at_ori[0],at_ori[1],at_ori[2]);
		orientation2_ = Vector3D(at_ori[3],at_ori[4],at_ori[5]);
	}
	else
	{
		std::cout << "(0x20,0x37) Image Orientation - Patient is missing. We will compute the orientation from (0x20,0x35) instead.\n";
		const gdcm::DataElement& orientation = ds.GetDataElement(gdcm::Tag(0x0020,0x0035));
		gdcm::Attribute<0x0020,0x0035> at_ori; //test
		at_ori.SetFromDataElement(orientation);
		orientation1_ = Vector3D(-at_ori[0],at_ori[1],-at_ori[2]);
		orientation2_ = Vector3D(-at_ori[3],at_ori[4],-at_ori[5]);
	}
	
	const gdcm::DataElement& spacing = ds.GetDataElement(gdcm::Tag(0x0028,0x0030));
	gdcm::Attribute<0x0028,0x0030> at_spc;
	at_spc.SetFromDataElement(spacing);
	pixelSizeX_ = at_spc[0];
	pixelSizeY_ = at_spc[1];
	
	//patient name (0010,0010) 
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0010));
		gdcm::Attribute<0x0010,0x0010> at;
		at.SetFromDataElement(de);
		patientName_ = at.GetValue();
	}
	
	//patient id (0010,0020)
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0020));
		gdcm::Attribute<0x0010,0x0020> at;
		at.SetFromDataElement(de);
		patientId_ = at.GetValue();
	}
	
	//acquisition date (0008,0022) 
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0008,0x0022));
		gdcm::Attribute<0x0008,0x0022> at;
		at.SetFromDataElement(de);
		scanDate_ = at.GetValue();
	}
	
	//date of birth (0010,0030)
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0030));
		gdcm::Attribute<0x0010,0x0030> at;
		at.SetFromDataElement(de);
		dateOfBirth_ = at.GetValue();
	}
	
	//gender (0010,0040)
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x0040));
		gdcm::Attribute<0x0010,0x0040> at;
		at.SetFromDataElement(de);
		gender_ = at.GetValue();
	}
	
	//age (0010,1010)
	{
		const gdcm::DataElement& de = ds.GetDataElement(gdcm::Tag(0x0010,0x1010));
		gdcm::Attribute<0x0010,0x1010> at;
		at.SetFromDataElement(de);
		age_ = at.GetValue();
	}
}

ImagePlane* DICOMImage::GetImagePlaneFromDICOMHeaderInfo() const
{	
	//Now construct the plane_ from the info

	//int imageSize = std::max<u_int>(width_,height_);
	//cout << "imageSize: " << imageSize << endl;

	if (plane_)
	{
		return plane_;
	}
	
	plane_ = new ImagePlane();

	// plane_'s tlc starts from the edge of the first voxel
	// rather than centre; (0020, 0032) is the centre of the first voxel
	plane_->tlc = position3D_ - 0.5 * pixelSizeX_ * orientation1_ -  0.5f * pixelSizeY_ * orientation2_;

	double fieldOfViewX = width_ * pixelSizeX_;//JDCHUNG consider name change
	cout << "width in mm = " << fieldOfViewX ;
	
	plane_->trc = plane_->tlc + fieldOfViewX * orientation1_;

	double fieldOfViewY = height_ * pixelSizeY_;//JDCHUNG
	cout << ", height in mm = " << fieldOfViewY << endl ;
	
	plane_->blc = plane_->tlc + fieldOfViewY * orientation2_;

	plane_->xside = plane_->trc - plane_->tlc;
	plane_->yside = plane_->tlc - plane_->blc;
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
	
	return plane_;
}

} // end namespace cap
