
#include <gtest/gtest.h>

#include "utils/time.h"


TEST(UtilsTimeTest, TimeNow)
{
	std::cout << TimeNow() << std::endl;
	EXPECT_NE("Error in NowTime()", TimeNow());
}

