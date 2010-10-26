/*
 * CAPContour.cpp
 *
 *  Created on: Oct 26, 2010
 *      Author: jchu014
 */
#include "CAPContour.h"

#include <iostream>
#include <sstream>
#include <boost/bind.hpp>

extern "C" {
#include "api/cmiss_region.h"
}
#include "CmguiExtensions.h"

namespace cap
{

CAPContour::CAPContour(size_t contourNumber, size_t frameNumber )
:
		contourNumber_(contourNumber),
		frameNumber_(frameNumber)
{
	
}

CAPContour::~CAPContour()
{
	// clean up cmgui resources
	// = nodes, region etc
}

namespace
{

int AddNodesToVector(Cmiss_node_id node, void *user_data)
{
	std::vector<Cmiss_node_id> nodes = *((std::vector<Cmiss_node_id>*)(user_data));
}

} // unnamed namespace

void CAPContour::ReadFromExFile(std::string const& filename, Cmiss_context_id context)
{
	filename_ = filename;
	Cmiss_region* root_region = Cmiss_context_get_default_region(context);
	
	if (!Cmiss_region_read_file(root_region,filename.c_str()))
	{
		std::cout << "Error reading ex file - " << filename << std::endl;
	}
	
	size_t positionOfLastSlash = filename.find_last_of("/\\");
	size_t positionOfDotExnodeExtension = filename.find_last_of(".");
	size_t regionNameLength = positionOfDotExnodeExtension - positionOfLastSlash - 1;
	std::string regionName = filename.substr(positionOfLastSlash + 1, regionNameLength);
	//DEBUG
	std::cout << "contour region = " << regionName <<"\n";
	
	std::stringstream ss;
	ss << "gfx mod g_e ";
	ss << regionName;
	ss << " node_points";
	Cmiss_context_execute_command(context, ss.str().c_str());
	
	//Store pointers to nodes so we can directly manipulate their visibility attributes
	nodes_.clear();
	
	Cmiss_region_id region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region << '\n';
		throw std::exception();
	}
	Cmiss_region_for_each_node_in_region(region, AddNodesToVector, (void*)&nodes_);
	
	// Clean up
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

void CAPContour::SetVisibility(bool visibility)
{
	std::for_each(nodes_.begin(), nodes_.end(),
			boost::bind(Cmiss_node_set_visibility_field, _1, startTime_, endTime_, visibility)
	);
}

void CAPContour::SetValidPeriod(double startTime, double endTime)
{
	const double EPSILON = std::numeric_limits<double>::epsilon();
	startTime_ = startTime;
	endTime_ = endTime - EPSILON;
}

}
