/*
 * DICOMImage.h
 *
 *  Created on: Feb 11, 2009
 *      Author: jchu014
 */

#ifndef DICOMIMAGE_H_
#define DICOMIMAGE_H_

#include "gdcmStringFilter.h"
#include "gdcmReader.h"
#include "gdcmSequenceOfItems.h"
#include "gdcmTesting.h"
#include "gdcmTag.h"
#include "gdcmAttribute.h"

#include <string>
#include <sstream>

using namespace std;

class DICOMImage
{
public:
	DICOMImage(const string& filename)
	: name(filename)
	{
		//Exception safeness! -> better not perform file i/o in the ctor?
		gdcm::StringFilter sf;
		gdcm::Reader r;
		r.SetFileName( filename.c_str() );
		if( !r.Read() )
		{
			cout << "Can't find the file: " << filename << endl;
			throw 1;//what should we throw?
		}
		gdcm::DataSet const& ds = r.GetFile().GetDataSet();
		sf.SetFile( r.GetFile() );

		const gdcm::DataElement& rows = ds.GetDataElement(gdcm::Tag(0x0028,0x0010));
		const gdcm::ByteValue* value = rows.GetByteValue();
		std::pair<std::string, std::string> s = sf.ToStringPair( rows.GetTag() );
		std::cout << s.first << "==> " << s.second << std::endl;
//		value->PrintHex(cout, value->GetLength());
		height = *(reinterpret_cast<const u_short*>((value->GetPointer()))); //Endianness??
		cout << "Rows: " << height;
		cout << endl;

		const gdcm::DataElement& cols = ds.GetDataElement(gdcm::Tag(0x0028,0x0011));
		value = cols.GetByteValue();
//		s = sf.ToStringPair( cols.GetTag() );
//		std::cout << s.first << "==> " << s.second << std::endl;
//		value->PrintHex(cout, value->GetLength());
		width = *(reinterpret_cast<const u_short*>((value->GetPointer())));
		cout << "Columns: " << width;
		cout << endl;

		const gdcm::DataElement& thick = ds.GetDataElement(gdcm::Tag(0x0018,0x0050));
		value = thick.GetByteValue();
		s = sf.ToStringPair( thick.GetTag() );
		std::cout << s.first << "==> " << s.second << std::endl;
		value->PrintHex(cout, value->GetLength());
		thickness = *(reinterpret_cast<const float*>(value->GetPointer()));
		cout << ": " << thickness;
		cout << endl;

		cout << "Thic: " << ds.GetDataElement(gdcm::Tag(0x0018,0x0050)) << endl;
		cout << "Posi: " << ds.GetDataElement(gdcm::Tag(0x0020,0x0032)) << endl;
		cout << "Orie: " << ds.GetDataElement(gdcm::Tag(0x0020,0x0037)) << endl;
		cout << "Spac: " << ds.GetDataElement(gdcm::Tag(0x0028,0x0030)) << endl;

		const gdcm::DataElement& pos = ds.GetDataElement(gdcm::Tag(0x0020,0x0032));
		value = pos.GetByteValue();
		value->PrintHex(cout, value->GetLength());
		cout << endl;
		//const float* ptr = reinterpret_cast<const float*>(value->GetPointer());
		gdcm::Attribute<0x0020,0x0032> at;
		at.SetFromDataElement(pos);
		const float* ptr = at.GetValues();
		floats[3] = *ptr++;
		floats[4] = *ptr++;
		floats[5] = *ptr;

//		if (3 != sscanf((char *)ptr, "%f\\%f\\%f", floats+3, floats+4, floats+5)) //easy but c way
//		{
//			//error
//		}
//		std::istringstream in(string((char*)ptr)); //c++ but no easy way to use '\' as delimiter
//		in >> floats[3] >> floats[4] >> floats[5];



		const gdcm::DataElement& ori = ds.GetDataElement(gdcm::Tag(0x0020,0x0037));
		//value = ori.GetByteValue();
		//ptr = reinterpret_cast<const float*>(value->GetPointer());
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
		//see CIM planes_handler.cpp for constructing plane out of these values

		for (int i=3;i<12;i++)
		{
			cout << floats[i] <<endl;
		}
	};

	int getOrientation();
	int getPostion();
	void setContrast();
	void setBrightNess();
//private:
	string name;
	u_int width;
	u_int height;
	float thickness;
	float floats[14];
	float pixelSizeX, pixelSizeY;
};

#include <vector>

class ImageSequence
{
	u_int numberOfFrames;
	vector<DICOMImage*> images;
};

class ImageSlice
{
	u_int sliceNumber;
	ImageSequence imageSequence;
};

class ImageGroup // a bunch of image slices i.e LA & SA
{
	u_int numberOfImageSlices; //redendant?
	vector<ImageSlice*> imageSlices;
	string groupName; //either SA or LA
};

