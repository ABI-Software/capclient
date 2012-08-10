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

#include "io/imagesource.h"
#include "zinc/utilities.h"
#include "utils/filesystem.h"
#include "logmsg.h"

namespace cap
{
	Log::~Log() {}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;
}

TEST(FieldImage, CreateFile)
{
	cap::ArchiveHandler ah;
	ah.SetArchive(_T(SAMPLEARCHIVE_FILE));
	cap::ArchiveEntry ae = ah.GetEntry("68693672.dcm");
	std::string filename = cap::CreateTemporaryEmptyFile(FILESYSTEM_TESTDIR);
	bool res = cap::WriteCharBufferToFile(filename, ae.buffer_, ae.bufferSize_);
	EXPECT_EQ(true, res);
//	std::string name = "/68693672.dcm";
//	cap::ImageSource is(std::string(DICOMIMAGE_IMAGEDIR) + name);

	cap::ImageSource is(filename);

	Cmiss_context_id context = Cmiss_context_create("testz");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(root_region);
	Cmiss_field_image_id field_image = CreateImageTexture(field_module, is);
	EXPECT_NE(static_cast<Cmiss_field_image_id>(0), field_image);


	cap::RemoveFile(filename);
	Cmiss_field_image_destroy(&field_image);
	Cmiss_region_destroy(&root_region);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_context_destroy(&context);
}

TEST(FieldImage, CreateArchive)
{
	cap::ArchiveHandler ah;
	ah.SetArchive(_T(SAMPLEARCHIVE_FILE));
	cap::ArchiveEntry ae = ah.GetEntry("68693672.dcm");
	cap::ImageSource is(_T(SAMPLEARCHIVE_FILE), ae);

	Cmiss_context_id context = Cmiss_context_create("testx");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(root_region);
	Cmiss_field_image_id field_image = CreateImageTexture(field_module, is);
	EXPECT_NE(static_cast<Cmiss_field_image_id>(0), field_image);


	Cmiss_field_image_destroy(&field_image);
	Cmiss_region_destroy(&root_region);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_context_destroy(&context);
}

// Can't do multiple tests with this setup, one should be fine.
TEST(FieldImage, GetProperties)
{
	std::string name = "/68691116.dcm";
	cap::ImageSource is(std::string(DICOMIMAGE_IMAGEDIR) + name);

	Cmiss_context_id context = Cmiss_context_create("testy");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(root_region);
	Cmiss_field_image_id field_image = CreateImageTexture(field_module, is);
	EXPECT_NE(static_cast<Cmiss_field_image_id>(0), field_image);

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
		EXPECT_STREQ("44", value.c_str());
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


