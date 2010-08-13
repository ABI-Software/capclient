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
#undef private

TEST(CAPXMLFile, ReadXML)
{
	using namespace cap;
	
	CAPXMLFile xmlFile("test/SampleAnalysisUsingXsd.xml");

	xmlFile.ReadFile();
	EXPECT_EQ(xmlFile.chamber_, "LV");
	EXPECT_EQ(xmlFile.output_.focalLength, 0.0);
	EXPECT_EQ(xmlFile.output_.interval, 0.0);
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

	CAPXMLFile::Image image;
	image.sopiuid = "111";
	xmlFile.AddImage(image);
	EXPECT_EQ(xmlFile.input_.images[0].sopiuid, "111");
	
	CAPXMLFile::Point p;
	p.surface = EPICARDIUM;
	p.type = GUIDEPOINT;
	EXPECT_NO_THROW(xmlFile.AddPointToImage("111", p));
	EXPECT_EQ(xmlFile.input_.images[0].points[0].surface, EPICARDIUM );
	EXPECT_EQ(xmlFile.input_.images[0].points[0].type, GUIDEPOINT );
	
	EXPECT_THROW(xmlFile.AddPointToImage("222", CAPXMLFile::Point()), std::invalid_argument);
}
