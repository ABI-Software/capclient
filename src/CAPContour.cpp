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
#include "finite_element/finite_element_region.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
}
//#include "CmguiExtensions.h"
#include "computed_field/computed_field_finite_element.h"

namespace cap
{

struct CAPContour::ContourImpl
{
	ContourImpl()
	:
		region(0),
		cmissContext(0),
		sceneObject(0)
	{}
	
	Cmiss_context_id cmissContext;
	Cmiss_region* region;
	Scene_object* sceneObject;
//	Cmiss_field_id field;
};


CAPContour::CAPContour(size_t contourNumber, size_t frameNumber, std::string const& filename )
:
		contourNumber_(contourNumber),
		frameNumber_(frameNumber),
		filename_(filename),
		pImpl_(new CAPContour::ContourImpl())
{
	
}

CAPContour::~CAPContour()
{
	if (pImpl_->region == 0)
	{
		return;
	}
	
	// clean up cmgui resources
	// = nodes, region etc
	Cmiss_region_id root = Cmiss_context_get_default_region(pImpl_->cmissContext);
	Cmiss_region_id region = pImpl_->region;
	Cmiss_region_remove_child(root, region);
	std::cout << __func__ << '\n';
//		Cmiss_region_destroy(&region);
//		Cmiss_region_destroy(&pImpl_->region);
	Cmiss_region_destroy(&root);
}

namespace
{

int AddNodesToVector(Cmiss_node_id node, void *user_data)
{
//	std::cout << __func__ << '\n';
	std::vector<Cmiss_node_id>& nodes = *((std::vector<Cmiss_node_id>*)(user_data));
	nodes.push_back(node);
}

} // unnamed namespace

// FIXME : currently this func has to be called after previous image set has been deleted
//
void CAPContour::ReadFromExFile(Cmiss_context_id context)
{
	pImpl_->cmissContext = context;
	
	Cmiss_region* root_region = Cmiss_context_get_default_region(context);
	
	if (!Cmiss_region_read_file(root_region,filename_.c_str()))
	{
		std::cout << "Error reading ex file - " << filename_ << std::endl;
	}
	
	size_t positionOfLastSlash = filename_.find_last_of("/\\");
	size_t positionOfDotExnodeExtension = filename_.find_last_of(".");
	size_t regionNameLength = positionOfDotExnodeExtension - positionOfLastSlash - 1;
	std::string regionName = filename_.substr(positionOfLastSlash + 1, regionNameLength);
	//DEBUG
	std::cout << "contour region = " << regionName <<"\n";
	
	//Store pointers to nodes so we can directly manipulate their visibility attributes
	nodes_.clear();
	
	Cmiss_region_id region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region << '\n';
		throw std::exception();
	}
	pImpl_->region = region;
	
	Cmiss_region_for_each_node_in_region(region, AddNodesToVector, (void*)&nodes_);
	
	// Create visibility field //TODO Factor out
	CM_field_type cm_field_type = CM_GENERAL_FIELD;
	char* name = (char*)"visibility";
	Coordinate_system coordinate_system;
	coordinate_system.type = RECTANGULAR_CARTESIAN;
	Value_type value_type = FE_VALUE_VALUE;
	const int number_of_components = 1;
	char* component_names[] = {(char*)"visibility"};
	
	FE_region_get_FE_field_with_properties(
		Cmiss_region_get_FE_region(region),
		name, GENERAL_FE_FIELD,
		/*indexer_field*/(struct FE_field *)NULL, /*number_of_indexed_values*/0,
		cm_field_type, &coordinate_system,
		value_type, number_of_components, component_names,
		/*number_of_times*/0, /*time_value_type*/UNKNOWN_VALUE,
		/*external*/(struct FE_field_external_information *)NULL);
	
//	manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
//	Computed_field* field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("visibility",cfm);
	
	std::stringstream ss;
	ss << "gfx mod g_e ";
	ss << regionName;
	ss << " node_points visibility_field visibility";
	
	
	Cmiss_context_execute_command(context, ss.str().c_str());
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(context);
	Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	pImpl_->sceneObject = Scene_get_scene_object_with_Cmiss_region(scene, region);
	
	// Clean up
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

namespace {

Cmiss_node_id Cmiss_node_set_visibility_field_private(Cmiss_node_id node, 
		struct FE_region* fe_region, 
		Cmiss_field* visibilityField,
		struct FE_field *fe_field, double startTime, double endTime, bool visibility)
{
//	std::cout << __func__ << " : start = " << startTime << " , end = " << endTime << '\n';
	
