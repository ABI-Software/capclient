
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

std::vector<std::string> FindIntersection(std::vector<std::string> one, std::vector<std::string> two)
{
	std::vector<std::string> intersection;

	std::vector<std::string>::const_iterator cit1 = one.begin();
	while (cit1 != one.end())
	{
		bool in = false;
		std::vector<std::string>::const_iterator cit2 = two.begin();
		while (cit2 != two.end())
		{
			if (*cit1 == *cit2)
				in = true;
			++cit2;
		}
		if (in)
			intersection.push_back(*cit1);
		++cit1;
	}

	return intersection;
}

TEST(UtilsMisc, SamePlane)
{
	std::vector<std::string> labels1;
	labels1.push_back("LA1");
	labels1.push_back("SA1");
	std::vector<std::string> labels2;
	labels2.push_back("LA1");
	labels2.push_back("SA2");
	std::vector<std::string> labels3;
	labels3.push_back("LA6");
	labels3.push_back("LA1");

	std::vector<std::string> intersection = FindIntersection(labels1, labels2);
	EXPECT_EQ(1, intersection.size());
	EXPECT_EQ("LA1", intersection.at(0));
	intersection = FindIntersection(intersection, labels3);
	EXPECT_EQ(1, intersection.size());
	EXPECT_EQ("LA1", intersection.at(0));
}

#include <algorithm>

TEST(UtilsMisc, DifferentPlane)
{
	std::vector<std::string> labels1;
	labels1.push_back("LA1");
	labels1.push_back("SA1");
	std::vector<std::string> labels2;
	labels2.push_back("LA1");
	labels2.push_back("SA2");
	std::vector<std::string> labels3;
	labels3.push_back("LA6");
	labels3.push_back("SA1");

	std::vector<std::string> current_string_intersection = labels2;
	std::vector<std::string> string_intersection;
	std::set_intersection(labels1.begin(), labels1.end(), current_string_intersection.begin(), current_string_intersection.end(), std::back_inserter(string_intersection));
	current_string_intersection = string_intersection;
	EXPECT_EQ(1, string_intersection.size());
	EXPECT_EQ("LA1", string_intersection.at(0));
	std::vector<std::string> intersection = FindIntersection(labels1, labels2);
	EXPECT_EQ(1, intersection.size());
	EXPECT_EQ("LA1", intersection.at(0));
	intersection = FindIntersection(intersection, labels3);
	EXPECT_EQ(0, intersection.size());
}

TEST(UtilsMisc, FindString)
{
	std::string label = "LA1";
	std::vector<std::string> strings;
	strings.push_back("SA1");
	strings.push_back("SA2");
	strings.push_back("SA3");
	strings.push_back("LA1");
	strings.push_back("LA3");
	std::vector<std::string>::const_iterator cit = find(strings.begin(), strings.end(), label);
	EXPECT_TRUE(cit != strings.end());
	EXPECT_EQ("LA1", *cit);
}

TEST(UtilsMisc, DontFindString)
{
	std::string label = "LA6";
	std::vector<std::string> strings;
	strings.push_back("SA1");
	strings.push_back("SA2");
	strings.push_back("SA3");
	strings.push_back("LA1");
	strings.push_back("LA3");
	std::vector<std::string>::const_iterator cit = find(strings.begin(), strings.end(), label);
	EXPECT_TRUE(cit == strings.end());
}


