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

#include "zinc/extensions.h"
#define private public
#include "model/modeller.h"
#undef private
#include "model/heart.h"
#include "modellercapclientwindow.h"
#include "modellercapclient.h"
#include "utils/debug.h"
#include "logmsg.h"
#include "math/geometry.h"


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

	Cmiss_node_id node1 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node2 != 0);
	using namespace cap;
	Modeller *modeller = new Modeller(0);
	Point3D coord1(4, 5, 6);
	Point3D coord2(9, 8, 7);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), coord1, 0.0);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), coord2, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(BASE, modeller->GetCurrentMode());

	delete modeller;
	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_region_destroy(&region);
	Cmiss_context_destroy(&context);
}

TEST(Modeller, AddBasePoints)
{
	Cmiss_context_id context = Cmiss_context_create("ModellerTest");
	Cmiss_region_id region = Cmiss_context_get_default_region(context);

	Cmiss_node_id node1 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node3 != 0);

	using namespace cap;
	Modeller *modeller = new Modeller(0);
	Point3D coord1(4, 5, 6);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), coord1, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	Point3D coord2(1, 5, 3);
	Point3D coord3(9, 8, 7);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), coord2, 0.0);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node3), coord3, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(RV, modeller->GetCurrentMode());

	delete modeller;
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

	Cmiss_node_id node1 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node4 != 0);

	Modeller *modeller = new Modeller(0);
	Point3D coord1(4, 5, 6);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), coord1, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	Point3D coord2(1, 5, 3);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), coord2, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	Point3D coord3(9, 8, 7);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node3), coord3, 0.0);
	EXPECT_FALSE(modeller->OnAccept());
	EXPECT_EQ(RV, modeller->GetCurrentMode());
	Point3D coord4(9, 10, 7);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node4), coord4, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(BASEPLANE, modeller->GetCurrentMode());

	delete modeller;
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

	Cmiss_node_id node1 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node4 != 0);
	Cmiss_node_id node5 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node5 != 0);
	Cmiss_node_id node6 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node6 != 0);

	Point3D apex(24.2506, -71.3943, -9.00449);
	Point3D base(-20.4335, -22.5206, 38.4314);
	Point3D rv1(30.286, -18.3221, 1.32952);
	Point3D rv2(-3.52178, -35.387, -20.3095);
	Point3D bp1(-18.4984, -52.6508, 43.958);
	Point3D bp2(-3.04825, -0.0334985, 8.7444);

	Modeller *modeller = new Modeller(&mcc);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), apex, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), base, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node3), rv1, 0.0);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node4), rv2, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(BASEPLANE, modeller->GetCurrentMode());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node5), bp1, 0.25);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node6), bp2, 0.25);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(GUIDEPOINT, modeller->GetCurrentMode());

	modeller->AlignModel();

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

	delete modeller;
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
	Cmiss_node_id node1 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(context);
	EXPECT_TRUE(node4 != 0);
	Point3D bp1(-18.4984, -52.6508, 43.958);
	Point3D bp2(-3.04825, -0.0334985, 8.7444);

	CAPClient mcc;
	mcc.gui_->CreateHeartModel();
	mcc.gui_->LoadTemplateHeartModel(25);
	Modeller *modeller = new Modeller(&mcc);
	modeller->ChangeMode(BASEPLANE);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), bp1, 0.2);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), bp2, 0.2);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node3), bp1, 0.4);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node4), bp2, 0.42);
	EXPECT_FALSE(modeller->OnAccept());
	EXPECT_EQ(BASEPLANE, modeller->GetCurrentMode());
	// Moving node4 to new time.
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node4), bp2, 0.40);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(GUIDEPOINT, modeller->GetCurrentMode());

	delete modeller;
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

	Cmiss_node_id node1 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node4 != 0);
	Cmiss_node_id node5 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node5 != 0);
	Cmiss_node_id node6 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node6 != 0);

	Point3D apex(25.1651, -66.7366, -11.5065);
	Point3D base( -14.4939, -17.1653, 28.9188);
	Point3D rv1( 0.328333, -40.9305, -6.57035);
	Point3D rv2( 8.43801, -63.0973, 38.1512);
	Point3D bp1( -6.03773, -8.1123, 14.9891);
	Point3D bp2( -19.2071, -27.9634, 38.2394);

	Modeller *modeller = new Modeller(&mcc);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), apex, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), base, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node3), rv1, 0.0);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node4), rv2, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(BASEPLANE, modeller->GetCurrentMode());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node5), bp1, 0.5);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node6), bp2, 0.5);
	//modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node7), bp3, 0.8);
	//modeller.AddModellingPoint(region, Cmiss_node_get_identifier(node8), bp4, 0.8);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(GUIDEPOINT, modeller->GetCurrentMode());

	modeller->AlignModel();
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

	delete modeller;
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

