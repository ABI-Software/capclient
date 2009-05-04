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

using namespace std;

DICOMImage::DICOMImage(const string& filename_)
	: filename(filename_), plane(0)
{
}

ImagePlane* DICOMImage::GetImagePlaneFromDICOMHeaderInfo()
{
	//First, load info from DICOM header

	//Exception safeness! -> better not perform file i/o in the ctor?
	//gdcm::StringFilter sf;
	gdcm::Reader r;
	r.SetFileName( filename.c_str() );
	if( !r.Read() )
	{
		cout << "Can't find the file: " << filename << endl;
		return 0;
	}
	gdcm::DataSet const& ds = r.GetFile().GetDataSet();
	//sf.SetFile( r.GetFile() );

	const gdcm::DataElement& rows = ds.GetDataElement(gdcm::Tag(0x0028,0x0010));
//	const gdcm::ByteValue* value = rows.GetByteValue();
//	height = *(reinterpret_cast<const unsigned short*>((value->GetPointer()))); //Endianness??
	gdcm::Attribute<0x0028,0x0010> at_rows;
	at_rows.SetFromDataElement(rows);
	height = at_rows.GetValue();
	cout << "Rows: " << height;
	cout << endl;

	const gdcm::DataElement& cols = ds.GetDataElement(gdcm::Tag(0x0028,0x0011));
//	value = cols.GetByteValue();
//	width = *(reinterpret_cast<const unsigned short*>((value->GetPointer())));
	gdcm::Attribute<0x0028,0x0011> at_cols;
	at_cols.SetFromDataElement(cols);
	width = at_cols.GetValue();
	cout << "Columns: " << width;
	cout << endl;

	const gdcm::DataElement& thick = ds.GetDataElement(gdcm::Tag(0x0018,0x0050));
//	value = thick.GetByteValue();
//	thickness = *(reinterpret_cast<const float*>(value->GetPointer()));
	gdcm::Attribute<0x0018,0x0050> at_thick;
	at_thick.SetFromDataElement(thick);
	thickness = at_thick.GetValue();

	const gdcm::DataElement& position = ds.GetDataElement(gdcm::Tag(0x0020,0x0032));
	gdcm::Attribute<0x0020,0x0032> at;
	at.SetFromDataElement(position);
	Point3D position3D(at[0],at[1],at[2]);

	const gdcm::DataElement& orientation = ds.GetDataElement(gdcm::Tag(0x0020,0x0037));
	gdcm::Attribute<0x0020,0x0037> at_ori;
	at_ori.SetFromDataElement(orientation);
	Vector3D orientation1(at_ori[0],at_ori[1],at_ori[2]);
	Vector3D orientation2(at_ori[3],at_ori[4],at_ori[5]);

	const gdcm::DataElement& spacing = ds.GetDataElement(gdcm::Tag(0x0028,0x0030));
	gdcm::Attribute<0x0028,0x0030> at_spc;
	at_spc.SetFromDataElement(spacing);
	pixelSizeX = at_spc[0];
	pixelSizeY = at_spc[1];

	//Now construct the plane from the info

	//int imageSize = std::max<u_int>(width,height);
	//cout << "imageSize: " << imageSize << endl;

	plane = new ImagePlane();

	// JGB - 2007/12/05 - plane's tlc starts from the edge of the first voxel
	// rather than centre; (0020, 0032) is the centre of the first voxel
	plane->tlc = position3D - pixelSizeX * (0.5f*orientation1 + 0.5f*orientation2);

	float fieldOfViewX = width * pixelSizeX;//JDCHUNG consider name change
	cout << "width in mm = " << fieldOfViewX ;
	
	plane->trc = plane->tlc + fieldOfViewX * orientation1;

	float fieldOfViewY = height * pixelSizeY;//JDCHUNG
	cout << ", height in mm = " << fieldOfViewY << endl ;
	
	plane->blc = plane->tlc + fieldOfViewY * orientation2;

	plane->xside = plane->trc - plane->tlc;
	plane->yside = plane->tlc - plane->blc;
	plane->normal.CrossProduct(plane->xside, plane->yside);
	plane->normal.Normalise();

	plane->brc = plane->blc + plane->xside;

#ifdef DEBUG
	std::cout << plane->trc << endl;
	std::cout << plane->tlc << endl;
	std::cout << plane->brc << endl;
	std::cout << plane->blc << endl;
#endif
	
	plane->d = DotProduct((plane->tlc - Point3D(0,0,0)) ,plane->normal);
	
	return plane;
}
