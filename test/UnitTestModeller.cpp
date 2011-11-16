/*
 * UnitTestCAPModeller.cpp
 *
 *  Created on: Oct 19, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#include "model/modeller.h"
#include "model/heart.h"

TEST(CAPModellerTest, ModellingModeApex)
{
	using namespace cap;
	ModellingModeApex m;
	m.PerformEntryAction();
}


TEST(CAPTimeSmootherTest, Create)
{
	using namespace cap;
	TimeSmoother ts;
}

#ifdef _MSC_VER
TEST(SecureStringScanTest, ScanLine)
{
	char line[] = "RUA                       11            11            11";
	char Type[4];
	int Nrow, Ncol, Nnzero, Neltvl = 0;
	// Testing this will fail...
	//int result = sscanf_s(line, "%c%d%d%d%d", Type, sizeof(Type), &Nrow, &Ncol, &Nnzero, &Neltvl); 
	// Must test this to pass
	int result = sscanf_s(line, "%s%d%d%d%d", Type, sizeof(Type), &Nrow, &Ncol, &Nnzero, &Neltvl);
	Type[3] = '\0';

	EXPECT_EQ(4, result);
	EXPECT_STREQ(Type, "RUA");
	EXPECT_EQ(Nrow, 11);
	EXPECT_EQ(Ncol, 11);
	EXPECT_EQ(Nnzero, 11);
	EXPECT_EQ(Neltvl, 0);
	//std::cout << Type << std::endl;
	//std::cout << Nrow << " " << Ncol << " " << Nnzero << " " << Neltvl << std::endl;
}
#endif

TEST(StringScanTest, ScanLine)
{
	char line[] = "RUA                       11            11            11";
	char Type[4];
	int Nrow, Ncol, Nnzero, Neltvl = 0;
	int result = sscanf(line, "%3c%d%d%d%d", Type, &Nrow, &Ncol, &Nnzero, &Neltvl);
	Type[3] = '\0';

	EXPECT_EQ(4, result);
	EXPECT_STREQ(Type, "RUA");
	EXPECT_EQ(Nrow, 11);
	EXPECT_EQ(Ncol, 11);
	EXPECT_EQ(Nnzero, 11);
	EXPECT_EQ(Neltvl, 0);
}

TEST(Modeller, AddApexPoint)
{
	using namespace cap;
	Modeller modeller(0);
	Point3D coord(4, 5, 6);
	modeller.AddDataPoint(0, coord, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
}

