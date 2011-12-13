

#include <gtest/gtest.h>

#include "unittestconfigure.h"
#include "dicomimage.h"
#include "math/algebra.h"

TEST(DICOMImageTest, GetFilename)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);
	ASSERT_STREQ(filename.c_str(), di.GetFilename().c_str());
}

TEST(DICOMImageTest, GetImageOrientation)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);
	std::pair<cap::Vector3D,cap::Vector3D> orient = di.GetImageOrientation();

	EXPECT_NEAR(0.77604642, orient.first.x, 1e-07);
	EXPECT_NEAR(0.63067579, orient.first.y, 1e-07);
	EXPECT_NEAR(0.0, orient.first.z, 1e-07);
	EXPECT_NEAR(-0.28337461, orient.second.x, 1e-07);
	EXPECT_NEAR(0.34869241, orient.second.y, 1e-07);
	EXPECT_NEAR(-0.89337138, orient.second.z, 1e-07);
}

TEST(DICOMImageTest, GetImagePosition)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);

	cap::Point3D pt = di.GetImagePosition();
	EXPECT_NEAR(7.2672908, pt.x, 1e-07);
	EXPECT_NEAR(-207.37742, pt.y, 1e-07);
	EXPECT_NEAR(125.92049, pt.z, 1e-07);
}

TEST(DICOMImageTest, GetSize)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);
	
	double pixSizeX = di.GetPixelSizeX();
	double pixSizeY = di.GetPixelSizeX();
	EXPECT_NEAR(1.328125, pixSizeX, 1e-07);
	EXPECT_NEAR(1.328125, pixSizeY, 1e-07);
	double imWiPx = di.GetImageWidthPx();
	double imHePx = di.GetImageHeightPx();
	EXPECT_NEAR(208, imWiPx, 1e-07);
	EXPECT_NEAR(256, imHePx, 1e-07);

	EXPECT_NEAR(276.25, di.GetImageWidthMm(), 1e-07);
	EXPECT_NEAR(340.0, di.GetImageHeightMm(), 1e-07);
	//EXPECT_NEAR(pixSizeX*imWiPx, di.GetImageWidthMm(), 1e-07);
	//EXPECT_NEAR(pixSizeY*imHePx, di.GetImageHeightMm(), 1e-07);
}

TEST(DICOMImageTest, GetDetails)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);

	EXPECT_STREQ("042Y", di.GetAge().c_str());
	EXPECT_STREQ("19620112", di.GetDateOfBirth().c_str());
	EXPECT_STREQ("M ", di.GetGender().c_str());
	EXPECT_EQ(11, di.GetInstanceNumber());
	EXPECT_STREQ("20080926102825", di.GetPatientID().c_str());
	EXPECT_STREQ("Anonymized-AA.", di.GetPatientName().c_str());
	EXPECT_STREQ("20040313", di.GetScanDate().c_str());
	EXPECT_STREQ("*tfi2d1_29", di.GetSequenceName().c_str());
	EXPECT_STREQ("Anonymized with DicomWorks", di.GetSeriesDescription().c_str());
	EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6868385327594041", di.GetSeriesInstanceUID().c_str());
	EXPECT_EQ(44, di.GetSeriesNumber());
	EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6868460923122952", di.GetSopInstanceUID().c_str());
	EXPECT_STREQ("1.3.12.2.1107.5.2.6.22043.4.0.6860603417842558", di.GetStudyInstanceUID().c_str());
	EXPECT_NEAR(167.5, di.GetTriggerTime(), 1e-07);
}

TEST(DICOMImageTest, GetContours)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);
}

TEST(DICOMImageTest, AddContour)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);
}

TEST(DICOMImageTest, ImagePlane)
{
	std::string base(DICOMIMAGE_IMAGEDIR);
	std::string filename = base + "/68691116.dcm";
	cap::DICOMImage di(filename);

	cap::ImagePlane *plane = di.GetImagePlane();
	std::cout << plane->tlc << std::endl;
	std::cout << plane->xside << std::endl;
	std::cout << plane->yside << std::endl;
}


