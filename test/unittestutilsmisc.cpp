
#include <gtest/gtest.h>

#include "utils/misc.h"

TEST(UtilsMisc, TimeNow)
{
	std::string time = cap::TimeNow();
    EXPECT_EQ(11, time.size());
	EXPECT_NE("Error in NowTime()", cap::TimeNow());
}

TEST(UtilsMisc, EndsWith)
{
    EXPECT_TRUE(cap::EndsWith("yabbadabbadoo", "doo"));
    EXPECT_TRUE(cap::EndsWith("filename.exnode", ".exnode"));
    EXPECT_FALSE(cap::EndsWith("notgoingtomatch", "oranges"));
}

TEST(UtilsMisc, ToString)
{
	double pi = 3.14159276756;
	int myint = 6;

	EXPECT_STREQ("3.14159", cap::ToString(pi).c_str());
	EXPECT_STREQ("6", cap::ToString(myint).c_str());
}

TEST(UtilsMisc, FromString)
{
	std::string myint = "6 ";
	std::string delimint = "6 3   5 78 97  ";
	std::string delimdouble = "0.7760464234567890124567890\\0.63067579\\1.4428528e-008\\-0.28337461\\0.34869241\\-0.89337138 ";

	EXPECT_EQ(6, cap::FromString<int>(myint));
	std::vector<int> values = cap::FromString<int>(delimint, ' ');
	ASSERT_EQ(5, values.size());
	EXPECT_EQ(6, values[0]);
	EXPECT_EQ(3, values[1]);
	EXPECT_EQ(5, values[2]);
	EXPECT_EQ(78, values[3]);
	EXPECT_EQ(97, values[4]);
	std::vector<double> values_dbl = cap::FromString<double>(delimdouble, '\\');
	ASSERT_EQ(6, values_dbl.size());
	EXPECT_EQ(0.7760464234567890124567890, values_dbl[0]);
	EXPECT_EQ(0.63067579, values_dbl[1]);
	EXPECT_EQ(1.4428528e-008, values_dbl[2]);
	EXPECT_EQ(-0.28337461, values_dbl[3]);
	EXPECT_EQ(0.34869241, values_dbl[4]);
	EXPECT_EQ(-0.89337138, values_dbl[5]);
}


