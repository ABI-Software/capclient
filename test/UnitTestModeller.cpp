/*
 * UnitTestCAPModeller.cpp
 *
 *  Created on: Oct 19, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>

extern "C"
{
#include <configure/cmgui_configure.h>
#include <api/cmiss_context.h>
}

#include "cmgui/extensions.h"
#include "model/modeller.h"
#include "model/heart.h"
#include "modellercapclientwindow.h"
#include "modellercapclient.h"

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

TEST(Modeller, AddApexPoints)
{
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_node_id node1 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node2 != 0);
	using namespace cap;
	Modeller modeller(0);
	Point3D coord1(4, 5, 6);
	Point3D coord2(9, 8, 7);
	modeller.AddDataPoint(node1, coord1, 0.0);
	modeller.AddDataPoint(node2, coord2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::BASE, modeller.GetCurrentMode());
	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AddBasePoints)
{
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_node_id node1 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node3 != 0);

	using namespace cap;
	Modeller modeller(0);
	Point3D coord1(4, 5, 6);
	modeller.AddDataPoint(node1, coord1, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	Point3D coord2(1, 5, 3);
	Point3D coord3(9, 8, 7);
	modeller.AddDataPoint(node2, coord2, 0.0);
	modeller.AddDataPoint(node3, coord3, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::RV, modeller.GetCurrentMode());

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AddRVPoints)
{
	using namespace cap;
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");

	Cmiss_node_id node1 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node4 != 0);

	Modeller modeller(0);
	Point3D coord1(4, 5, 6);
	modeller.AddDataPoint(node1, coord1, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	Point3D coord2(1, 5, 3);
	modeller.AddDataPoint(node2, coord2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	Point3D coord3(9, 8, 7);
	modeller.AddDataPoint(node3, coord3, 0.0);
	EXPECT_FALSE(modeller.OnAccept());
	EXPECT_EQ(Modeller::RV, modeller.GetCurrentMode());
	Point3D coord4(9, 10, 7);
	modeller.AddDataPoint(node4, coord4, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::BASEPLANE, modeller.GetCurrentMode());

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AddBasePlanePoints)
{
	using namespace cap;
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	CAPClient mcc;
	mcc.gui_->CreateHeartModel();
	mcc.gui_->LoadTemplateHeartModel(25);

	Cmiss_node_id node1 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node4 != 0);
	Cmiss_node_id node5 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node5 != 0);
	Cmiss_node_id node6 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node6 != 0);

	Point3D apex(24.2506, -71.3943, -9.00449);
	Point3D base(-20.4335, -22.5206, 38.4314);
	Point3D rv1(30.286, -18.3221, 1.32952);
	Point3D rv2(-3.52178, -35.387, -20.3095);
	Point3D bp1(-18.4984, -52.6508, 43.958);
	Point3D bp2(-3.04825, -0.0334985, 8.7444);

	Modeller modeller(&mcc);
	modeller.AddDataPoint(node1, apex, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddDataPoint(node2, base, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddDataPoint(node3, rv1, 0.0);
	modeller.AddDataPoint(node4, rv2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::BASEPLANE, modeller.GetCurrentMode());
	modeller.AddDataPoint(node5, bp1, 0.25);
	modeller.AddDataPoint(node6, bp2, 0.25);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::GUIDEPOINT, modeller.GetCurrentMode());

	modeller.AlignModel();

	// x-axis
	EXPECT_NEAR(0.548551, mcc.m_[0][0], 0.0001);
	EXPECT_NEAR(-0.59998, mcc.m_[0][1], 0.0001);
	EXPECT_NEAR(-0.58233, mcc.m_[0][2], 0.0001);

	// Offset
	EXPECT_NEAR(-5.540289, mcc.m_[3][0], 0.0001);
	EXPECT_NEAR(-38.81020, mcc.m_[3][1], 0.0001);
	EXPECT_NEAR(22.621017, mcc.m_[3][2], 0.0001);

	// focal length
	EXPECT_NEAR(35.1947, mcc.fl_, 0.0001);

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_node_destroy(&node5);
	Cmiss_node_destroy(&node6);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, BasePlanePointsDifferentTimes)
{
	using namespace cap;
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_node_id node1 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node4 != 0);
	Point3D bp1(-18.4984, -52.6508, 43.958);
	Point3D bp2(-3.04825, -0.0334985, 8.7444);

	CAPClient mcc;
	Modeller modeller(&mcc);
	modeller.ChangeMode(Modeller::BASEPLANE);
	modeller.AddDataPoint(node1, bp1, 0.2);
	modeller.AddDataPoint(node2, bp2, 0.2);
	modeller.AddDataPoint(node3, bp1, 0.4);
	modeller.AddDataPoint(node4, bp2, 0.42);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::GUIDEPOINT, modeller.GetCurrentMode());


	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AlignModel)
{
	using namespace cap;
	//Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	CAPClient mcc;
	mcc.gui_->CreateHeartModel();
	mcc.gui_->LoadTemplateHeartModel(25);

	Cmiss_node_id node1 = Cmiss_context_create_node(mcc.gui_->cmissContext_, 24.2506, -71.3943, -9.00449);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -20.4335, -22.5206, 38.4314);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(mcc.gui_->cmissContext_, 30.286, -18.3221, 1.32952);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -3.52178, -35.387, -20.3095);
	EXPECT_TRUE(node4 != 0);
	Cmiss_node_id node5 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -18.4984, -52.6508, 43.958);
	EXPECT_TRUE(node5 != 0);
	Cmiss_node_id node6 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -3.04825, -0.0334985, 8.7444);
	EXPECT_TRUE(node6 != 0);

	Point3D apex(24.2506, -71.3943, -9.00449);
	Point3D base(-20.4335, -22.5206, 38.4314);
	Point3D rv1(30.286, -18.3221, 1.32952);
	Point3D rv2(-3.52178, -35.387, -20.3095);
	Point3D bp1(-18.4984, -52.6508, 43.958);
	Point3D bp2(-3.04825, -0.0334985, 8.7444);

	Modeller modeller(&mcc);
	modeller.AddDataPoint(node1, apex, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddDataPoint(node2, base, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddDataPoint(node3, rv1, 0.0);
	modeller.AddDataPoint(node4, rv2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::BASEPLANE, modeller.GetCurrentMode());
	modeller.AddDataPoint(node5, bp1, 0.5);
	modeller.AddDataPoint(node6, bp2, 0.5);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(Modeller::GUIDEPOINT, modeller.GetCurrentMode());

	modeller.AlignModel();

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_node_destroy(&node5);
	Cmiss_node_destroy(&node6);
	//Cmiss_context_destroy(&context);
}