TEST(Modeller, VersionCompare1)
{
	using namespace cap;
	CAPClient mcc;
	mcc.gui_->CreateHeartModel();
	mcc.gui_->LoadTemplateHeartModel(25);
	Cmiss_region_id region = Cmiss_context_get_default_region(mcc.gui_->cmissContext_);

	Cmiss_node_id node1 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node1 != 0);
	Cmiss_node_id node2 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node2 != 0);
	Cmiss_node_id node3 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node3 != 0);
	Cmiss_node_id node4 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node4 != 0);
	Cmiss_node_id node5 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node5 != 0);
	Cmiss_node_id node6 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	EXPECT_TRUE(node6 != 0);

	Point3D apex( 89.628687430877548, -78.206408494868128, -21.803196495260526);
	Point3D base( 27.592421097555413, -17.769046781492548,  34.601963709271729);
	Point3D rv1( 53.171321476093446, -83.289550729043100,  11.0322556633171840);
	Point3D rv2( 50.039933609886077, -58.613376930432722, -19.0720215461791900);
	Point3D bp1( 43.875263893798689, -31.327755841858153,  49.4522910562259600);
	Point3D bp2( 27.780713203789222, -19.947668699134610,   8.7570269946642281);

	Modeller *modeller = new Modeller(&mcc);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), apex, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), base, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node3), rv1, 0.0);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node4), rv2, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(BASEPLANE, modeller->GetCurrentMode());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node5), bp1, 0.0);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node6), bp2, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(GUIDEPOINT, modeller->GetCurrentMode());

	modeller->AlignModel();
	modeller->SmoothAlongTime();
	EXPECT_NEAR(4.465629188395864e+001, mcc.fl_, 1e-04);

	// The results are compared against the output from version 1.0.0 of the
	// CAPClient software, hence the error target of 1e-05.
	EXPECT_NEAR(0.600212, mcc.m_[0][0], 1e-05);
	EXPECT_NEAR(-0.584743, mcc.m_[0][1], 1e-05);
	EXPECT_NEAR(-0.54573, mcc.m_[0][2], 1e-05);
	EXPECT_NEAR(0.0, mcc.m_[0][3], 1e-05);
	EXPECT_NEAR(-0.741702, mcc.m_[1][0], 1e-05);
	EXPECT_NEAR(-0.662281, mcc.m_[1][1], 1e-05);
	EXPECT_NEAR(-0.106123, mcc.m_[1][2], 1e-05);
	EXPECT_NEAR(0.0, mcc.m_[1][3], 1e-05);
	EXPECT_NEAR(-0.299372, mcc.m_[2][0], 1e-05);
	EXPECT_NEAR(0.468466, mcc.m_[2][1], 1e-05);
	EXPECT_NEAR(-0.831214, mcc.m_[2][2], 1e-05);
	EXPECT_NEAR(0.0, mcc.m_[2][3], 1e-05);
	EXPECT_NEAR(48.2691, mcc.m_[3][0], 1e-05);
	EXPECT_NEAR(-37.9128, mcc.m_[3][1], 1e-04);
	EXPECT_NEAR(15.8021, mcc.m_[3][2], 1e-04);
	EXPECT_NEAR(1.0, mcc.m_[3][3], 1e-06);

	const std::vector<double> lambdas0 = mcc.gui_->heartModel_->GetLambdaAtTime(0.0);
	EXPECT_NEAR(9.986390000000001e-001, lambdas0[0], 1e-05);
	EXPECT_NEAR(8.478210000000001e-001, lambdas0[4], 1e-05);
	EXPECT_NEAR(1.011044000000000e+000, lambdas0[15], 1e-05);
	EXPECT_NEAR(6.041640000000000e-001, lambdas0[28], 1e-05);
	EXPECT_NEAR(1.048598000000000e+000, lambdas0[36], 1e-05);

	const std::vector<double> mus = mcc.gui_->heartModel_->GetMuAtTime(0.0);
	EXPECT_NEAR(1.945295131734922e+000, mus[0], 1e-05);
	EXPECT_NEAR(4.766615711622894e-001, mus[33], 1e-05);

	const std::vector<double> lambdas5 = mcc.gui_->heartModel_->GetLambdaAtTime(0.035714285714285712*4);// 5th time step
	EXPECT_NEAR(8.738901737953558e-001, lambdas5[0], 1e-02);
	EXPECT_NEAR(8.037266357302894e-001, lambdas5[4], 1e-02);
	EXPECT_NEAR(1.008226359927051e+000, lambdas5[15], 1e-02);
	EXPECT_NEAR(5.802560785641432e-001, lambdas5[28], 1e-02);
	EXPECT_NEAR(1.009777476515084e+000, lambdas5[36], 1e-02);

	const std::vector<double> lambdas18 = mcc.gui_->heartModel_->GetLambdaAtTime(0.035714285714285712*17);// 18th time step
	EXPECT_NEAR(1.042354380279787e+000, lambdas18[3], 1e-02);
	EXPECT_NEAR(8.584946225012954e-001, lambdas18[9], 1e-02);
	EXPECT_NEAR(1.199457764436078e+000, lambdas18[17], 1e-02);
	EXPECT_NEAR(8.468334942475250e-001, lambdas18[23], 1e-02);
	EXPECT_NEAR(7.286260450192187e-001, lambdas18[25], 1e-02);

	delete modeller;
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

