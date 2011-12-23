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

Cmiss_field_image_id Cmiss_field_module_create_image_texture(Cmiss_field_module_id field_module, const std::string& filename)
{
	//std::cout << "SceneViewerPanel::" << __func__ << std::endl;
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
	
	return field_image;
}

// Can't do multiple tests with this setup, one should be fine.
TEST(FieldImage, GetProperties)
{
	std::string name = "/68691116.dcm";
	std::string imageName = std::string(DICOMIMAGE_IMAGEDIR) + name;

	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(root_region);
	Cmiss_field_image_id field_image = Cmiss_field_module_create_image_texture(field_module, imageName);

	std::string value;
	char *prop = 0;

	prop = Cmiss_field_image_get_property(field_image, "dcm:SeriesInstanceUID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6868385327594041", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:SOPInstanceUID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6868460923122952", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:StudyInstanceUID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6860603417842558", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sAge");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("042Y", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sBirthDate");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("19620112", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:PatientID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("20080926102825", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sName");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("Anonymized-AA.", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:AcquisitionDate");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("20040313", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:SeriesDescription");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("Anonymized with DicomWorks", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sSex");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("M ", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:InstanceNumber");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("11", value.c_str());
	}

	prop = Cmiss_field_image_get_property(field_image, "dcm:SequenceName");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("*tfi2d1_29", value.c_str());
	}

	Cmiss_field_image_destroy(&field_image);
	Cmiss_region_destroy(&root_region);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_context_destroy(&context);
}

