/*
 * DICOMImage.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */
#include "Config.h"

#include "DICOMImage.h"

#include "gdcmStringFilter.h"
#include "gdcmReader.h"
#include "gdcmSequenceOfItems.h"
#include "gdcmTesting.h"
#include "gdcmTag.h"
#include "gdcmAttribute.h"
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
	gdcm::StringFilter sf;
	gdcm::Reader r;
	r.SetFileName( filename.c_str() );
	if( !r.Read() )
	{
		cout << "Can't find the file: " << filename << endl;
		return 0;
	}
	gdcm::DataSet const& ds = r.GetFile().GetDataSet();
	sf.SetFile( r.GetFile() );

	const gdcm::DataElement& rows = ds.GetDataElement(gdcm::Tag(0x0028,0x0010));
	const gdcm::ByteValue* value = rows.GetByteValue();
//		std::pair<std::string, std::string> s = sf.ToStringPair( rows.GetTag() );
//		std::cout << s.first << "==> " << s.second << std::endl;
	height = *(reinterpret_cast<const unsigned short*>((value->GetPointer()))); //Endianness??
	cout << "Rows: " << height;
	cout << endl;

	const gdcm::DataElement& cols = ds.GetDataElement(gdcm::Tag(0x0028,0x0011));
	value = cols.GetByteValue();
	width = *(reinterpret_cast<const unsigned short*>((value->GetPointer())));
	cout << "Columns: " << width;
	cout << endl;

	const gdcm::DataElement& thick = ds.GetDataElement(gdcm::Tag(0x0018,0x0050));
	value = thick.GetByteValue();
	thickness = *(reinterpret_cast<const float*>(value->GetPointer()));

//		cout << "Thic: " << ds.GetDataElement(gdcm::Tag(0x0018,0x0050)) << endl;
	cout << "Posi: " << ds.GetDataElement(gdcm::Tag(0x0020,0x0032)) << endl;
	cout << "Orie: " << ds.GetDataElement(gdcm::Tag(0x0020,0x0037)) << endl;
//		cout << "Spac: " << ds.GetDataElement(gdcm::Tag(0x0028,0x0030)) << endl;

	const gdcm::DataElement& pos = ds.GetDataElement(gdcm::Tag(0x0020,0x0032));
//		value = pos.GetByteValue();
//		value->PrintHex(cout, value->GetLength());
//		cout << endl;
	//const float* ptr = reinterpret_cast<const float*>(value->GetPointer());
	gdcm::Attribute<0x0020,0x0032> at;
	at.SetFromDataElement(pos);
	const float* ptr = at.GetValues();
	floats[3] = *ptr++;
	floats[4] = *ptr++;
	floats[5] = *ptr;

	const gdcm::DataElement& ori = ds.GetDataElement(gdcm::Tag(0x0020,0x0037));
	gdcm::Attribute<0x0020,0x0037> at_ori;
	at_ori.SetFromDataElement(ori);
	ptr = at_ori.GetValues();
	floats[6] = *ptr++;
	floats[7] = *ptr++;
	floats[8] = *ptr++;
	floats[9] = *ptr++;
	floats[10] = *ptr++;
	floats[11] = *ptr;

	const gdcm::DataElement& spacing = ds.GetDataElement(gdcm::Tag(0x0028,0x0030));
	//value = spacing.GetByteValue();
	//ptr = reinterpret_cast<const float*>(value->GetPointer());
	gdcm::Attribute<0x0028,0x0030> at_spc;
	at_spc.SetFromDataElement(spacing);
	ptr = at_spc.GetValues();
	pixelSizeX = *ptr++;
	pixelSizeY = *ptr;

	//Now construct the plane from the info

	//int imageSize = std::max<u_int>(width,height);
	//cout << "imageSize: " << imageSize << endl;

	plane = new ImagePlane();

	// JGB - 2007/12/05 - plane's tlc starts from the edge of the first voxel
	// rather than centre; (0020, 0032) is the centre of the first voxel
	float tlcex, tlcey, tlcez; // top left corner edge
	tlcex = floats[3] - pixelSizeX * (0.5f*floats[6] + 0.5f*floats[9]);
	tlcey = floats[4] - pixelSizeX * (0.5f*floats[7] + 0.5f*floats[10]);
	tlcez = floats[5] - pixelSizeX * (0.5f*floats[8] + 0.5f*floats[11]);

	plane->tlc.x = tlcex;
	plane->tlc.y = tlcey;
	plane->tlc.z = tlcez;

	float fieldOfViewX = width * pixelSizeX;//JDCHUNG consider name change
	cout << "width in mm = " << fieldOfViewX ;

	plane->trc.x = plane->tlc.x + fieldOfViewX*floats[6];
	plane->trc.y = plane->tlc.y + fieldOfViewX*floats[7];
	plane->trc.z = plane->tlc.z + fieldOfViewX*floats[8];

	float fieldOfViewY = height * pixelSizeY;//JDCHUNG
	cout << ", height in mm = " << fieldOfViewY << endl ;

	plane->blc.x = plane->tlc.x + fieldOfViewY*floats[9];
	plane->blc.y = plane->tlc.y + fieldOfViewY*floats[10];
	plane->blc.z = plane->tlc.z + fieldOfViewY*floats[11];

	//FIND_VECTOR(plane->xside, plane->tlc, plane->trc);
	plane->xside = plane->trc - plane->tlc;
	//FIND_VECTOR(plane->yside, plane->blc, plane->tlc);
	plane->yside = plane->tlc - plane->blc;
	CROSS_PRODUCT(plane->normal, plane->xside, plane->yside);
	NORMALISE(plane->normal);

	plane->brc.x = plane->blc.x + plane->xside.x;
	plane->brc.y = plane->blc.y + plane->xside.y;
	plane->brc.z = plane->blc.z + plane->xside.z;

#ifdef DEBUG
	std::cout << plane->trc << endl;
	std::cout << plane->tlc << endl;
	std::cout << plane->brc << endl;
	std::cout << plane->blc << endl;
#endif
	
	plane->d = DOT((plane->tlc - Point3D(0,0,0)) ,plane->normal);
	
	return plane;
	dir_path.append(path_separator);

}