	struct FE_node_field_creator *node_field_creator;
	
	if (node_field_creator = CREATE(FE_node_field_creator)(
		/*number_of_components*/1))
	{
		struct FE_time_sequence *fe_time_sequence;
		FE_value times[5];
		double values[5];
		int numberOfTimes;
		double newFieldValue = visibility ? 1.0f : 0.0f;
		
		//Handle edge cases
		if (startTime == 0.0f && endTime == 1.0f)
		{
			times[0] = 0.0f;
			times[1] = 1.0f;
			values[0] = newFieldValue;
			values[1] = newFieldValue;
			numberOfTimes = 2;
		}
		else if (startTime == 0.0f)
		{
			times[0] = 0.0f;
			times[1] = endTime;
			times[2] = 1.0f;
			values[0] = newFieldValue;
			values[1] = newFieldValue;
			values[2] = 0;
			numberOfTimes = 3;
		}
		else if (endTime == 1.0f)
		{
			times[0] = 0.0f;
			times[1] = startTime;
			times[2] = 1.0f;
			values[0] = 0;
			values[1] = newFieldValue;
			values[2] = newFieldValue;
			numberOfTimes = 3;
		}
		else
		{
			times[0] = 0.0f;
			times[1] = startTime;
			times[2] = endTime;
			times[3] = 1.0f;
			values[0] = 0;
			values[1] = newFieldValue;
			values[2] = newFieldValue;
			values[3] = 0;
			numberOfTimes = 4;
		}
		
		if (!(fe_time_sequence = FE_region_get_FE_time_sequence_matching_series(
								fe_region, numberOfTimes, times)))
		{
			//Error
			std::cout << "Error: " << __func__ << " can't get time_sequence" << std::endl;
		}
		
		if (define_FE_field_at_node(node,fe_field,
			/*(struct FE_time_sequence *)NULL*/ fe_time_sequence,
			node_field_creator))
		{
			int number_of_values;
			for (int i = 0; i < numberOfTimes; ++i)
			{									 													
				Cmiss_field_set_values_at_node( visibilityField, node, times[i] , 1 , &(values[i]));
			}
			DESTROY(FE_node_field_creator)(&node_field_creator);
			return node;
		}
		else
		{
			std::cout << "Error define_FE_field_at_node\n";
		}
	}
}


}

void CAPContour::SetVisibility(bool visibility)
{
//	std::cout << __func__ << ": " << nodes_.size() << '\n';
	Scene_object_set_visibility(pImpl_->sceneObject, visibility ? g_VISIBLE : g_INVISIBLE );
}

void CAPContour::SetValidPeriod(double startTime, double endTime)
{
//	std::cout << __func__ << " : start = " << startTime << " , end = " << endTime << '\n';
	
	const double EPSILON = std::numeric_limits<double>::epsilon();
	startTime_ = startTime;
	endTime_ = endTime - EPSILON;
	
	Cmiss_node_id node = nodes_.at(0);
		
	FE_region* fe_region = FE_node_get_FE_region(node);
	
	Cmiss_region* cmiss_region;
	FE_region_get_Cmiss_region(fe_region, &cmiss_region);
		
//	fe_region = FE_region_get_data_FE_region(fe_region);
//	if (!fe_region)
//	{
//		std::cout << "fe_region is null" << std::endl;
//	}
	
	manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(cmiss_region);
	Computed_field* visibilityField = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("visibility",cfm);
	
	if (!visibilityField)
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_node_set_visibility_field.  Can't find visibility field");
	}
	
	struct FE_field *fe_field;
	struct LIST(FE_field) *fe_field_list;
	if (visibilityField && (fe_field_list=
						Computed_field_get_defining_FE_field_list(visibilityField)))
	{
		if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
			(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
			(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
			fe_field_list)) && (1 == get_FE_field_number_of_components(
			fe_field)) && (FE_VALUE_VALUE == get_FE_field_value_type(fe_field)))
		{
			std::for_each(nodes_.begin(), nodes_.end(),
					boost::bind(Cmiss_node_set_visibility_field_private, _1, fe_region, visibilityField,
							fe_field, startTime_, endTime_, true));
		}
	}
}

}
