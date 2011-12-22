/*
 * CmguiImageSliceGraphics.cpp
 *
 *  Created on: Jan 12, 2011
 *      Author: jchu014
 */


#include <iostream>
#include <cstdio>
#include <limits>

#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

extern "C"
{
#include <zn/cmiss_region.h>
}

#include "capclientconfig.h"
#include "CmguiImageSliceGraphics.h"
#include "material.h"
#include "cmgui/sceneviewerpanel.h"
#include "math/algebra.h"

namespace cap
{
	
CmguiImageSliceGraphics::CmguiImageSliceGraphics(
		const SceneViewerPanel& cmguiManager,
		const std::string& sliceName,
		const std::vector<Cmiss_texture*>& textures)
	: cmguiManager_(cmguiManager)
	, sliceName_(sliceName)
	, textures_(textures)
{
	this->LoadImagePlaneModel();
//		this->TransformImagePlane();
	
	this->InitializeDataPointGraphicalSetting();
}
	
CmguiImageSliceGraphics::~CmguiImageSliceGraphics()
{
	Cmiss_region_id root = 0; //-- TODO: fix this  Cmiss_context_get_default_region(cmguiManager_.GetCmissContext());
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root, sliceName_.c_str());
	if(!region)
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region << std::endl;
	}
	Cmiss_region_remove_child(root, region);
	std::cout << __func__ << '\n';
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root);
	
	BOOST_FOREACH(Cmiss_texture* tex, textures_)
	{
//		Cmiss_texture* temp = tex;
//		DEACCESS(Texture)(&temp);
//		DESTROY(Texture)(&tex);
	}
}
	
void CmguiImageSliceGraphics::SetVisible(bool visibility)
{
//	GT_visibility_type visible = visibility ? g_VISIBLE : g_INVISIBLE;
//	Scene_object_set_visibility(sceneObject_, visible);
}
	
void CmguiImageSliceGraphics::ChangeTexture(size_t index)
{
	// Cmiss_texture* tex = textures_[index]; Going put texture vector into cmiss_field_image
	material_->ChangeTexture(0); /**< TODO: correct this with by passing in a proper image field */
}

void CmguiImageSliceGraphics::SetBrightness(float brightness)
{
	material_->SetBrightness(brightness);
}
	
void CmguiImageSliceGraphics::SetContrast(float contrast)
{
	material_->SetContrast(contrast);
}
	
Point3D CmguiImageSliceGraphics::GetTopLeftCornerPosition()
{
	Cmiss_context_id cmissContext_ = 0; //-- TODO: fix this  cmguiManager_.GetCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);

	//Got to find the child region first!!
	Cmiss_region* region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region << std::endl;
	}

	//--Cmiss_node_id node;
	Point3D tlc;
	//--if (node = Cmiss_region_get_node(region, "3"))
	//--{
	//--	FE_node_get_position_cartesian(node, 0, &(tlc.x), &(tlc.y), &(tlc.z), 0);
	//--}
	//--else
	{
		std::cout << "Error:\n";
		throw std::exception();
	}
	
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
	
	return tlc;
}
	
void CmguiImageSliceGraphics::TransformTo(ImagePlane* plane)
{
	int nodeNum = 1;

	Cmiss_context_id cmissContext =  0; //-- TODO: fix this cmguiManager_.GetCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext);
	//Got to find the child region first!!
	Cmiss_region* region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : " << region << std::endl;
	}
	
	char nodeName[256]; //FIX
	sprintf(nodeName,"%d", nodeNum);
	Cmiss_node_id node = 0; // Cmiss_region_get_node(region, nodeName);
	//if (node) {
	//	FE_node_set_position_cartesian(node, 0, plane->blc.x, plane->blc.y, plane->blc.z);
	//}
	//else
	{
		std::cout << nodeName << std::endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node) // = Cmiss_region_get_node(region, nodeName))
	{
		//FE_node_set_position_cartesian(node, 0, plane->brc.x, plane->brc.y, plane->brc.z);
	}
	else
	{
		std::cout << nodeName << std::endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node) // = Cmiss_region_get_node(region, nodeName))
	{
		//FE_node_set_position_cartesian(node, 0, plane->tlc.x, plane->tlc.y, plane->tlc.z);
	}
	else
	{
		std::cout << nodeName << std::endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node) // = Cmiss_region_get_node(region, nodeName))
	{
		//FE_node_set_position_cartesian(node, 0, plane->trc.x, plane->trc.y, plane->trc.z);
	}
	else
	{
		std::cout << nodeName << std::endl;
	}

	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}
	
