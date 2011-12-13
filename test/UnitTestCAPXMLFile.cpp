/*
 * UnitTestCAPXMLFile.cpp
 *
 *  Created on: Jun 4, 2010
 *      Author: jchu014
 */

#include <boost/foreach.hpp>

#include <iostream>
#include <sstream>

#include <gtest/gtest.h>
#include <stdexcept>
#include <cstdio>
#include "unittestconfigure.h"
#define private public
#include "CAPXMLFile.h"
#undef private
#include "CAPXMLFileHandler.h"
#include "datapoint.h"
#include "logmsg.h"

namespace
{
	std::string test_file("SampleAnalysisUsingXsd.xml");
}


namespace cap
{
	std::string TimeNow() { return ""; }
	Log::~Log() {}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;
}

TEST(CAPXMLFile, ReadXML)
{
	using namespace cap;
	
	CAPXMLFile xmlFile(SAMPLEANALYSISXML_FILE);

	xmlFile.ReadFile();
	ASSERT_EQ("SampleAnalysisUsingXsd", xmlFile.name_);
	EXPECT_EQ( "LV", xmlFile.chamber_);
	EXPECT_EQ(10.0, xmlFile.output_.focalLength);
	EXPECT_EQ(0.3, xmlFile.output_.interval);
	EXPECT_EQ("2.16.124.113543.6006.99.03832048922002137666", xmlFile.studyIUid_);
	
	CAPXMLFile::Image& image = xmlFile.GetInput().images.at(0);
	EXPECT_TRUE(image.imagePosition);
	EXPECT_EQ(Point3D(4.0, 5.0, 6.0), *(image.imagePosition));
	
	EXPECT_TRUE(image.imageOrientation);
	EXPECT_EQ(Vector3D(7.0, 8.0, 9.0), image.imageOrientation->first);
	EXPECT_EQ(Vector3D(10.0, 11.0, 12.0), image.imageOrientation->second);

//	EXPECT_EQ(image.contourFiles.at(0).fileName, "cap:ContourFile");
//	EXPECT_EQ(image.contourFiles.at(0).number, 1);
	
	CAPXMLFile::Exnode& exnode = xmlFile.GetOutput().exnodes.at(0);
	EXPECT_EQ("cap.exnode", exnode.exnode);
	EXPECT_EQ(1, exnode.frame);

	CAPXMLFile::ProvenanceDetail& pd = xmlFile.documentation_.provenanceDetails[0];
	EXPECT_EQ("Converted from CIM model", pd.comment);
	// add more tests
}

TEST(CAPXMLFile, WriteXML)
{
	using namespace cap;
		
	CAPXMLFile xmlFile(SAMPLEANALYSISXML_FILE);

	xmlFile.ReadFile();
	ASSERT_EQ("SampleAnalysisUsingXsd", xmlFile.name_);

	CAPXMLFile::Point p;
	p.time = 0.56777;
	p.type = BASEPLANE;
	//p.values = std::map<std::string
	xmlFile.AddPoint(p);
	xmlFile.WriteFile("dummy");
	// add tests here

	CAPXMLFile xmlFile2("dummy");
	xmlFile2.ReadFile();
	
	EXPECT_EQ(xmlFile.chamber_, xmlFile2.chamber_);
	EXPECT_EQ(xmlFile.output_.focalLength, xmlFile2.output_.focalLength);
	EXPECT_EQ(xmlFile.output_.interval, xmlFile2.output_.interval);
	EXPECT_EQ(xmlFile.name_, xmlFile2.name_);
	EXPECT_EQ(xmlFile.studyIUid_, xmlFile2.studyIUid_);
	EXPECT_EQ(xmlFile.input_.points.size(), xmlFile2.input_.points.size());

	CAPXMLFile::Image& image = xmlFile.GetInput().images.at(0);
	CAPXMLFile::Image& image2 = xmlFile2.GetInput().images.at(0);
	
	EXPECT_EQ(*(image.imagePosition), *(image2.imagePosition));
	
	EXPECT_EQ(image.imageOrientation->first, image2.imageOrientation->first);
	EXPECT_EQ(image.imageOrientation->second, image2.imageOrientation->second);

//	EXPECT_EQ(image.contourFiles.at(0).fileName, image2.contourFiles.at(0).fileName);
//	EXPECT_EQ(image.contourFiles.at(0).number, image2.contourFiles.at(0).number);
	
	CAPXMLFile::Exnode& exnode = xmlFile.GetOutput().exnodes.at(0);
	CAPXMLFile::Exnode& exnode2 = xmlFile2.GetOutput().exnodes.at(0);
	EXPECT_EQ(exnode.exnode, exnode2.exnode);
	EXPECT_EQ(exnode.frame, exnode2.frame);

	CAPXMLFile::ProvenanceDetail& pd = xmlFile.documentation_.provenanceDetails[0];
	CAPXMLFile::ProvenanceDetail& pd2 = xmlFile2.documentation_.provenanceDetails[0];
	EXPECT_EQ(pd.comment, pd2.comment);
	
	//EXPECT_EQ(0, remove("dummy"));
}

