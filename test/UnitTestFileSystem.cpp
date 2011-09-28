#include <gtest/gtest.h>
#include "FileSystem.h"

TEST(FileSystemTest, GetFileName)
{
	std::string result;
	result = cap::FileSystem::GetFileName("/home/dummies/are/us");
	ASSERT_EQ(std::string("us"), result);
	result = cap::FileSystem::GetFileName("/home/dummies/are/ferry.txt");
	ASSERT_EQ(std::string("ferry.txt"), result);
	result = cap::FileSystem::GetFileName("ferry.txt");
	ASSERT_EQ(std::string("ferry.txt"), result);
	result = cap::FileSystem::GetFileName(".txt");
	ASSERT_EQ(std::string(".txt"), result);
	
}

TEST(FileSystemTest, GetFileNameWOE)
{
	std::string result;
	result = cap::FileSystem::GetFileNameWOE("/home/dummies/are/us");
	ASSERT_EQ(std::string("us"), result);
	result = cap::FileSystem::GetFileNameWOE("/home/dummies/are/ferry.txt");
	ASSERT_EQ(std::string("ferry"), result);
	result = cap::FileSystem::GetFileNameWOE("ferry.txt");
	ASSERT_EQ(std::string("ferry"), result);
	result = cap::FileSystem::GetFileNameWOE(".txt");
	ASSERT_EQ(std::string(""), result);
	result = cap::FileSystem::GetFileNameWOE("/never/work.today/ok");
	ASSERT_EQ(std::string("ok"), result);
	
}

