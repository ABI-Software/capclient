/*
 * CAPXMLFile.h
 *
 *  Created on: May 25, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILE_H_
#define CAPXMLFILE_H_

#include "CAPTypes.h"

#include <string>
#include <vector>
#include <map>

namespace cap
{

class CAPModelLVPS4X4;

struct Value
{
	double value;
	std::string variable; // lambda, mu, theta
};

//struct Surface // consider enum?
//{
//	static std::string const EPI;
//	static std::string const ENDO;
//};

enum Surface
{
	EPI,
	ENDO
};

enum PointType
{
	APEX,
	BASE,
	RV,
	BP,
	GUIDE
};

struct Point
{
	std::map<std::string, Value> values;
	Surface surface;
	PointType type;
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

struct Input
{
	std::vector<Image> images;
};

struct Output
{
	std::vector<Frame> frames;
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
	void WriteFile(std::string const & filename);
	
	void AddImage(Image const& image);
	void AddPointToImage(std::string const& imageSopiuid, Point const& point);
	void AddContourFileToImage(std::string const& imageSopiuid, ContourFile const& contourFile);
	
	void AddFrame(Frame const& frame);
	
	void ContructCAPXMLFile(SlicesWithImages const& dicomFiles, CAPModelLVPS4X4 const& model);
	
	std::vector<Image> const& GetImages() const
	{
		return input_.images;
	}
	
	void Clear()
	{
		input_.images.clear();
		output_.frames.clear();
	}
	
	std::string const& GetName() const
	{
		return name_;
	}
	
private:
	std::string filename_;
	std::string chamber_; // LV
	double focalLength_;
	double interval_;
	std::string name_; // 
	std::string studyIUid_; //
	
	// input
	// images (frame, slice, uid, LABEL)
	//   point (Surface, type - apex base rv bp guide)
	//      value (value, variable)
	//   ContourFile
	// 
	
	// output
	//   Frame exnode="FileName0.exnode" number="0"
	
	// documentation
	//   version
	//   history

	Input input_;
	Output output_;
	Documentation documentation_;
};

} // end namespace cap
#endif /* CAPXMLFILE_H_ */
