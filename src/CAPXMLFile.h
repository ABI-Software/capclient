/*
 * CAPXMLFile.h
 *
 *  Created on: May 25, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILE_H_
#define CAPXMLFILE_H_

#include <string>
#include <vector>

namespace cap
{

class Value
{
	double value;
	std::string variable; // lambda, mu, theta
};

//class Surface // consider enum?
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

class Point
{
	Value lambda, mu, theta;
	Surface surface;
	PointType type;
};

class ContourFile
{
	std::string fileName;
};

class Image
{
	std::vector<Point> points;
	std::vector<ContourFile> countourFiles;
	std::string sopiuid;
	int frame;
	int slice;
	std::string label;//LA1, SA2 etc
};

class Frame
{
	std::string exnode;
	int number;
};

class Version
{
	int number;
	std::string date;
	std::string log;
};

class History
{
	std::string entry;
	std::string date;
};

class Input
{
	std::vector<Image> images;
};

class Output
{
	std::vector<Frame> frames;
};

class Documentation
{
	Version version;
	History history;
};

class CAPXMLFile
{
public:
	void Read();
private:
	std::string chamber; // LV
	double focalLength;
	double interval;
	std::string name; // 
	std::string studyIUid; //
	
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

	Input input;
	Output output;
	Documentation documentation;

};

} // end namespace cap
#endif /* CAPXMLFILE_H_ */
