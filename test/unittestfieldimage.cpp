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

	// Series Instance UID (0x0020,0x000e)
	prop = Cmiss_field_image_get_property(field_image, "dcm:SeriesInstanceUID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6868385327594041", value.c_str());
	}

	// SOP instance uid (0x0008,0x0018)
	prop = Cmiss_field_image_get_property(field_image, "dcm:SOPInstanceUID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6868460923122952", value.c_str());
	}

	// Study Instance uid (0x0020,0x000d)
	prop = Cmiss_field_image_get_property(field_image, "dcm:StudyInstanceUID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6860603417842558", value.c_str());
	}

	// Series Description (0x0008,0x103e)
	prop = Cmiss_field_image_get_property(field_image, "dcm:SeriesDescription");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("Anonymized with DicomWorks", value.c_str());
	}

	// Series Number (0x0020,0x0011)
	prop = Cmiss_field_image_get_property(field_image, "dcm:SeriesNumber");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("11", value.c_str());
	}

	// Acquisition Date (0x0008,0x0022)
	prop = Cmiss_field_image_get_property(field_image, "dcm:AcquisitionDate");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("20040313", value.c_str());
	}

	// Instance Number (0x0020,0x0013)
	prop = Cmiss_field_image_get_property(field_image, "dcm:InstanceNumber");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("11", value.c_str());
	}

	// Patient's Age (0x0010,0x1010)
	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sAge");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("042Y", value.c_str());
	}

	// Patient's Birth Date (0x0010,0x0030)
	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sBirthDate");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("19620112", value.c_str());
	}

	// Patient ID (0x0010,0x0020)
	prop = Cmiss_field_image_get_property(field_image, "dcm:PatientID");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("20080926102825", value.c_str());
	}

	// Patient's Name (0x0010,0x0010)
	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sName");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("Anonymized-AA.", value.c_str());
	}

	// Patient's Sex (0x0010,0x0040)
	prop = Cmiss_field_image_get_property(field_image, "dcm:Patient'sSex");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("M ", value.c_str());
	}

	// Image Position (0x0020,0x0032)
	prop = Cmiss_field_image_get_property(field_image, "dcm:ImagePosition(Patient)");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("7.2672908\\-207.37742\\125.92049", value.c_str());
	}

	// Image Position (0x0020,0x0030)(old)
	prop = Cmiss_field_image_get_property(field_image, "dcm:ImagePosition");
	EXPECT_TRUE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("7.2672908\\-207.37742\\125.92049", value.c_str());
	}

	// Image Orientation (0x0020,0x0037)
	prop = Cmiss_field_image_get_property(field_image, "dcm:ImageOrientation(Patient)");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("0.77604642\\0.63067579\\1.4428528e-008\\-0.28337461\\0.34869241\\-0.89337138 ", value.c_str());
	}

	// Image Orientation (0x0020,0x0035)(old)
	prop = Cmiss_field_image_get_property(field_image, "dcm:ImageOrientation");
	EXPECT_TRUE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("0.77604642\\0.63067579\\1.4428528e-008\\-0.28337461\\0.34869241\\-0.89337138 ", value.c_str());
	}

	// Rows (0x0028,0x0010)
	prop = Cmiss_field_image_get_property(field_image, "dcm:Rows");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("256", value.c_str());
	}

	// Columns (0x0028,0x0011)
	prop = Cmiss_field_image_get_property(field_image, "dcm:Columns");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("208", value.c_str());
	}

	// Pixel Spacing (0x0028,0x0030)
	prop = Cmiss_field_image_get_property(field_image, "dcm:PixelSpacing");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("1.328125\\1.328125 ", value.c_str());
	}

	// Slice Thickness (0x0018,0x0050)
	prop = Cmiss_field_image_get_property(field_image, "dcm:SliceThickness");
	EXPECT_FALSE(prop == 0);
	if (prop)
	{
		value = prop;
		Cmiss_deallocate(prop);
		EXPECT_STREQ("6 ", value.c_str());
	}

	Cmiss_field_image_destroy(&field_image);
	Cmiss_region_destroy(&root_region);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_context_destroy(&context);
}


