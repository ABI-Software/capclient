
#include <gtest/gtest.h>

#include "io/imagesource.h"

TEST(ImageSource, SetGet)
{
	cap::ImageSource is("");
	EXPECT_EQ(cap::FILEONDISK, is.GetType());
}

TEST(ImageSource, Create)
{
	std::string filename, archiveLocation;
	cap::ArchiveEntry archiveEntry;
	cap::ImageSource is1(filename);
	EXPECT_EQ(cap::FILEONDISK, is1.GetType());
	cap::ImageSource is2(archiveLocation, archiveEntry);
	EXPECT_EQ(cap::ARCHIVEENTRY, is2.GetType());
}
