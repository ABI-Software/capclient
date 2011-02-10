/*
 * UnitTestCAPAnnotationFile.cpp
 *
 *  Created on: Nov 8, 2010
 *      Author: jchu014
 */

/*
 * UnitTestCAPXMLFile.cpp
 *
 *  Created on: Jun 4, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>
//#include <stdexcept>
//#include <cstdio>
#define private public
#include "../src/CAPAnnotationFile.h"
#undef private

namespace
{
	std::string test_file("../test/SampleAnnotationFile.xml");
}

TEST(CAPAnnotationFile, ReadXML)
{
	using namespace cap;
	
	CAPAnnotationFile anno(test_file);
	anno.ReadFile();
	
	EXPECT_EQ(anno.cardiacAnnotation_.imageAnnotations.size(), 234);
	
	CAPAnnotationFile::ImageAnnotation& imageAnnotation =
			anno.cardiacAnnotation_.imageAnnotations.at(0);
	
	EXPECT_EQ(imageAnnotation.sopiuid, 
			std::string("2.16.124.113543.6006.99.07104390983379737982"));
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).rid, "SNOMEDID:T-32004");
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).scope, "Slice");
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).label, "Apex of Heart");
	EXPECT_DOUBLE_EQ(imageAnnotation.rOIs.at(0).points.at(0).x, 125.04440497335702);
	EXPECT_DOUBLE_EQ(imageAnnotation.rOIs.at(0).points.at(0).y, 117.31438721136767);
	EXPECT_EQ(imageAnnotation.rOIs.at(0).points.at(0).number, -1);
	
	EXPECT_EQ(imageAnnotation.labels.at(0).rid, "RID:10741");
	EXPECT_EQ(imageAnnotation.labels.at(0).scope, "Series");
	EXPECT_EQ(imageAnnotation.labels.at(0).label, "Fast Gradient Echo");
}

TEST(CAPAnnotationFile, WriteXML)
{
	using namespace cap;
	
	CAPAnnotationFile anno(test_file);
	anno.ReadFile();
	
	anno.WriteFile("dummy");
	
	CAPAnnotationFile anno2("dummy");
	anno2.ReadFile();
	
	EXPECT_EQ(anno.cardiacAnnotation_.studyiuid, anno2.cardiacAnnotation_.studyiuid);
	EXPECT_EQ(anno.cardiacAnnotation_.imageAnnotations.size(), anno2.cardiacAnnotation_.imageAnnotations.size());
	
	CAPAnnotationFile::ImageAnnotation& imageAnnotation =
				anno2.cardiacAnnotation_.imageAnnotations.at(0);
		
	EXPECT_EQ(imageAnnotation.sopiuid, 
			std::string("2.16.124.113543.6006.99.07104390983379737982"));
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).rid, "SNOMEDID:T-32004");
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).scope, "Slice");
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).label, "Apex of Heart");
	EXPECT_DOUBLE_EQ(imageAnnotation.rOIs.at(0).points.at(0).x, 125.04440497335702);
	EXPECT_DOUBLE_EQ(imageAnnotation.rOIs.at(0).points.at(0).y, 117.31438721136767);
	EXPECT_EQ(imageAnnotation.rOIs.at(0).points.at(0).number, -1);
	
	EXPECT_EQ(imageAnnotation.labels.at(0).rid, "RID:10741");
	EXPECT_EQ(imageAnnotation.labels.at(0).scope, "Series");
	EXPECT_EQ(imageAnnotation.labels.at(0).label, "Fast Gradient Echo");
		
	EXPECT_EQ(remove("dummy"), 0);
}

