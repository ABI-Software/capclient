/*
 * contour.cpp
 *
 *  Created on: Oct 26, 2010
 *      Author: jchu014
 */
#include "contour.h"

#include <iostream>
#include <sstream>
#include <limits>
#include <boost/bind.hpp>

namespace cap
{

Contour::Contour(size_t contourNumber, gtMatrix const& transform, std::vector<Point3D> const& points)
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

Contour::~Contour()
{
}

}
