/*
 * UnitTestCAPXMLFile.cpp
 *
 *  Created on: Jun 4, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#define private public
#include "../src/CAPXMLFile.h"
#undef private

TEST(CAPXMLFile, ReadXML)
{
	using namespace cap;
	
	CAPXMLFile xmlFile("SampleAnalysisUsingXsd.xml");

	xmlFile.ReadFile();
	EXPECT_EQ(xmlFile.chamber_, "LV");
	EXPECT_EQ(xmlFile.focalLength_, 0.0);
	EXPECT_EQ(xmlFile.interval_, 0.0);
	EXPECT_EQ(xmlFile.name_, "SampleAnalysisUsingXsd");
	EXPECT_EQ(xmlFile.studyIUid_, "2.16.124.113543.6006.99.03832048922002137666");
}

TEST(CAPXMLFile, WriteXML)
{
	using namespace cap;
		
	CAPXMLFile xmlFile("SampleAnalysisUsingXsd.xml");

	xmlFile.ReadFile();
	xmlFile.WriteFile("dummy");

}