class ImageSet // The whole lot ImageManager??
{
	vector<ImageGroup*> imageGroups;

	//or
	ImageGroup LA;
	ImageGroup SA;
};

int testDICOMImage(const string& filename)
{
//	try
//	{
		DICOMImage a(filename);
//	}
//	catch(int)
//	{
//		return 1;
//	}

	return 0;

//	gdcm::StringFilter sf;
//	gdcm::Reader r;
//	r.SetFileName( filename.c_str() );
//	if( !r.Read() )
//	{
//		return 1;
//	}
//	gdcm::DataSet const& ds = r.GetFile().GetDataSet();
//	sf.SetFile( r.GetFile() );
//
//	int ret = 0;
//	gdcm::DataSet::ConstIterator it = ds.Begin();
//	for( ; it != ds.End(); ++it)
//	{
//		const gdcm::DataElement &ref = *it;
//		std::pair<std::string, std::string> s = sf.ToStringPair( ref.GetTag() );
//
//		gdcm::Tag tag = ref.GetTag();
//
//		// HERE you select de tags you want....
//
//		gdcm::Tag seriesDesc(0x0008,0x103e); //  =tag of "Series description" => [0008,103E]
//		gdcm::Tag seriesInstUID(0x0020,0x000e); //  =tag of "Series Instance UID" => [0020,000E]
//		gdcm::Tag nbSlices(0x0054,0x0081); //  =tag of "Number of Slices" => [0054,0081]
//
//
//		if(tag == seriesDesc || tag == seriesInstUID || tag == nbSlices){
//			if( !s.second.empty() == 0)
//			{
//				std::cout << /*s.first << " -> "*/ tag << " " << s.second << std::endl;
//			}
//		}
//	}

	//getPlaneInformationFromDicomImageHeader()
    /*
        comment for the variables - JGB - 2007/12/05
        integers[0];   columns (0028, 0011)
        integers[1];   rows (0028, 0010)
        floats[0];     FOV if exists in header (obsolete)
        floats[1];     slice thickness (0018, 0050)
        floats[2];     offset ??
        floats[3-5];   image position x, y,z (0020, 0032)
        floats[6-11];  image orientation (0020, 0037)
        floats[6-8];   direction cosine of the 1st row
        floats[9-11];  direction cosine of the 1st column
        floats[12-13]; pixel spacing x, y (0028, 0030)
      */

//	gdcm::Tag orientation(0x0020, 0x0037);
//	if (ds.FindDataElement(orientation))
//	{
//		const gdcm::DataElement& de = ds.GetDataElement(orientation);
//		std::pair<std::string, std::string> s = sf.ToStringPair( de.GetTag() );
//		std::cout << orientation << "= " << s.first << "==> " << s.second << std::endl;
//		std::cout << de << endl;
//	}

//	const gdcm::DataElement& rows = ds.GetDataElement(gdcm::Tag(0x0028,0x0010));
//	const gdcm::ByteValue* value = rows.GetByteValue();
//	std::pair<std::string, std::string> s = sf.ToStringPair( rows.GetTag() );
//	std::cout << s.first << "==> " << s.second << std::endl;
//	value->PrintHex(cout, value->GetLength());
//	cout << ": " << *(reinterpret_cast<const u_short*>((value->GetPointer())));
//	cout << endl;
//
//	const gdcm::DataElement& cols = ds.GetDataElement(gdcm::Tag(0x0028,0x0011));
//	value = cols.GetByteValue();
//	s = sf.ToStringPair( cols.GetTag() );
//	std::cout << s.first << "==> " << s.second << std::endl;
//	value->PrintHex(cout, value->GetLength());
//	cout << ": " << *(reinterpret_cast<const u_short*>((value->GetPointer())));
//	cout << endl;
//
//	cout << "Rows: " << ds.GetDataElement(gdcm::Tag(0x0028,0x0010)) << endl;
//	cout << "Cols: " << ds.GetDataElement(gdcm::Tag(0x0028,0x0011)) << endl;
//	cout << "Thic: " << ds.GetDataElement(gdcm::Tag(0x0018,0x0050)) << endl;
//	cout << "Posi: " << ds.GetDataElement(gdcm::Tag(0x0020,0x0032)) << endl;
//	cout << "Orie: " << ds.GetDataElement(gdcm::Tag(0x0020,0x0037)) << endl;
//	cout << "Spac: " << ds.GetDataElement(gdcm::Tag(0x0028,0x0030)) << endl;

//	return ret;
}

struct Point3D
{
	float x,y,z;
	friend std::ostream& operator<<(std::ostream &_os, const Point3D &val);
};

