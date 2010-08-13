/*
 * CAPXMLFile.h
 *
 *  Created on: May 25, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILE_H_
#define CAPXMLFILE_H_

#include "CAPTypes.h"
#include "CAPMath.h"

#include <string>
#include <vector>
#include <map>

namespace cap
{

class CAPModelLVPS4X4;
class DataPoint;
class Point3D;
class Vector3D;
class CmguiManager;

struct Value
{
	double value;
	std::string variable; // lambda, mu, theta or x, y, z
};

//struct Surface // consider enum?
//{
//	static std::string const EPI;
//	static std::string const ENDO;
//};

struct Point
{
	std::map<std::string, Value> values;
	SurfaceType surface;
	DataPointType type;
};

struct ContourFile
{
	std::string fileName;
};

struct Image
{
	std::vector<Point> points;
	std::vector<ContourFile> countourFiles;
	std::string sopiuid;
	int frame;
	int slice;
	std::string label;//LA1, SA2 etc
	std::tr1::shared_ptr<Point3D> imagePosition;
	std::tr1::shared_ptr<std::pair<Vector3D, Vector3D> > imageOrientation;
};

struct Frame
{
	std::string exnode;
	int number;
};

struct Version
{
	int number;
	std::string date;
	std::string log;
};

struct History
{
	std::string entry;
	std::string date;
};

struct CAPXMLInput
{
	std::vector<Image> images;
};

struct CAPXMLOutput
{
	double focalLength;
	double interval;
	std::string transformationMatrix;
	std::vector<Frame> frames;
	std::string elemFileName;
};

struct Documentation
{
	Version version;
	History history;
};

class CAPXMLFile
{
public:
	explicit CAPXMLFile(std::string const & filename);
	~CAPXMLFile();// need this so compiler wont generate dtor
	
	void ReadFile();
	void WriteFile(std::string const & filename) const;
	
	void AddImage(Image const& image);
	void AddPointToImage(std::string const& imageSopiuid, Point const& point);
	void AddContourFileToImage(std::string const& imageSopiuid, ContourFile const& contourFile);
	
	void AddFrame(Frame const& frame);
	

//	/**
//	 *  Populate member fields of CAPXMLFile from infomation obtained from
//	 *  SlicesWithImages, vector<DataPoint> and CAPModelLVPS4X4
//	 *  so an libxml2 tree can be generated and written to a file
//	 */
//	void ContructCAPXMLFile(SlicesWithImages const& dicomFiles, 
//							std::vector<DataPoint> const& dataPoints,
//							CAPModelLVPS4X4 const& model);
//	
//	/**
//	 *  Translate the infomation stored in CAPXMLFile into the form to be
//	 *  consumed by the client
//	 *  (i.e SlicesWithImages, vector<DataPoint> and CAPModelLVPS4X4)
//	 *  Also generated the instances of DICOMImage, Cmiss_texture,
//	 *  DataPoint (and the Cmiss_node) and the model.
//	 */
//	SlicesWithImages GetSlicesWithImages(CmguiManager const& cmguiManager) const;
//
//	std::vector<DataPoint> GetDataPoints(CmguiManager const& cmguiManager) const;

	std::vector<std::string> GetExnodeFileNames() const;

	std::string const& GetExelemFileName() const;

	void GetTransformationMatrix(gtMatrix& mat) const;

	double GetFocalLength() const;

//	std::vector<Image> const& GetImages() const
//	{
//		return input_.images;
//	}
	
	void Clear()
	{
		input_.images.clear();
		output_.frames.clear();
	}
	
	std::string const& GetName() const
	{
		return name_;
	}
	
	void SetName(std::string const& name)
	{
		name_ = name;
	}
	
	std::string const& GetFilename() const
	{
		return filename_;
	}
	
	void SetFilename(std::string const& filename)
	{
		filename_ = filename;
	}
	
	std::string const& GetChamber() const
	{
		return chamber_;
	}
	
	void SetChamber(std::string const& chamber)
	{
		chamber_ = chamber;
	}
	
	std::string const& GetStudyInstanceUID() const
	{
		return studyIUid_;
	}
	
	void SetStudyInstanceUID(std::string const& uid)
	{
		studyIUid_ = uid;
	}
	
	CAPXMLInput& GetInput()
	{
		return input_;
	}
	
	CAPXMLOutput& GetOutput()
	{
		return output_;
	}
	
private:
	std::string filename_;
	std::string chamber_; // LV
	std::string name_; // 
	std::string studyIUid_; //
	
	// input
	// images (frame, slice, uid, LABEL)
	//   point (Surface, type - apex base rv bp guide)
	//      value (value, variable)
	//   ContourFile
	// 
	
	// output
	//   Exnode frame="0"
	//   Exelem
	
	// documentation
	//   version
	//   history

	CAPXMLInput input_;
	CAPXMLOutput output_;
	Documentation documentation_;
};

} // end namespace cap
#endif /* CAPXMLFILE_H_ */