void CmguiImageSliceGraphics::LoadImagePlaneModel()
{
	std::cout << "CmguiImageSliceGraphics::LoadImagePlaneModel()" << std::endl;
	//-- TODO: fix me cmguiManager_.ReadRectangularModelFiles(sliceName_);
	Cmiss_graphics_module_id gModule = Cmiss_context_get_default_graphics_module(0 /* TODO: fix me cmissContext_ */);
	material_ = boost::make_shared<Material>(sliceName_, gModule);
	std::cout << "    " << material_ << std::endl;
	/** TODO: replace AssignMaterialToObject with CreateTextureImageSurface */
	// Assign material & cache the sceneObject for convenience
	//sceneObject_ = cmguiManager_.AssignMaterialToObject(0, material_->GetCmissMaterial(), sliceName_);
	//cmguiManager_->CreateTextureImageSurface(0, material_->GetCmissMaterial(), sliceName_);
}
	
Cmiss_field_id CmguiImageSliceGraphics::CreateVisibilityField()
{
	Cmiss_context_id cmissContext_ =  0; //-- TODO: fix this cmguiManager_.GetCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region* region;
	region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str());
	//CM_field_type cm_field_type = CM_GENERAL_FIELD;
	char* name = (char*)"visibility";
	//Coordinate_system coordinate_system;
	//coordinate_system.type = RECTANGULAR_CARTESIAN;
	//Value_type value_type = FE_VALUE_VALUE;
	const int number_of_components = 1;
	char* component_names[] = {(char*)"visibility"};
	
	//FE_region_get_FE_field_with_properties(
	//	Cmiss_region_get_FE_region(region),
	//	name, GENERAL_FE_FIELD,
	//	/*indexer_field*/(struct FE_field *)NULL, /*number_of_indexed_values*/0,
	//	cm_field_type, &coordinate_system,
	//	value_type, number_of_components, component_names,
	//	/*number_of_times*/0, /*time_value_type*/UNKNOWN_VALUE,
	//	/*external*/(struct FE_field_external_information *)NULL);
	
	//manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
	//Computed_field* field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("visibility",cfm);
	
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);

	return 0;
}

void CmguiImageSliceGraphics::InitializeDataPointGraphicalSetting()
{	
/*	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_DATA_POINTS);
	Graphical_material* material = create_Graphical_material("DataPoints");//TODO Need to clean up consider using Material_package
	Graphical_material* materialSelected = create_Graphical_material("DataPointsSelected");
	
	Colour green = {0,1,0}; //BGR
	Colour yellow = {0,1,1}; //BGR
	Graphical_material_set_diffuse(material, &yellow);
	Graphical_material_set_diffuse(materialSelected, &green);
	GT_element_settings_set_select_mode(settings, GRAPHICS_SELECT_ON);
	GT_element_settings_set_material(settings,material);
	GT_element_settings_set_selected_material(settings, materialSelected);

	//Glyphs
	GT_object *glyph, *old_glyph;
	Glyph_scaling_mode glyph_scaling_mode;
	Triple glyph_centre,glyph_scale_factors,glyph_size;
	Computed_field *orientation_scale_field, *variable_scale_field; ;
	glyph=make_glyph_sphere("sphere",12,6);
	
	Triple new_glyph_size;
	new_glyph_size[0] = 2, new_glyph_size[1] = 2, new_glyph_size[2] = 2;
	
	if (!(GT_element_settings_get_glyph_parameters(settings,
		 &old_glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
		 &orientation_scale_field, glyph_scale_factors,
		 &variable_scale_field) &&
		GT_element_settings_set_glyph_parameters(settings,glyph,
		 glyph_scaling_mode, glyph_centre, new_glyph_size,
		 orientation_scale_field, glyph_scale_factors,
		 variable_scale_field)))
	{
		std::cout << "No glyphs defined" << std::endl;
	}

	//Initialze fields needed for time-dependent visibility control

	Cmiss_field* visibilityField = CreateVisibilityField();
	GT_element_settings_set_visibility_field(settings, visibilityField);
	
	GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(sceneObject_);
	GT_element_group_add_settings(gt_element_group, settings, 0);
	
	/////////////////////////////////////////////////////////////////////
	// Do the same for node points
	// TODO eliminate code duplication
	{
		GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_NODE_POINTS);
		Graphical_material* material = create_Graphical_material("NodePoints");//TODO Need to clean up consider using Material_package
		Graphical_material* materialSelected = create_Graphical_material("NodePointsSelected");
		
		Colour green = {0,1,0}; //BGR
		Colour white = {1,1,1}; //BGR
		Graphical_material_set_diffuse(material, &white);
		Graphical_material_set_diffuse(materialSelected, &green);
		GT_element_settings_set_select_mode(settings, GRAPHICS_SELECT_ON);
		GT_element_settings_set_material(settings,material);
		GT_element_settings_set_selected_material(settings, materialSelected);
	
		//Glyphs
		GT_object *glyph, *old_glyph;
		Glyph_scaling_mode glyph_scaling_mode;
		Triple glyph_centre,glyph_scale_factors,glyph_size;
		Computed_field *orientation_scale_field, *variable_scale_field; ;
		glyph=make_glyph_point("point",g_POINT_MARKER, 1);
		
		Triple new_glyph_size;
		new_glyph_size[0] = 2, new_glyph_size[1] = 2, new_glyph_size[2] = 2;
		
		if (!(GT_element_settings_get_glyph_parameters(settings,
			 &old_glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
			 &orientation_scale_field, glyph_scale_factors,
			 &variable_scale_field) &&
			GT_element_settings_set_glyph_parameters(settings,glyph,
			 glyph_scaling_mode, glyph_centre, new_glyph_size,
			 orientation_scale_field, glyph_scale_factors,
			 variable_scale_field)))
		{
			std::cout << "No glyphs defined" << std::endl;
		}
	
		//Initialze fields needed for time-dependent visibility control
	
		Cmiss_field* visibilityField = CreateVisibilityField();
		GT_element_settings_set_visibility_field(settings, visibilityField);
		
		GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(sceneObject_);
		GT_element_group_add_settings(gt_element_group, settings, 0);
	}
	*/
}

