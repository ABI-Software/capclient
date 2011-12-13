
#include <gtest/gtest.h>

#include "utils/misc.h"


TEST(UtilsTimeTest, TimeNow)
{
	std::cout << cap::TimeNow() << std::endl;
	EXPECT_NE("Error in NowTime()", cap::TimeNow());
}

