/*
 * CAPContour.h
 *
 *  Created on: Oct 18, 2010
 *      Author: jchu014
 */

#ifndef CAPCONTOUR_H_
#define CAPCONTOUR_H_

#include "math/algebra.h"

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace cap
{

// This class represents a contour line on an image.
// Note that this class does not contain any implementation details
// related to actually displaying the contour line in 3D.
// That is done by a class that implements the ImageSliceGraphics interface
// (e.g CmguiImageSliceGraphics)

class CAPContour : boost::noncopyable
{
public:
	CAPContour(size_t contourNumber, const gtMatrix& transform, std::vector<Point3D> const& points);
	~CAPContour();

	size_t GetFrameNumner() const
	{
		return frameNumber_;
	}
	
	size_t GetContourNumber() const
	{
		return contourNumber_;
	}
	
	std::vector<Point3D> const& GetCoordinates() const
	{
		return coords_;
	}
	
//	std::pair<double, double> GetValidTimeRange() const
//	{
//		return std::make_pair(startTime_, endTime_);
//	}
	
	gtMatrix const& GetTransformation() const
	{
		return transform_;
	}
	
private:
	size_t frameNumber_;
	size_t contourNumber_;
//	double startTime_, endTime_;
	
	gtMatrix transform_;
	
	std::vector<Point3D> coords_;
};

typedef boost::shared_ptr<CAPContour> ContourPtr;

} // namespace cap
#endif /* CAPCONTOUR_H_ */