namespace {

Cmiss_node_id Cmiss_node_set_visibility_field_private(Cmiss_node_id node, 
		struct FE_region* fe_region, 
		Cmiss_field* visibilityField,
		struct FE_field *fe_field, double startTime, double endTime, bool visibility)
{
//	std::cout << __func__ << " : start = " << startTime << " , end = " << endTime << '\n';
	
	struct FE_node_field_creator *node_field_creator = 0;
	
	if (node_field_creator) // = CREATE(FE_node_field_creator)(
//		/*number_of_components*/1))
	{
		struct FE_time_sequence *fe_time_sequence = 0;
		double times[5];
		double values[5];
		int numberOfTimes = 0;
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
		
		if (!(fe_time_sequence)) // = FE_region_get_FE_time_sequence_matching_series(
//								fe_region, numberOfTimes, times)))
		{
			//Error
			std::cout << "Error: " << __func__ << " can't get time_sequence" << std::endl;
		}
		
		if (0) //define_FE_field_at_node(node,fe_field,
//			/*(struct FE_time_sequence *)NULL*/ fe_time_sequence,
//			node_field_creator))
		{
			//--int number_of_values;
			for (int i = 0; i < numberOfTimes; ++i)
			{									 													
//--				Cmiss_field_set_values_at_node( visibilityField, node, times[i] , 1 , &(values[i]));
			}
//--			DESTROY(FE_node_field_creator)(&node_field_creator);
			return node;
		}
		else
		{
			std::cout << "Error define_FE_field_at_node\n";
		}
	}

