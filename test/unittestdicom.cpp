/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */


#include "unittestconfigure.h"

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_status.h>
#include <zn/cmiss_core.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_image.h>
#include <zn/cmiss_stream.h>
}

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>

#include <gtest/gtest.h>

#include "logmsg.h"
#define private public
#include "dicomimage.h"
#undef private

namespace cap
{
    std::string TimeNow() { return ""; }
    Log::~Log() {}
    LogLevelEnum Log::reportingLevel_ = LOGDEBUG;
}

Cmiss_field_image_id Cmiss_create_image_field(Cmiss_context_id context, const std::string& filename)
{
    Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
    Cmiss_field_module_id field_module = Cmiss_region_get_field_module(root_region);

    Cmiss_field_id temp_field = Cmiss_field_module_create_image(field_module, 0, 0);
    std::string name = "tex"; //-- GetNextNameInSeries(field_module, "tex_");
    Cmiss_field_set_name(temp_field, name.c_str());
    Cmiss_field_image_id field_image = Cmiss_field_cast_image(temp_field);
    Cmiss_field_destroy(&temp_field);
    Cmiss_stream_information_id stream_information = Cmiss_field_image_create_stream_information(field_image);
    //--Cmiss_stream_information_region_id stream_information_region = Cmiss_stream_information_cast_region(stream_information);

    /* Read image data from a file */
    Cmiss_stream_resource_id stream = Cmiss_stream_information_create_resource_file(stream_information, filename.c_str());
    //--Cmiss_stream_information_region_set_attribute_real(stream_information_region, CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, 0.0);
    int r = Cmiss_field_image_read(field_image, stream_information);
    if (r == CMISS_OK)
    {
        Cmiss_field_image_set_filter_mode(field_image, CMISS_FIELD_IMAGE_FILTER_LINEAR);

        Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS, 1/*dicom_image->GetImageWidthMm()*/);
        Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS, 1/*dicom_image->GetImageHeightMm()*/);
        Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS, 1);
    }
    else
    {
        Cmiss_field_image_destroy(&field_image);
    }

    Cmiss_stream_resource_destroy(&stream);
    //--Cmiss_stream_information_region_destroy(&stream_information_region);
    Cmiss_stream_information_destroy(&stream_information);
    Cmiss_region_destroy(&root_region);
    Cmiss_field_module_destroy(&field_module);

    return field_image;
}

// Can't do multiple tests with this setup, one should be fine.
TEST(DICOM, Analyze)
{
    std::string name = "/A820BF0A.dcm";
    std::string imageName = std::string(DICOMIMAGE_IMAGEDIR) + name;
    cap::DICOMImage *di = new cap::DICOMImage(imageName);

    Cmiss_context_id context = Cmiss_context_create("test");

    Cmiss_field_image_id field = Cmiss_create_image_field(context, imageName);
    bool success = di->Analyze(field);
    EXPECT_TRUE(success);

    delete di;
    Cmiss_field_image_destroy(&field);
    Cmiss_context_destroy(&context);
}

TEST(DICOM, AssignTagValue)
{
    std::string name = "/A820BF0A.dcm";
    std::string imageName = std::string(DICOMIMAGE_IMAGEDIR) + name;
    cap::DICOMImage *di = new cap::DICOMImage(imageName);

    di->AssignTagValue("s", "y");
    delete di;
}

TEST(DICOM, IsTagRequired)
{
    std::string name = "/A820BF0A.dcm";
    std::string imageName = std::string(DICOMIMAGE_IMAGEDIR) + name;
    cap::DICOMImage *di = new cap::DICOMImage(imageName);

    EXPECT_FALSE(di->IsTagRequired("dcm:Don'tKnow"));
    EXPECT_TRUE(di->IsTagRequired("dcm:Columns"));
    EXPECT_FALSE(di->IsTagRequired("dcm:SequenceName"));

    delete di;
}

TEST(DICOM, GetDefaultTagValue)
{
    std::string name = "/A820BF0A.dcm";
    std::string imageName = std::string(DICOMIMAGE_IMAGEDIR) + name;
    cap::DICOMImage *di = new cap::DICOMImage(imageName);

    std::string value;
    value = di->GetDefaultTagValue("dcm:Don'tKnow");
    EXPECT_EQ("N/A", value);
    value = di->GetDefaultTagValue("dcm:SequenceName");
    EXPECT_EQ("", value);
    value = di->GetDefaultTagValue("dcm:Patient'sName");
    EXPECT_EQ("N/A", value);
    value = di->GetDefaultTagValue("dcm:InstanceNumber");
    EXPECT_EQ("-1", value);

    delete di;
}

