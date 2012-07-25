
#include <gtest/gtest.h>
#include <algorithm>

#include <wx/filefn.h>

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

#include "logmsg.h"


namespace cap
{
	std::string TimeNow() { return ""; }
	Log::~Log() {}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;
}

TEST(FileSystemTest, GetFileName)
{
	std::string result;
	result = cap::GetFileName(testString1);
	EXPECT_EQ(std::string("us"), result);
	result = cap::GetFileName(testString2);
	EXPECT_EQ(std::string("ferry.txt"), result);
	result = cap::GetFileName(testString3);
	EXPECT_EQ(std::string("ferry.txt"), result);
	result = cap::GetFileName(testString4);
	EXPECT_EQ(std::string(".txt"), result);
	result = cap::GetFileName(testString5);
	EXPECT_EQ(std::string("ok"), result);
	result = cap::GetFileName(testString6);
	EXPECT_EQ(std::string("where the heart is.juno"), result);
	result = cap::GetFileName(testString7);
	EXPECT_EQ(std::string("output.xml"), result);
	std::string curdir = wxGetCwd().c_str();
	result = cap::GetFileName(curdir);
	EXPECT_EQ(std::string(""), result);
	
}

TEST(FileSystemTest, GetFileNameWOE)
{
	std::string result;
	result = cap::GetFileNameWOE(testString1);
	EXPECT_EQ(std::string("us"), result);
	result = cap::GetFileNameWOE(testString2);
	EXPECT_EQ(std::string("ferry"), result);
	result = cap::GetFileNameWOE(testString3);
	EXPECT_EQ(std::string("ferry"), result);
	result = cap::GetFileNameWOE(testString4);
	EXPECT_EQ(std::string(""), result);
	result = cap::GetFileNameWOE(testString5);
	EXPECT_EQ(std::string("ok"), result);
	result = cap::GetFileNameWOE(testString6);
	EXPECT_EQ(std::string("where the heart is"), result);
	result = cap::GetFileNameWOE(testString7);
	EXPECT_EQ(std::string("output"), result);
}

TEST(FileSystemTest, GetAllFileNames)
{
	std::vector<std::string> names = cap::GetAllFileNames(FILESYSTEM_TESTDIR);

	std::sort(names.begin(), names.end());
	ASSERT_EQ(3, names.size());
	EXPECT_EQ(std::string("file1.txt"), names.at(0));
	EXPECT_EQ(std::string("file2.txt"), names.at(1));
	EXPECT_EQ(std::string("file3.txt"), names.at(2));
}

TEST(FileSystemTest, GetAllFileNamesRecursive)
{
	std::vector<std::string> names = cap::GetAllFileNamesRecursive(FILESYSTEM_TESTDIR);

	std::sort(names.begin(), names.end());
	ASSERT_EQ(5, names.size());
	EXPECT_EQ(std::string("file1.txt"), names.at(0));
	EXPECT_EQ(std::string("file2.txt"), names.at(1));
	EXPECT_EQ(std::string("file3.txt"), names.at(2));
	EXPECT_EQ(std::string("subdir/subfile1.txt"), names.at(3));
	EXPECT_EQ(std::string("subdir/subfile2.txt"), names.at(4));
}

TEST(FileSysteText, GetPath)
{
	std::string result;
	std::string curdir = wxGetCwd().c_str();
	result = cap::GetPath(curdir);
	EXPECT_EQ(curdir, result);
	result = cap::GetPath(curdir + "/this/bit/isnot/real");
	std::string dirpath = curdir;
	size_t found = dirpath.find_first_of('\\');
	while (found != std::string::npos)
	{
		dirpath[found] = '/';
		found = dirpath.find_first_of('\\', found+1);
	}

	EXPECT_EQ(dirpath, result);

}

TEST(FileSystemTest, MakeDirectory)
{
	using namespace cap;
	bool res = cap::MakeDirectory(std::string(FILESYSTEM_TESTDIR) + "/created/programmatically");
    EXPECT_FALSE(res);
	res = cap::MakeDirectory(std::string(FILESYSTEM_TESTDIR) + "/created magic");
    EXPECT_TRUE(res);
	res = cap::MakeDirectory(std::string(FILESYSTEM_TESTDIR) + "/mine");
    EXPECT_TRUE(res);

	// remove created directories
	std::string base(FILESYSTEM_TESTDIR);
	res = DeleteDirectory(base + "/created magic");
	EXPECT_EQ(true, res);
	res = DeleteDirectory(base + "/mine");
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, TempFile1)
{
	using namespace cap;
	std::string filename = CreateTemporaryEmptyFile();
	bool exists = IsFile(filename);
	ASSERT_EQ(true, exists) << "filename doesn't exist : '" << filename << "'";
	bool res = RemoveFile(filename);
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, TempFile2)
{
	using namespace cap;
	std::string filename = CreateTemporaryEmptyFile(std::string(FILESYSTEM_TESTDIR));
	bool exists = IsFile(filename);
	ASSERT_EQ(true, exists) << "filename doesn't exist : '" << filename << "'";
	std::string justfilename = GetFileName(filename);
	std::string abslocation = std::string(FILESYSTEM_TESTDIR) + '/' + justfilename;
	EXPECT_EQ(abslocation, filename);
	bool res = RemoveFile(filename);
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, WriteBufferFile)
{
	using namespace cap;
	std::string filename = CreateTemporaryEmptyFile(std::string(FILESYSTEM_TESTDIR));
	bool res = WriteCharBufferToFile(filename, globalsmoothtvmatrix_dat, globalsmoothtvmatrix_dat_len);
	EXPECT_EQ(true, res);
	res = RemoveFile(filename);
	EXPECT_EQ(true, res);
}

TEST(FileSystemTest, WriteBufferString)
{
	using namespace cap;
	unsigned char data[] = {
		0x30, 0x2e, 0x39, 0x32, 0x34, 0x30, 0x33, 0x31, 0x20, 0x30, 0x2e, 0x38, 0x35, 0x35, 0x35, 0x37, 0x39, 0x20, 0x30, 0x2e, 0x39, 0x30, 0x35, 0x34, 0x36, 0x30};
	unsigned int len = 26;
	std::string res = WriteCharBufferToString(data, len);
	EXPECT_EQ("0.924031 0.855579 0.905460", res);
}

