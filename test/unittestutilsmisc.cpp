
#include <gtest/gtest.h>

#include "utils/misc.h"

TEST(UtilsMisc, TimeNow)
{
	std::cout << cap::TimeNow() << std::endl;
	EXPECT_NE("Error in NowTime()", cap::TimeNow());
}

TEST(UtilsMisc, EndsWith)
{
	EXPECT_EQ(true, cap::EndsWith("yabbadabbadoo", "doo"));
	EXPECT_EQ(true, cap::EndsWith("filename.exnode", ".exnode"));
	EXPECT_EQ(false, cap::EndsWith("notgoingtomatch", "oranges"));
}

