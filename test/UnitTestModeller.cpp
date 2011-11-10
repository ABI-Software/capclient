/*
 * UnitTestCAPModeller.cpp
 *
 *  Created on: Oct 19, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#include "model/modeller.h"
#include "heartmodel.h"

namespace cap 
{
// Fake implementation of HeartModel
double HeartModel::MapToModelFrameTime(double time) const
{
	return 0.0;
}

int HeartModel::MapToModelFrameNumber(double time) const
{
	return 0;
}

int HeartModel::ComputeXi(const Point3D& dataPoint, Point3D& xi, double time) const
{
	xi.x = 0.5;
	xi.y = 0.5;
	xi.z = 0.5;
	return 0;
}

Point3D HeartModel::TransformToLocalCoordinateRC(const Point3D& g) const
{
	return Point3D(0,0,0);
}

Point3D HeartModel::TransformToProlateSpheroidal(const Point3D& rc) const
{
	return Point3D(0,0,0);
}

void HeartModel::SetLambdaForFrame(const std::vector<double>& hermiteLambdaParams, int frameNumber)
{}

void HeartModel::SetLambda(const std::vector<double>& lambdaParams, double time)
{}

void HeartModel::SetLocalToGlobalTransformation(const gtMatrix& transform)
{}

void HeartModel::SetFocalLength(double focalLength)
{}

void HeartModel::SetTheta(int frameNumber)
{}

void HeartModel::SetMuFromBasePlaneForFrame(const Plane& plane, int frameNumber)
{}

// DataPoint
DataPoint::DataPoint(Cmiss_node*, cap::Point3D const&, cap::DataPointType, double, double)
{
}

DataPoint::DataPoint(DataPoint const& rhs)
{
}

DataPoint::~DataPoint()
{}

DataPointType DataPoint::GetDataPointType() const
{
	return APEX;
}

const Cmiss_node* DataPoint::GetCmissNode() const
{
	return 0;
}

Cmiss_node* DataPoint::GetCmissNode()
{
	return 0;
}

void DataPoint::SetValidPeriod(double startTime, double endTime)
{	
}

double DataPoint::GetTime() const
{
	return 0.0;
}

const Point3D& DataPoint::GetCoordinate() const
{
	return coordinate_;
}

void DataPoint::SetCoordinate(const Point3D& coord)
{}

std::string DataPoint::GetSliceName() const
{
	return "FOO";
}

void DataPoint::SetSurfaceType(SurfaceType)
{}

void DataPoint::SetVisible(bool)
{}

DataPoint& DataPoint::operator=(DataPoint const& rhs)
{
	if (this == &rhs)
		return *this;

	//Do assignment here

	return *this;
}

} // namespace cap

TEST(CAPModellerTest, CAPModellingModeApex)
{
	using namespace cap;
	CAPModellingModeApex m;
	m.PerformEntryAction();
	
}


TEST(CAPTimeSmootherTest, Create)
{
	using namespace cap;
	CAPTimeSmoother ts;
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


