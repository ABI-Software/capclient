/*
 * UnitTestCAPModeller.cpp
 *
 *  Created on: Oct 19, 2010
 *      Author: jchu014
 */

#include <gtest/gtest.h>
#include "CAPModeller.h"
#include "CAPModelLVPS4X4.h"

namespace cap 
{
// Fake implementation of CAPModelLVPS4X4
double CAPModelLVPS4X4::MapToModelFrameTime(double time) const
{
	return 0.0;
}

int CAPModelLVPS4X4::MapToModelFrameNumber(double time) const
{
	return 0;
}

int CAPModelLVPS4X4::ComputeXi(const Point3D& dataPoint, Point3D& xi, double time) const
{
	xi.x = 0.5;
	xi.y = 0.5;
	xi.z = 0.5;
	return 0;
}

Point3D CAPModelLVPS4X4::TransformToLocalCoordinateRC(const Point3D& g) const
{
	return Point3D(0,0,0);
}

Point3D CAPModelLVPS4X4::TransformToProlateSheroidal(const Point3D& rc) const
{
	return Point3D(0,0,0);
}

void CAPModelLVPS4X4::SetLambdaForFrame(const std::vector<double>& hermiteLambdaParams, int frameNumber)
{}

void CAPModelLVPS4X4::SetLambda(const std::vector<double>& lambdaParams, double time)
{}

void CAPModelLVPS4X4::SetLocalToGlobalTransformation(const gtMatrix& transform)
{}

void CAPModelLVPS4X4::SetFocalLengh(double focalLength)
{}

void CAPModelLVPS4X4::SetTheta(int frameNumber)
{}

void CAPModelLVPS4X4::SetMuFromBasePlaneForFrame(const Plane& plane, int frameNumber)
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
{}

} // namespace cap

TEST(CAPModellerTest, CAPModellingModeApex)
{
	using namespace cap;
	CAPModellingModeApex m;
	m.PerformEntryAction();
	
}
