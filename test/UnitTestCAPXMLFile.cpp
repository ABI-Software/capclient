/*
 * UnitTestCAPXMLFile.cpp
 *
 *  Created on: Jun 4, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#include <stdexcept>
#define private public
#include "../src/CAPXMLFile.h"
#include "../src/DataPoint.h"
#undef private

/*
 * fake defintion of DataPoint for testing
 *
 */

namespace cap
{

DataPoint::DataPoint(Cmiss_node* node, const Point3D& coord, DataPointType dataPointType, float time, float weight)
{};
	
DataPoint::DataPoint(const DataPoint& other)
{};
		
DataPoint::~DataPoint()
{}
	
const Cmiss_node* DataPoint::GetCmissNode() const
{}

Cmiss_node* DataPoint::GetCmissNode()
{}

const Point3D& DataPoint::GetCoordinate() const
{}

void DataPoint::SetCoordinate(const Point3D& coord)
{}

float DataPoint::GetTime() const
{}

void DataPoint::SetValidPeriod(float startTime, float endTime)
{}

void DataPoint::SetVisible(bool visibility)
{
}

DataPointType DataPoint::GetDataPointType() const
{
}

void DataPoint::SetDataPointType(DataPointType type)
{
}

SurfaceType DataPoint::GetSurfaceType() const
{
}

void DataPoint::SetSurfaceType(SurfaceType type)
{
}

std::string DataPoint::GetSliceName() const
{
}

// assignment operator
DataPoint& DataPoint::operator=(const DataPoint& rhs)
{
}
	
} // end namespace cap


TEST(CAPXMLFile, ReadXML)
{
	using namespace cap;
	
	CAPXMLFile xmlFile("test/SampleAnalysisUsingXsd.xml");

	xmlFile.ReadFile();
	EXPECT_EQ(xmlFile.chamber_, "LV");
	EXPECT_EQ(xmlFile.focalLength_, 0.0);
	EXPECT_EQ(xmlFile.interval_, 0.0);
	EXPECT_EQ(xmlFile.name_, "SampleAnalysisUsingXsd");
	EXPECT_EQ(xmlFile.studyIUid_, "2.16.124.113543.6006.99.03832048922002137666");
	
	// add more tests
}

TEST(CAPXMLFile, WriteXML)
{
	using namespace cap;
		
	CAPXMLFile xmlFile("test/SampleAnalysisUsingXsd.xml");

	xmlFile.ReadFile();
	xmlFile.WriteFile("dummy");
	// add tests here

}

TEST(CAPXMLFile, AddImage)
{
	using namespace cap;
		
	CAPXMLFile xmlFile("test/SampleAnalysisUsingXsd.xml");

	Image image;
	image.sopiuid = "111";
	xmlFile.AddImage(image);
	EXPECT_EQ(xmlFile.input_.images[0].sopiuid, "111");
	
	Point p;
	p.surface = EPICARDIUM;
	p.type = GUIDEPOINT;
	EXPECT_NO_THROW(xmlFile.AddPointToImage("111", p));
	EXPECT_EQ(xmlFile.input_.images[0].points[0].surface, EPICARDIUM );
	EXPECT_EQ(xmlFile.input_.images[0].points[0].type, GUIDEPOINT );
	
	EXPECT_THROW(xmlFile.AddPointToImage("222", Point()), std::invalid_argument);
}