inline std::ostream& operator<<(std::ostream &os, const Point3D &val)
{
  os << "( " << val.x << ", " << val.y << ", " << val.z << ") ";
  return os;
};

struct ImagePlane
{
  int registered;

  Point3D tlc;            /**< top left corner */
  Point3D trc;            /**< top right corner */
  Point3D blc;            /**< bottom left corner */
  Point3D brc;            /**< bottom right corner */
  Point3D normal;         /**< normal of the plane */

  Point3D xside;          /**< vector from blc to brc */
  Point3D yside;          /**< vector from blc to tlc */

  int            imageSize;      /**< also stored in StudyInfo - image square */
  float          fieldOfView;    /**< should match length of xside */
  float          sliceThickness; /**< may vary between series */
  float          sliceGap;       /**< non-zero for short axis only */
  float          pixelSizeX;     /**< in mm, should be square */
  float          pixelSizeY;     /**< in mm, should be square */
};

#include <algorithm>

#define FIND_VECTOR(ans, from, to) \
(ans).x = (to).x - (from).x; \
(ans).y = (to).y - (from).y; \
(ans).z = (to).z - (from).z

#define DOT(a,b) ( (a).x*(b).x + (a).y*(b).y + (a).z*(b).z )

#define CROSS_PRODUCT(ans, vec1, vec2) \
(ans).x = (vec1).y*(vec2).z - (vec1).z*(vec2).y; \
(ans).y = (vec1).z*(vec2).x - (vec1).x*(vec2).z; \
(ans).z = (vec1).x*(vec2).y - (vec1).y*(vec2).x

#define NORMALISE(vec) \
{ \
  double length = sqrt(DOT((vec), (vec))); \
  (vec).x = (vec).x / length; \
  (vec).y = (vec).y / length; \
  (vec).z = (vec).z / length; \
}

ImagePlane* getImagePlaneFromDICOMHeaderInfo(const string& filename)
{

	DICOMImage image(filename);

	int width = image.width, height = image.height;
    int imageSize = std::max<u_int>(width,height);
    cout << "imageSize: " << imageSize << endl;
    float* floats = image.floats;

	//pixelSizeX = (data->imagesSubsampled[se][sl]) ?
	//(floats[12]*2.0) : floats[12];
	int pixelSizeX = image.pixelSizeX;

	ImagePlane* plane = new ImagePlane();
    //plane = data->plane[se][sl];
	plane->imageSize = imageSize;

	int fieldOfView = pixelSizeX * (float)(imageSize);
	cout << "fieldOfView: " << fieldOfView << endl;

	// JGB - 2007/12/05 - plane's tlc starts from the edge of the first voxel
	// rather than centre; (0020, 0032) is the centre of the first voxel
	float tlcex, tlcey, tlcez; // top left corner edge
	tlcex = floats[3] - pixelSizeX * (0.5f*floats[6] + 0.5f*floats[9]);
	tlcey = floats[4] - pixelSizeX * (0.5f*floats[7] + 0.5f*floats[10]);
	tlcez = floats[5] - pixelSizeX * (0.5f*floats[8] + 0.5f*floats[11]);

	plane->tlc.x = tlcex - pixelSizeX * (
	0.5*(imageSize - width) * floats[6]
	+ 0.5*(imageSize - height) * floats[9]);
	plane->tlc.y = tlcey - pixelSizeX * (
	0.5*(imageSize - width) * floats[7]
	+ 0.5*(imageSize - height) * floats[10]);
	plane->tlc.z = tlcez - pixelSizeX * (
	0.5*(imageSize - width) * floats[8]
	+ 0.5*(imageSize - height) * floats[11]);

	plane->trc.x = plane->tlc.x + fieldOfView*floats[6];
	plane->trc.y = plane->tlc.y + fieldOfView*floats[7];
	plane->trc.z = plane->tlc.z + fieldOfView*floats[8];

	plane->blc.x = plane->tlc.x + fieldOfView*floats[9];
	plane->blc.y = plane->tlc.y + fieldOfView*floats[10];
	plane->blc.z = plane->tlc.z + fieldOfView*floats[11];

	FIND_VECTOR(plane->xside, plane->tlc, plane->trc);
	FIND_VECTOR(plane->yside, plane->blc, plane->tlc);
	CROSS_PRODUCT(plane->normal, plane->xside, plane->yside);
	NORMALISE(plane->normal);

	plane->brc.x = plane->blc.x + plane->xside.x;
	plane->brc.y = plane->blc.y + plane->xside.y;
	plane->brc.z = plane->blc.z + plane->xside.z;

	std::cout << plane->trc << endl;
	std::cout << plane->tlc << endl;
	std::cout << plane->brc << endl;
	std::cout << plane->blc << endl;

	return plane;
}
#endif /* DICOMIMAGE_H_ */
