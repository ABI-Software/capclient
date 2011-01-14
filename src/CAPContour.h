/*
 * CAPContour.h
 *
 *  Created on: Oct 18, 2010
 *      Author: jchu014
 */

#ifndef CAPCONTOUR_H_
#define CAPCONTOUR_H_

#include "CAPMath.h"

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

//extern "C" {
//#include "api/cmiss_node.h"
//#include "api/cmiss_context.h"
//}

//struct Scene_object;

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
//	CAPContour(size_t contourNumber, size_t frameNumber, std::string const& filename);
	CAPContour(size_t contourNumber, const gtMatrix& transform, std::vector<Point3D> const& points);
	~CAPContour();
	
//	void ReadFromExFile(Cmiss_context_id context); // FIXME
//	void SetVisibility(bool visibility);
//	void SetValidPeriod(double startTime, double endTime);
//	std::string const& GetFilename() const
//	{
//		return filename_;
//	}
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
	size_t contourNumber_;
//	size_t frameNumber_;
//	std::string filename_;
	double startTime_, endTime_;
	
	gtMatrix transform_;
	
	std::vector<Point3D> coords_;
	
//	std::vector<Cmiss_node_id> nodes_; //FIXME
	
//	struct ContourImpl;
//	boost::scoped_ptr<ContourImpl> pImpl_;
};

typedef boost::shared_ptr<CAPContour> ContourPtr;

} // namespace cap
#endif /* CAPCONTOUR_H_ */
