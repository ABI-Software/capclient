/*
 * UnitTestCAPXMLFile.cpp
 *
 *  Created on: Jun 4, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#include <stdexcept>
#include <cstdio>
#define private public
#include "../src/CAPXMLFile.h"
#undef private

namespace
{
	std::string test_file("../test/SampleAnalysisUsingXsd.xml");
}
TEST(CAPXMLFile, ReadXML)
{
	using namespace cap;
	
	CAPXMLFile xmlFile(test_file);

	xmlFile.ReadFile();
	EXPECT_EQ(xmlFile.chamber_, "LV");
	EXPECT_EQ(xmlFile.output_.focalLength, 10.0);
	EXPECT_EQ(xmlFile.output_.interval, 0.3);
	EXPECT_EQ(xmlFile.name_, "SampleAnalysisUsingXsd");
	EXPECT_EQ(xmlFile.studyIUid_, "2.16.124.113543.6006.99.03832048922002137666");
	
	CAPXMLFile::Image& image = xmlFile.GetInput().images.at(0);
	EXPECT_TRUE(image.imagePosition);
	EXPECT_EQ(*(image.imagePosition), Point3D(4.0, 5.0, 6.0));
	
	EXPECT_TRUE(image.imageOrientation);
	EXPECT_EQ(image.imageOrientation->first, Vector3D(7.0, 8.0, 9.0));
	EXPECT_EQ(image.imageOrientation->second, Vector3D(10.0, 11.0, 12.0));

	EXPECT_EQ(image.contourFiles.at(0).fileName, "cap:ContourFile");
	EXPECT_EQ(image.contourFiles.at(0).number, 1);
	
	CAPXMLFile::Exnode& exnode = xmlFile.GetOutput().exnodes.at(0);
	EXPECT_EQ(exnode.exnode, "cap.exnode");
	EXPECT_EQ(exnode.frame, 1);

	CAPXMLFile::ProvenanceDetail& pd = xmlFile.documentation_.provenanceDetails[0];
	EXPECT_EQ(pd.comment, "Converted from CIM model");
	// add more tests
}

TEST(CAPXMLFile, WriteXML)
{
	using namespace cap;
		
	CAPXMLFile xmlFile(test_file);

	xmlFile.ReadFile();
	xmlFile.WriteFile("dummy");
	// add tests here

	CAPXMLFile xmlFile2("dummy");
	xmlFile2.ReadFile();
	
	EXPECT_EQ(xmlFile.chamber_, xmlFile2.chamber_);
	EXPECT_EQ(xmlFile.output_.focalLength, xmlFile2.output_.focalLength);
	EXPECT_EQ(xmlFile.output_.interval, xmlFile2.output_.interval);
	EXPECT_EQ(xmlFile.name_, xmlFile2.name_);
	EXPECT_EQ(xmlFile.studyIUid_, xmlFile2.studyIUid_);
	
	CAPXMLFile::Image& image = xmlFile.GetInput().images.at(0);
	CAPXMLFile::Image& image2 = xmlFile2.GetInput().images.at(0);
	
	EXPECT_EQ(*(image.imagePosition), *(image2.imagePosition));
	
	EXPECT_EQ(image.imageOrientation->first, image2.imageOrientation->first);
	EXPECT_EQ(image.imageOrientation->second, image2.imageOrientation->second);

	EXPECT_EQ(image.contourFiles.at(0).fileName, image2.contourFiles.at(0).fileName);
	EXPECT_EQ(image.contourFiles.at(0).number, image2.contourFiles.at(0).number);
	
	CAPXMLFile::Exnode& exnode = xmlFile.GetOutput().exnodes.at(0);
	CAPXMLFile::Exnode& exnode2 = xmlFile2.GetOutput().exnodes.at(0);
	EXPECT_EQ(exnode.exnode, exnode2.exnode);
	EXPECT_EQ(exnode.frame, exnode2.frame);

	CAPXMLFile::ProvenanceDetail& pd = xmlFile.documentation_.provenanceDetails[0];
	CAPXMLFile::ProvenanceDetail& pd2 = xmlFile2.documentation_.provenanceDetails[0];
	EXPECT_EQ(pd.comment, pd2.comment);
	
	EXPECT_EQ(remove("dummy"), 0);
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
