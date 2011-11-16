
#include <gtest/gtest.h>
#include <algorithm>
#include "unittestconfigure.h"
#include "utils/filesystem.h"
#include "hexified/globalsmoothtvmatrix.dat.h"

char testString1[] = "/home/dummies/are/us";
char testString2[] = "/home/dummies/are/ferry.txt";
char testString3[] = "ferry.txt";
char testString4[] = ".txt";
char testString5[] = "/never/work.today/ok";
char testString6[] = "/home/is\\where the heart is.juno";
char testString7[] = "D:\\work\\cardiacatlas\\capclient\\build\\output.xml";

TEST(FileSystemTest, GetFileName)
{
	std::string result;
	result = cap::FileSystem::GetFileName(testString1);
	EXPECT_EQ(std::string("us"), result);
	result = cap::FileSystem::GetFileName(testString2);
	EXPECT_EQ(std::string("ferry.txt"), result);
	result = cap::FileSystem::GetFileName(testString3);
	EXPECT_EQ(std::string("ferry.txt"), result);
	result = cap::FileSystem::GetFileName(testString4);
	EXPECT_EQ(std::string(".txt"), result);
	result = cap::FileSystem::GetFileName(testString5);
	EXPECT_EQ(std::string("ok"), result);
	result = cap::FileSystem::GetFileName(testString6);
	EXPECT_EQ(std::string("where the heart is.juno"), result);
	result = cap::FileSystem::GetFileName(testString7);
	EXPECT_EQ(std::string("output.xml"), result);
	
}

TEST(FileSystemTest, GetFileNameWOE)
{
	std::string result;
	result = cap::FileSystem::GetFileNameWOE(testString1);
	EXPECT_EQ(std::string("us"), result);
	result = cap::FileSystem::GetFileNameWOE(testString2);
	EXPECT_EQ(std::string("ferry"), result);
	result = cap::FileSystem::GetFileNameWOE(testString3);
	EXPECT_EQ(std::string("ferry"), result);
	result = cap::FileSystem::GetFileNameWOE(testString4);
	EXPECT_EQ(std::string(""), result);
	result = cap::FileSystem::GetFileNameWOE(testString5);
	EXPECT_EQ(std::string("ok"), result);
	result = cap::FileSystem::GetFileNameWOE(testString6);
	EXPECT_EQ(std::string("where the heart is"), result);
	result = cap::FileSystem::GetFileNameWOE(testString7);
	EXPECT_EQ(std::string("output"), result);
}

TEST(FileSystemTest, GetAllFileNames)
{
	std::vector<std::string> names = cap::FileSystem::GetAllFileNames(FILESYSTEM_TESTDIR);

	std::sort(names.begin(), names.end());
	EXPECT_EQ(3, names.size());
	EXPECT_EQ(std::string("file1.txt"), names.at(0));
	EXPECT_EQ(std::string("file2.txt"), names.at(1));
	EXPECT_EQ(std::string("file3.txt"), names.at(2));
}

TEST(FileSystemTest, MakeDirectory)
{
	using namespace cap;
	bool res = cap::FileSystem::MakeDirectory(std::string(FILESYSTEM_TESTDIR) + "/created/programmatically");
	EXPECT_EQ(false, res);
	res = cap::FileSystem::MakeDirectory(std::string(FILESYSTEM_TESTDIR) + "/created magic");
	EXPECT_EQ(true, res);
	res = cap::FileSystem::MakeDirectory(std::string(FILESYSTEM_TESTDIR) + "/mine");
	EXPECT_EQ(true, res);

	// remove created directories
	std::string base(FILESYSTEM_TESTDIR);
	res = FileSystem::DeleteDirectory(base + "/created magic");
	EXPECT_EQ(true, res);
	res = FileSystem::DeleteDirectory(base + "/mine");
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, TempFile1)
{
	using namespace cap;
	std::string filename = FileSystem::CreateTemporaryEmptyFile();
	bool exists = FileSystem::IsFile(filename);
	ASSERT_EQ(true, exists) << "filename doesn't exist : '" << filename << "'";
	bool res = FileSystem::RemoveFile(filename);
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, TempFile2)
{
	using namespace cap;
	std::string filename = FileSystem::CreateTemporaryEmptyFile(std::string(FILESYSTEM_TESTDIR));
	bool exists = FileSystem::IsFile(filename);
	ASSERT_EQ(true, exists) << "filename doesn't exist : '" << filename << "'";
	std::string justfilename = FileSystem::GetFileName(filename);
	std::string abslocation = std::string(FILESYSTEM_TESTDIR) + '/' + justfilename;
	EXPECT_EQ(abslocation, filename);
	bool res = FileSystem::RemoveFile(filename);
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, WriteBufferFile)
{
	using namespace cap;
	std::string filename = FileSystem::CreateTemporaryEmptyFile(std::string(FILESYSTEM_TESTDIR));
	bool res = FileSystem::WriteCharBufferToFile(filename, globalsmoothtvmatrix_dat, globalsmoothtvmatrix_dat_len);
	EXPECT_EQ(true, res);
	res = FileSystem::RemoveFile(filename);
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, WriteBufferString)
{
	using namespace cap;
	unsigned char data[] = {
		0x30, 0x2e, 0x39, 0x32, 0x34, 0x30, 0x33, 0x31, 0x20, 0x30, 0x2e, 0x38, 0x35, 0x35, 0x35, 0x37, 0x39, 0x20, 0x30, 0x2e, 0x39, 0x30, 0x35, 0x34, 0x36, 0x30};
	unsigned int len = 26;
	std::string res = FileSystem::WriteCharBufferToString(data, len);
	EXPECT_EQ("0.924031 0.855579 0.905460", res);
}