TEST(CAPXMLFile, AddImage)
{
	using namespace cap;
		
	CAPXMLFile xmlFile("unused.file");

	CAPXMLFile::Image image;
	image.sopiuid = "111";
	xmlFile.AddImage(image);
	EXPECT_EQ("111", xmlFile.input_.images[0].sopiuid);
	
	CAPXMLFile::Point p;
	p.surface = EPICARDIUM;
	p.type = GUIDEPOINT;
	EXPECT_NO_THROW(xmlFile.AddPointToImage("111", p));
	EXPECT_EQ(EPICARDIUM, xmlFile.input_.images[0].points[0].surface);
	EXPECT_EQ(GUIDEPOINT, xmlFile.input_.images[0].points[0].type);
	
	EXPECT_THROW(xmlFile.AddPointToImage("222", CAPXMLFile::Point()), std::invalid_argument);
}

TEST(CAPXMLFile, GetInput)
{
	using namespace cap;
	
	CAPXMLFile xmlFile(SAMPLEIMAGES_FILE);

	xmlFile.ReadFile();
	ASSERT_EQ("SampleImages", xmlFile.name_);
	EXPECT_EQ( "LV", xmlFile.chamber_);
	EXPECT_EQ(1.0, xmlFile.output_.focalLength);
	EXPECT_EQ(0.25, xmlFile.output_.interval);
	EXPECT_EQ("1.3.12.2.1107.5.2.6.22043.4.0.6860603417842558", xmlFile.studyIUid_);
	
	CAPXMLFile::Input& input = xmlFile.GetInput();
	EXPECT_EQ(17, input.images.size());
	BOOST_FOREACH(CAPXMLFile::Image const& image, input.images)
	{
		std::cout << image.label << std::endl;
	}

	CAPXMLFile::Input& inputAgain = xmlFile.GetInput();
	EXPECT_EQ(17, inputAgain.images.size());
	BOOST_FOREACH(CAPXMLFile::Image const& image, inputAgain.images)
	{
		std::cout << image.label << std::endl;
	}
}

TEST(CAPXMLFileHandler, GetDataPoints)
{
	using namespace cap;

	CAPXMLFile xmlFile(SAMPLEIMAGES_FILE);
	xmlFile.ReadFile();
	ASSERT_EQ("SampleImages", xmlFile.name_);

	CAPXMLFileHandler fh(xmlFile);
	std::vector<DataPoint> pts = fh.GetDataPoints();
	EXPECT_EQ(0, pts.size());
}

// This test should be moved to a gui test.
//TEST(CAPXMLFileHandler, GetLabelledSlices)
//{
//	using namespace cap;
//
//	CAPXMLFile xmlFile(SAMPLEIMAGES_FILE);
//	xmlFile.ReadFile();
//	ASSERT_EQ("SampleImages", xmlFile.name_);
//
//	CAPXMLFileHandler fh(xmlFile);
//	LabelledSlices ls = fh.GetLabelledSlices();
//	EXPECT_EQ(3, ls.size());
//}


