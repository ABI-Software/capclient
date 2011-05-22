/*
 * CAPContour.cpp
 *
 *  Created on: Oct 26, 2010
 *      Author: jchu014
 */
#include "CAPContour.h"

#include <iostream>
#include <sstream>
#include <limits>
#include <boost/bind.hpp>

extern "C" {
#include "api/cmiss_region.h"
#include "finite_element/finite_element_region.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
}
//#include "CmguiExtensions.h"
#include "computed_field/computed_field_finite_element.h"

namespace cap
{

CAPContour::CAPContour(size_t contourNumber, gtMatrix const& transform, std::vector<Point3D> const& points)
:
	contourNumber_(contourNumber),
	coords_(points)
{
	//TODO wrap gtMatrix in a class
	for (size_t i = 0;i<4;i++)
	{
		for(size_t j = 0;j<4;j++)
		{
			transform_[i][j] = transform[i][j];
		}
	}
}

CAPContour::~CAPContour()
{
}

}