TEST(Modeller, MultiBasePlanePoints)
{
	cap::CAPClient mcc;
	mcc.gui_->CreateHeartModel();
	mcc.gui_->LoadTemplateHeartModel(25);
	Cmiss_region_id region = Cmiss_context_get_default_region(mcc.gui_->cmissContext_);

	Cmiss_node_id node1 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	Cmiss_node_id node2 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	Cmiss_node_id node3 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	Cmiss_node_id node4 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	Cmiss_node_id node5 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	Cmiss_node_id node6 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	Cmiss_node_id node7 = Cmiss_context_create_node(mcc.gui_->cmissContext_);
	Cmiss_node_id node8 = Cmiss_context_create_node(mcc.gui_->cmissContext_);

	cap::Point3D apex( 89.628687430877548, -78.206408494868128, -21.803196495260526);
	cap::Point3D base( 27.592421097555413, -17.769046781492548,  34.601963709271729);
	cap::Point3D rv1( 53.171321476093446, -83.289550729043100,  11.0322556633171840);
	cap::Point3D rv2( 50.039933609886077, -58.613376930432722, -19.0720215461791900);
	cap::Point3D bp1( 43.875263893798689, -31.327755841858153,  49.4522910562259600);
	cap::Point3D bp2( 27.780713203789222, -19.947668699134610,   8.7570269946642281);
	cap::Point3D bp3( 30.213103685565645, -22.394406862998537,   5.5534712125380929);
	cap::Point3D bp4( 48.648122315972287, -34.992845384900519,   57.783933663597807);

//	<cap:Point type="bp" time="0.285714">
//      <cap:Value variable="x">45.920617248331773</cap:Value>
//      <cap:Value variable="y">-33.575216286813578</cap:Value>
//      <cap:Value variable="z">44.312717123377922</cap:Value>
//    </cap:Point>
//    <cap:Point type="bp" time="0.285714">
//      <cap:Value variable="x">30.213103685565645</cap:Value>
//      <cap:Value variable="y">-22.394406862998537</cap:Value>
//      <cap:Value variable="z">5.5534712125380929</cap:Value>
//    </cap:Point>
//    <cap:Point type="bp" time="0.285714">
//      <cap:Value variable="x">24.969554998415806</cap:Value>
//      <cap:Value variable="y">-18.79270377682974</cap:Value>
//      <cap:Value variable="z">-9.0674626920319561</cap:Value>
//    </cap:Point>
//    <cap:Point type="bp" time="0.285714">
//      <cap:Value variable="x">48.648122315972287</cap:Value>
//      <cap:Value variable="y">-34.992845384900519</cap:Value>
//      <cap:Value variable="z">57.783933663597807</cap:Value>
//    </cap:Point>
	cap::Modeller *modeller = new cap::Modeller(&mcc);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node1), apex, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node2), base, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node3), rv1, 0.0);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node4), rv2, 0.0);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(cap::BASEPLANE, modeller->GetCurrentMode());
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node5), bp1, 0.5);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node6), bp2, 0.5);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node7), bp3, 0.5);
	modeller->AddModellingPoint(region, Cmiss_node_get_identifier(node8), bp4, 0.5);
	EXPECT_TRUE(modeller->OnAccept());
	EXPECT_EQ(cap::GUIDEPOINT, modeller->GetCurrentMode());

	Cmiss_node_destroy(&node1);
	Cmiss_node_destroy(&node2);
	Cmiss_node_destroy(&node3);
	Cmiss_node_destroy(&node4);
	Cmiss_node_destroy(&node5);
	Cmiss_node_destroy(&node6);
	Cmiss_node_destroy(&node7);
	Cmiss_node_destroy(&node8);
	Cmiss_region_destroy(&region);
	//Cmiss_context_destroy(&context);
}
