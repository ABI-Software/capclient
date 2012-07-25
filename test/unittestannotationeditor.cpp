/*
 * UnitTestAnnotationEditor.cpp
 *
 *  Created on: Feb 25, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#include "tools/annotationeditor/annotationeditor.h"
#include "tools/annotationeditor/iannotationwindow.h"
#include "io/annotationfile.h"

#include "unittestconfigure.h"

#include <boost/scoped_ptr.hpp>

namespace
{
    std::string test_file(SAMPLEANNOTATION_FILE);
}

namespace cap
{

class TestAnnotationWindow : public IAnnotationWindow
{
public:
	TestAnnotationWindow(AnnotationEditor& editor)
	:
		editor_(editor)
	{}
	
	virtual void UpdateAnnotationDisplay(ImageAnnotation const& anno)
	{
		imageAnno = anno;
	}
	
	ImageAnnotation imageAnno;
private:
	AnnotationEditor& editor_;
};
} // namespace cap

class AnnotationEditorTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		using namespace cap;
		win = new TestAnnotationWindow(editor);
		editor.SetWindow(win);
		
		annoFilePtr.reset(new AnnotationFile(test_file));
		annoFilePtr->ReadFile();

		CardiacAnnotation anno(annoFilePtr->GetCardiacAnnotation());
		editor.SetCardiacAnnotation(anno);
		
		std::string sopiuid("2.16.124.113543.6006.99.07104390983379737982");
		editor.InitializeAnnotationWindow(sopiuid);
	}
	
	cap::AnnotationEditor editor;
	cap::TestAnnotationWindow* win;
	boost::scoped_ptr<cap::AnnotationFile> annoFilePtr;
};

TEST_F(AnnotationEditorTest, GetCardiacAnnotation)
{
	using namespace cap;
	
	CardiacAnnotation const& anno_org = annoFilePtr->GetCardiacAnnotation();
	CardiacAnnotation const& anno = editor.GetCardiacAnnotation();
	
	EXPECT_EQ(anno.studyiuid, anno_org.studyiuid);
	EXPECT_EQ(anno.imageAnnotations.size(), anno_org.imageAnnotations.size());
	
	size_t numImageAnnotations = anno.imageAnnotations.size();
	
	std::string sopiuid("2.16.124.113543.6006.99.07104390983379737982");
	EXPECT_EQ(anno.imageAnnotations.at(0).sopiuid, sopiuid);
	
	for (size_t i = 0; i<numImageAnnotations; ++i)
	{
		EXPECT_EQ(anno.imageAnnotations[i].sopiuid, anno_org.imageAnnotations[i].sopiuid);
	}
}

TEST_F(AnnotationEditorTest, InitializeAnnotationWindow)
{
	using namespace cap;
	
	std::string sopiuid("2.16.124.113543.6006.99.07104390983379737982");
	editor.InitializeAnnotationWindow(sopiuid);
	
	ImageAnnotation const& imageAnnotation = win->imageAnno;
	EXPECT_EQ(sopiuid, imageAnnotation.sopiuid);
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).rid, "SNOMEDID:T-32004");
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).scope, "Slice");
	EXPECT_EQ(imageAnnotation.rOIs.at(0).labels.at(0).label, "Apex of Heart");
	EXPECT_DOUBLE_EQ(imageAnnotation.rOIs.at(0).points.at(0).x, 125.04440497335702);
	EXPECT_DOUBLE_EQ(imageAnnotation.rOIs.at(0).points.at(0).y, 117.31438721136767);
	EXPECT_EQ(imageAnnotation.rOIs.at(0).points.at(0).number, -1);
	
	size_t numLabels = imageAnnotation.labels.size();
	EXPECT_EQ(numLabels, 3);
	EXPECT_EQ(imageAnnotation.labels.at(0).rid, "RID:10741");
	EXPECT_EQ(imageAnnotation.labels.at(0).scope, "Series");
	EXPECT_EQ(imageAnnotation.labels.at(0).label, "Fast Gradient Echo");
}

TEST_F(AnnotationEditorTest, InitializeAnnotationWindowWithEmptyAnnotation)
{
	using namespace cap;
	
	std::string sopiuid("fake.uid");
	editor.InitializeAnnotationWindow(sopiuid);
	
	ImageAnnotation const& imageAnnotation = win->imageAnno;
	EXPECT_EQ(sopiuid, imageAnnotation.sopiuid);
	EXPECT_EQ(imageAnnotation.rOIs.size(), 0);
	size_t numLabels = imageAnnotation.labels.size();
	EXPECT_EQ(numLabels, 0);
}

TEST_F(AnnotationEditorTest, AddLabel)
{
	using namespace cap;
	
	Label label = {"rid", "Instance", "label"};
	editor.AddLabel(label);
	
	size_t numLabels = win->imageAnno.labels.size();
	EXPECT_EQ(numLabels, 4);
	
	EXPECT_EQ(win->imageAnno.labels.at(3).rid, "rid");
	EXPECT_EQ(win->imageAnno.labels.at(3).scope, "Instance");
	EXPECT_EQ(win->imageAnno.labels.at(3).label, "label");
}

TEST_F(AnnotationEditorTest, RemoveLabel)
{
	using namespace cap;
	
	editor.RemoveLabel(1);

	ImageAnnotation const& imageAnnotation = win->imageAnno;
	size_t numLabels = imageAnnotation.labels.size();
	EXPECT_EQ(numLabels, 2);
	
	EXPECT_EQ(imageAnnotation.labels.at(0).rid, "RID:10741");
	EXPECT_EQ(imageAnnotation.labels.at(0).scope, "Series");
	EXPECT_EQ(imageAnnotation.labels.at(0).label, "Fast Gradient Echo");
	
	EXPECT_EQ(imageAnnotation.labels.at(1).rid, "RID:10577");
	EXPECT_EQ(imageAnnotation.labels.at(1).scope, "Instance");
	EXPECT_EQ(imageAnnotation.labels.at(1).label, "Short Axis");
}


TEST_F(AnnotationEditorTest, EditLabel)
{
	using namespace cap;
	
	Label label = {"rid", "Instance", "label"};
	editor.EditLabel(1,label);
	
	ImageAnnotation const& imageAnnotation = win->imageAnno;
	size_t numLabels = imageAnnotation.labels.size();
	EXPECT_EQ(numLabels, 3);
	
	EXPECT_EQ(imageAnnotation.labels.at(0).rid, "RID:10741");
	EXPECT_EQ(imageAnnotation.labels.at(0).scope, "Series");
	EXPECT_EQ(imageAnnotation.labels.at(0).label, "Fast Gradient Echo");
	
	EXPECT_EQ(imageAnnotation.labels.at(2).rid, "RID:10577");
	EXPECT_EQ(imageAnnotation.labels.at(2).scope, "Instance");
	EXPECT_EQ(imageAnnotation.labels.at(2).label, "Short Axis");
	
	EXPECT_EQ(imageAnnotation.labels.at(1).rid, "rid");
	EXPECT_EQ(imageAnnotation.labels.at(1).scope, "Instance");
	EXPECT_EQ(imageAnnotation.labels.at(1).label, "label");
}