	return node;
}

//#include "computed_field/computed_field_finite_element.h"

void SetValidPeriod(std::vector<Cmiss_node*> nodes, double startTime, double endTime)
{
//	std::cout << __func__ << " : start = " << startTime << " , end = " << endTime << '\n';
	
	const double EPSILON = std::numeric_limits<double>::epsilon();
	const double halfDuration = ( endTime - startTime ) * 0.5;
	startTime = std::max(0.0, startTime - halfDuration);
	endTime = endTime - halfDuration;
	
	Cmiss_node_id node = nodes.at(0);
		
//	FE_region* fe_region = FE_node_get_FE_region(node);
	
//	Cmiss_region* cmiss_region;
//	FE_region_get_Cmiss_region(fe_region, &cmiss_region);
		
//	fe_region = FE_region_get_data_FE_region(fe_region);
//	if (!fe_region)
//	{
//		std::cout << "fe_region is null" << std::endl;
//	}
	
//	manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(cmiss_region);
//	Computed_field* visibilityField = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("visibility",cfm);
	
//	if (!visibilityField)
//	{
//		display_message(ERROR_MESSAGE,
//			"Cmiss_node_set_visibility_field.  Can't find visibility field");
//	}
	
/*	struct FE_field *fe_field;
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
			std::for_each(nodes.begin(), nodes.end(),
					boost::bind(Cmiss_node_set_visibility_field_private, _1, fe_region, visibilityField,
							fe_field, startTime, endTime, true));
		}
	}*/
}

Cmiss_node_id Cmiss_create_node_point_at_coord(struct Cmiss_region *cmiss_region, Cmiss_field_id field, double* coords, double time)
{	
//	FE_region* fe_region = Cmiss_region_get_FE_region(cmiss_region);
//	fe_region = FE_region_get_data_FE_region(fe_region);
	
//	if (!fe_region)
//	{
//		std::cout << "fe_region is null" << std::endl;
//	}
	
//	int node_identifier = FE_region_get_next_FE_node_identifier(fe_region, /*start*/1);
//	std::cout << "node id = " << node_identifier << std::endl;
	
//	if (Cmiss_node_id node = /*ACCESS(FE_node)*/(CREATE(FE_node)(node_identifier, fe_region, (struct FE_node *)NULL)))
//	{
//		if (/*ACCESS(FE_node)*/(FE_region_merge_FE_node(fe_region, node)))
//		{
//			int return_code;
//			struct FE_field *fe_field;
//			struct FE_node_field_creator *node_field_creator;
//			struct LIST(FE_field) *fe_field_list;

//			if (field && node)
//			{
//				if (field && (fe_field_list=
//					Computed_field_get_defining_FE_field_list(field)))
//				{
//					if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
//						(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
//						(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
//						fe_field_list)) && (3 >= get_FE_field_number_of_components(
//						fe_field)) && (FE_VALUE_VALUE == get_FE_field_value_type(fe_field)))
//					{
//						if (node_field_creator = CREATE(FE_node_field_creator)(
//							/*number_of_components*/3))
//						{
//							if (define_FE_field_at_node(node,fe_field,
//								(struct FE_time_sequence *)NULL,
//								node_field_creator))
//							{
//								std::cout << "Field has been defined at data_point" << std::endl;
//								if (Cmiss_field_set_values_at_node( field, node, time , 3 , coords))
//								{							
//									return node;
//								}
//							}
//							else
//							{
//								display_message(ERROR_MESSAGE,
//									"Cmiss_create_data_point_at_coord.  Failed");
//								return_code=0;
//							}
//							DESTROY(FE_node_field_creator)(&node_field_creator);
//						}
//						else
//						{
//							display_message(ERROR_MESSAGE,
//								"Cmiss_create_data_point_at_coord.  Unable to make creator.");
//							return_code=0;
//						}
//					}
//					else
//					{
//						display_message(ERROR_MESSAGE,
//							"Cmiss_create_data_point_at_coord.  Invalid field");
//						return_code=0;
//					}
//					DESTROY(LIST(FE_field))(&fe_field_list);
//				}
//				else
//				{
//					display_message(ERROR_MESSAGE,
//						"Cmiss_create_data_point_at_coord.  No field to define");
//					return_code=0;
//				}
//			}
//		}
//		else
//		{
//			std::cout << "ERROR: Cant merge node to region" << std::endl; 
//			DEACCESS(Cmiss_node)(&node);
//		}
//	}
	
	std::cout << "ERROR: Can't Create node" << std::endl; 
	return 0;
}

} // anonymouos namespace

void CmguiImageSliceGraphics::CreateContour(size_t contourNum,
		std::vector<Point3D> const& coords,
		std::pair<double, double> const& validTimeRange,
		gtMatrix const& transform)
{
	std::cout << __func__ << " : contour num = " << contourNum << '\n';
	Cmiss_region_id root = 0; //-- TODO: fix this Cmiss_context_get_default_region(cmguiManager_.GetCmissContext());
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root, sliceName_.c_str());
	if(!region)
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region << std::endl;
	}
	
	// Get Coordinate field.
	Cmiss_field_id field = 0; //Cmiss_region_find_field_by_name(region, "coordinates_rect");
	if (!field)
	{
		std::cout << "Cmiss_field is not defined in the region : coordinates_rect" << "\n";
	}
	
//	for (int i = 0; i<4;i++)
//		for (int j = 0; j<4;j++)
//			std::cout << "transform[" << i << "][" << j<< "] = " << transform[i][j] << '\n';
	
	std::vector<Cmiss_node_id> nodes;
	BOOST_FOREACH(Point3D const& coord, coords)
	{
		// Set the transformation on the node.
		Point3D afterTransform = transform * coord;
//		std::cout << "b4 = " << coord << ", after = " << afterTransform << '\n';
		
		// Create a cmiss node for each coordinate point in coords
		double temp_x = static_cast<double>(afterTransform.x);
		Cmiss_node_id node = Cmiss_create_node_point_at_coord(
				region,
				field, 
				&temp_x,
				validTimeRange.first);
		
		nodes.push_back(node);
	}
	
	// Set the valid time range for the nodes.
	SetValidPeriod(nodes, validTimeRange.first, validTimeRange.second);
	
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root);
}

} // namespace cap
