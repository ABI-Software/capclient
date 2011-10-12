
#include <gtest/gtest.h>
#include "unittestconfigure.h"
#include "FileSystem.h"


#ifdef _MSC_VER
# define rmdir _rmdir
	char testString1[] = "\\home\\dummies\\are\\us";
	char testString2[] = "\\home\\dummies\\are\\ferry.txt";
	char testString3[] = "ferry.txt";
	char testString4[] = ".txt";
	char testString5[] = "\\never\\work.today\\ok";
#else
	char testString1[] = "/home/dummies/are/us";
	char testString2[] = "/home/dummies/are/ferry.txt";
	char testString3[] = "ferry.txt";
	char testString4[] = ".txt";
	char testString5[] = "/never/work.today/ok";
#endif

TEST(FileSystemTest, GetFileName)
{
	std::string result;
	result = cap::FileSystem::GetFileName(testString1);
	ASSERT_EQ(std::string("us"), result);
	result = cap::FileSystem::GetFileName(testString2);
	ASSERT_EQ(std::string("ferry.txt"), result);
	result = cap::FileSystem::GetFileName(testString3);
	ASSERT_EQ(std::string("ferry.txt"), result);
	result = cap::FileSystem::GetFileName(testString4);
	ASSERT_EQ(std::string(".txt"), result);
	
}

TEST(FileSystemTest, GetFileNameWOE)
{
	std::string result;
	result = cap::FileSystem::GetFileNameWOE(testString1);
	ASSERT_EQ(std::string("us"), result);
	result = cap::FileSystem::GetFileNameWOE(testString2);
	ASSERT_EQ(std::string("ferry"), result);
	result = cap::FileSystem::GetFileNameWOE(testString3);
	ASSERT_EQ(std::string("ferry"), result);
	result = cap::FileSystem::GetFileNameWOE(testString4);
	ASSERT_EQ(std::string(""), result);
	result = cap::FileSystem::GetFileNameWOE(testString5);
	ASSERT_EQ(std::string("ok"), result);
	
}

TEST(FileSystemTest, GetAllFileNames)
{
	cap::FileSystem fs(FILESYSTEM_TESTDIR);
	std::vector<std::string> names = fs.getAllFileNames();

	std::vector<std::string>::const_iterator it = names.begin();
	ASSERT_EQ(std::string("file1.txt"), names.at(0));
	ASSERT_EQ(std::string("file2.txt"), names.at(1));
	ASSERT_EQ(std::string("file3.txt"), names.at(2));
	ASSERT_EQ(std::string("subdir"), names.at(3));
}

TEST(FileSystemTest, CreateDirectory)
{
	cap::FileSystem fs(FILESYSTEM_TESTDIR);
	bool res = fs.CreateDirectory("created/programmatically");
	EXPECT_EQ(false, res);
	res = fs.CreateDirectory("created magic");
	EXPECT_EQ(true, res);
	res = fs.CreateDirectory("mine");
	EXPECT_EQ(true, res);

	// remove created directories
	std::string base1(FILESYSTEM_TESTDIR);
	int ret = rmdir(base1.append("/created magic").c_str());
	EXPECT_EQ(0, ret);
	std::string base2(FILESYSTEM_TESTDIR);
	ret = rmdir(base2.append("/mine").c_str());
	EXPECT_EQ(0, ret);
}

