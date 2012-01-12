/*
 * UnitTestCAPModeller.cpp
 *
 *  Created on: Oct 19, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_context.h>
}

#include "cmgui/extensions.h"
#define private public
#include "model/modeller.h"
#undef private
#include "model/heart.h"
#include "modellercapclientwindow.h"
#include "modellercapclient.h"
#include "utils/debug.h"
#include "logmsg.h"


namespace cap
{
	std::string TimeNow() { return ""; }
	Log::~Log() {}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;
}

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

TEST(SecureStringScanTest, ScanLine2Strings)
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
	EXPECT_STREQ("RUA", Type);
	EXPECT_EQ(11, Nrow);
	EXPECT_EQ(11, Ncol);
	EXPECT_EQ(11, Nnzero);
	EXPECT_EQ(0, Neltvl);
}

TEST(Modeller, AddApexPoints)
{
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_region_id region = Cmiss_context_get_default_region(context);

	Cmiss_node_id node1 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node2 != 0);
	using namespace cap;
	Modeller modeller(0);
	Point3D coord1(4, 5, 6);
	Point3D coord2(9, 8, 7);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node1), coord1, 0.0);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node2), coord2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(BASE, modeller.GetCurrentMode());
	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_region_destroy(&region);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AddBasePoints)
{
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_region_id region = Cmiss_context_get_default_region(context);

	Cmiss_node_id node1 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context, 0.0, 0.0, 0.0);
	EXPECT_TRUE(node3 != 0);

	using namespace cap;
	Modeller modeller(0);
	Point3D coord1(4, 5, 6);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node1), coord1, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	Point3D coord2(1, 5, 3);
	Point3D coord3(9, 8, 7);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node2), coord2, 0.0);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node3), coord3, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(RV, modeller.GetCurrentMode());

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_region_destroy(&region);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AddRVPoints)
{
	using namespace cap;
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_region_id region = Cmiss_context_get_default_region(context);

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
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node1), coord1, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	Point3D coord2(1, 5, 3);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node2), coord2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	Point3D coord3(9, 8, 7);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node3), coord3, 0.0);
	EXPECT_FALSE(modeller.OnAccept());
	EXPECT_EQ(RV, modeller.GetCurrentMode());
	Point3D coord4(9, 10, 7);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node4), coord4, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(BASEPLANE, modeller.GetCurrentMode());

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_region_destroy(&region);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AddBasePlanePoints)
{
	using namespace cap;
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_region_id region = Cmiss_context_get_default_region(context);
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
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node1), apex, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node2), base, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node3), rv1, 0.0);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node4), rv2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(BASEPLANE, modeller.GetCurrentMode());
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node5), bp1, 0.25);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node6), bp2, 0.25);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(GUIDEPOINT, modeller.GetCurrentMode());

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
	Cmiss_region_destroy(&region);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, BasePlanePointsDifferentTimes)
{
	using namespace cap;
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_region_id region = Cmiss_context_get_default_region(context);
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
	modeller.ChangeMode(BASEPLANE);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node1), bp1, 0.2);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node2), bp2, 0.2);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node3), bp1, 0.4);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node4), bp2, 0.42);
	EXPECT_FALSE(modeller.OnAccept());
	EXPECT_EQ(BASEPLANE, modeller.GetCurrentMode());
	// Moving node4 to new time.
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node4), bp2, 0.40);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(GUIDEPOINT, modeller.GetCurrentMode());

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_region_destroy(&region);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AlignModel)
{
	using namespace cap;
	//Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	CAPClient mcc;
	mcc.gui_->CreateHeartModel();
	mcc.gui_->LoadTemplateHeartModel(25);
	Cmiss_region_id region = Cmiss_context_get_default_region(mcc.gui_->cmissContext_);

	Cmiss_node_id node1 = Cmiss_context_create_node(mcc.gui_->cmissContext_, 25.1651, -66.7366, -11.5065);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -14.4939, -17.1653, 28.9188);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(mcc.gui_->cmissContext_, 0.328333, -40.9305, -6.57035);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(mcc.gui_->cmissContext_, 8.43801, -63.0973, 38.1512);
	EXPECT_TRUE(node4 != 0);
	Cmiss_node_id node5 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -6.03773, -8.1123, 14.9891);
	EXPECT_TRUE(node5 != 0);
	Cmiss_node_id node6 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -19.2071, -27.9634, 38.2394);
	EXPECT_TRUE(node6 != 0);
	//Cmiss_node_id node7 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -18.4984, -52.6508, 33.958);
	//EXPECT_TRUE(node7 != 0);
	//Cmiss_node_id node8 = Cmiss_context_create_node(mcc.gui_->cmissContext_, -3.04825, -0.0334985, 0.7444);
	//EXPECT_TRUE(node8 != 0);

	Point3D apex(25.1651, -66.7366, -11.5065);
	Point3D base( -14.4939, -17.1653, 28.9188);
	Point3D rv1( 0.328333, -40.9305, -6.57035);
	Point3D rv2( 8.43801, -63.0973, 38.1512);
	Point3D bp1( -6.03773, -8.1123, 14.9891);
	Point3D bp2( -19.2071, -27.9634, 38.2394);
	//Point3D bp3(-18.4984, -52.6508, 33.958);
	//Point3D bp4(-3.04825, -0.0334985, 0.7444);

	Modeller modeller(&mcc);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node1), apex, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node2), base, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node3), rv1, 0.0);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node4), rv2, 0.0);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(BASEPLANE, modeller.GetCurrentMode());
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node5), bp1, 0.5);
	modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node6), bp2, 0.5);
	//modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node7), bp3, 0.8);
	//modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node8), bp4, 0.8);
	EXPECT_TRUE(modeller.OnAccept());
	EXPECT_EQ(GUIDEPOINT, modeller.GetCurrentMode());

	modeller.AlignModel();
	EXPECT_NEAR(32.5175, mcc.fl_, 1e-04);

	// The results are compared against the output from version 1.0.0 of the 
	// CAPClient software, hence the error target of 1e-05.
	EXPECT_NEAR(0.526947, mcc.m_[0][0], 1e-05);
	EXPECT_NEAR(-0.65865, mcc.m_[0][1], 1e-05);
	EXPECT_NEAR(-0.537128, mcc.m_[0][2], 1e-05);
	EXPECT_NEAR(0.0, mcc.m_[0][3], 1e-05);
	EXPECT_NEAR(-0.179444, mcc.m_[1][0], 1e-05);
	EXPECT_NEAR(-0.703966, mcc.m_[1][1], 1e-05);
	EXPECT_NEAR(0.687192, mcc.m_[1][2], 1e-05);
	EXPECT_NEAR(0.0, mcc.m_[1][3], 1e-05);
	EXPECT_NEAR(-0.830739, mcc.m_[2][0], 1e-05);
	EXPECT_NEAR(-0.265729, mcc.m_[2][1], 1e-05);
	EXPECT_NEAR(-0.489143, mcc.m_[2][2], 1e-05);
	EXPECT_NEAR(0.0, mcc.m_[2][3], 1e-05);
	EXPECT_NEAR(-1.27556, mcc.m_[3][0], 1e-05);
	EXPECT_NEAR(-33.6874, mcc.m_[3][1], 1e-04);
	EXPECT_NEAR(15.445, mcc.m_[3][2], 1e-04);
	EXPECT_NEAR(1.0, mcc.m_[3][3], 1e-06);

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_node_destroy(&node5);
	Cmiss_node_destroy(&node6);
	//Cmiss_node_destroy(&node7);
	//Cmiss_node_destroy(&node8);
	Cmiss_region_destroy(&region);
	//Cmiss_context_destroy(&context);
}

TEST(Modeller, InterpolateBasePlane)
{
	using namespace cap;
	Modeller modeller(0);

	Plane one;
	one.normal = Vector3D(0, 1, 0);
	one.position = Point3D();
	Plane two;
	two.normal = Vector3D(0, 1, 0);
	two.position = Point3D(0, 1, 0);
	std::map<double, Plane> planes;
	planes[0.0] = one;
	planes[1.0] = two;
	double time = 0.5;
	Plane iPlane = modeller.InterpolateBasePlane(planes, time);

	dbg("Interpolated : " + ToString(iPlane.position) + ", " + ToString(iPlane.normal));
	EXPECT_EQ(Point3D(0, 0.5, 0), iPlane.position);
}


