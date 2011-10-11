#include <gtest/gtest.h>
#include "FileSystem.h"

#ifdef _MSC_VER
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
	cap::FileSystem fs(".");
	std::vector<std::string> names = fs.getAllFileNames();

	std::vector<std::string>::const_iterator it = names.begin();
	for (;it != names.end(); it++)
		std::cout << *it << std::endl;
}

