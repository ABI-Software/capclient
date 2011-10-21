/*
 * CmguiExtensions.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#include <math.h>
#include <iostream>

#ifdef _MSC_VER
# define WINDOWS_LEAN_AND_MEAN
# include <windows.h>
#endif
#include <GL/gl.h>

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_scene.h"
#include "api/cmiss_region.h"
#include "api/cmiss_graphics_material.h"
#include "api/cmiss_graphics_module.h"
#include "api/cmiss_time.h"
	
}

#include "CmguiExtensions.h"
#include "cmguipanel.h"

int Cmiss_region_read_file_with_time(struct Cmiss_region *region, char *file_name, Cmiss_time_keeper_id time_keeper, double time)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;
/*	struct Cmiss_region *temp_region;
	struct IO_stream_package *io_stream_package;
	struct FE_import_time_index time_index;
	double maximum, minimum;
	time_index.time = time;

	ENTER(Cmiss_region_read_file);
	return_code = 0;
	if (region && file_name && (io_stream_package=CREATE(IO_stream_package)()))
	{
		temp_region = Cmiss_region_create_region(region);
		if (read_exregion_file_of_name(temp_region,file_name,io_stream_package, &time_index))
		{
			if (Cmiss_regions_FE_regions_can_be_merged(region,temp_region))
			{
				return_code=Cmiss_regions_merge_FE_regions(region,temp_region);
				if (return_code)
				{
					/* Increase the range of the default time keepeer and set the
					   minimum and maximum if we set anything */
/*					maximum = Time_keeper_get_maximum(time_keeper);
					minimum = Time_keeper_get_minimum(time_keeper);
					if (time < minimum)
					{
						Time_keeper_set_minimum(time_keeper, time);
						Time_keeper_set_maximum(time_keeper, maximum);
					}
					if (time > maximum)
					{
						Time_keeper_set_minimum(time_keeper, minimum);
						Time_keeper_set_maximum(time_keeper, time);
					}
				}
			}
		}

		DEACCESS(Cmiss_region)(&temp_region);
		DESTROY(IO_stream_package)(&io_stream_package);
	}
	LEAVE;
*/
	return(return_code);
}

//#include "computed_field/computed_field_finite_element.h"

using namespace cap;

static Cmiss_node_id Cmiss_node_set_visibility_field_private(Cmiss_node_id node, 
		struct FE_region* fe_region, 
		Cmiss_field* visibilityField,
		struct FE_field *fe_field, double startTime, double endTime, bool visibility)
{
// 	struct FE_node_field_creator *node_field_creator;
// 	
// 	if (node_field_creator = CREATE(FE_node_field_creator)(
// 		/*number_of_components*/1))
// 	{
// 		struct FE_time_sequence *fe_time_sequence;
// 		//FE_value times[] = {0.0f, 0,5, 0.7, 1.0};
// 		//double halfTime = 1.0f/28.0f;
// //		double startTime = time - halfTime;
// //		double endTime = time + halfTime;
// 		FE_value times[5];
// 		double values[5];
// 		int numberOfTimes;
// 		
// 		double newFieldValue = visibility ? 1.0f : 0.0f;
// 		
// 		//Handle edge cases
// 		if (startTime == 0.0f && endTime == 1.0f)
// 		{
// 			times[0] = 0.0f;
// 			times[1] = 1.0f;
// 			values[0] = newFieldValue;
// 			values[1] = newFieldValue;
// 			numberOfTimes = 2;
// 		}
// 		else if (startTime == 0.0f)
// 		{
// 			times[0] = 0.0f;
// 			times[1] = endTime;
// 			times[2] = 1.0f;
// 			values[0] = newFieldValue;
// 			values[1] = newFieldValue;
// 			values[2] = 0;
// 			numberOfTimes = 3;
// 		}
// 		else if (endTime == 1.0f)
// 		{
// 			times[0] = 0.0f;
// 			times[1] = startTime;
// 			times[2] = 1.0f;
// 			values[0] = 0;
// 			values[1] = newFieldValue;
// 			values[2] = newFieldValue;
// 			numberOfTimes = 3;
// 		}
// 		else
// 		{
// 			times[0] = 0.0f;
// 			times[1] = startTime;
// 			times[2] = endTime;
// 			times[3] = 1.0f;
// 			values[0] = 0;
// 			values[1] = newFieldValue;
// 			values[2] = newFieldValue;
// 			values[3] = 0;
// 			numberOfTimes = 4;
// 		}
// 		
// 		if (!(fe_time_sequence = FE_region_get_FE_time_sequence_matching_series(
// 								fe_region, numberOfTimes, times)))
// 		{
// 			//Error
// 			std::cout << "Error: " << __func__ << " can't get time_sequence" << std::endl;
// 		}
// 		
// 		if (define_FE_field_at_node(node,fe_field,
// 			/*(struct FE_time_sequence *)NULL*/ fe_time_sequence,
// 			node_field_creator))
// 		{
// //			double one = 1, zero = 0 ,two = 2; //visibility_field > 1 => true
// //			Cmiss_field_set_values_at_node( visibilityField, node, 0 , 1 , &zero);
// //			Cmiss_field_set_values_at_node( visibilityField, node, 1 , 1 , &zero);
// //			double halfTime = 1.0f/28.0f;
// //			double startTime = (time - halfTime) >= 0 ? (time - halfTime) : (time - halfTime +1);
// //			Cmiss_field_set_values_at_node( visibilityField, node, startTime , 1 , &zero);
// //			double endTime = (time + halfTime) < 1 ? (time + halfTime) : (time + halfTime - 1);
// //			Cmiss_field_set_values_at_node( visibilityField, node, time + halfTime , 1 , &zero);
// //			Cmiss_field_set_values_at_node( visibilityField, node, time , 1 , &zero);
// //	
// //			Cmiss_field_set_values_at_node( visibilityField, node, 0.25 , 1 , &zero);
// //			Cmiss_field_set_values_at_node( visibilityField, node, 0.75 , 1 , &zero);
// //			Cmiss_field_set_values_at_node( visibilityField, node, 0.5 , 1 , &one);
// //			Cmiss_field_set_values_at_node( visibilityField, node, 0.7 , 1 , &one);
// 
// 			int number_of_values;
// 
// 			for (int i = 0; i < numberOfTimes; ++i)
// 			{									 													
// //				set_FE_nodal_field_FE_values_at_time(fe_field,
// //				 node, &(values[i]) , &number_of_values, times[i]);
// 				Cmiss_field_set_values_at_node( visibilityField, node, times[i] , 1 , &(values[i]));
// 			}
// 	
// 			DESTROY(FE_node_field_creator)(&node_field_creator);
// 			return node;
// 		}
// 	}

	return 0;
}

Cmiss_node_id Cmiss_node_set_visibility_field(Cmiss_node_id node, double startTime, double endTime, bool visibility)
{
//	std::cout << __func__ << " : start = " << startTime << " , end = " << endTime << '\n';
	
/*	FE_region* fe_region = FE_node_get_FE_region(node);
	
	Cmiss_region* cmiss_region;
	FE_region_get_Cmiss_region(fe_region, &cmiss_region);
		
	fe_region = FE_region_get_data_FE_region(fe_region);
	if (!fe_region)
	{
		std::cout << "fe_region is null" << std::endl;
	}
	
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
			return Cmiss_node_set_visibility_field_private( node, fe_region, visibilityField, fe_field, startTime, endTime, visibility);
		}
	}
*/
	return 0;
}

Cmiss_node_id Cmiss_create_data_point_at_coord(struct Cmiss_region *cmiss_region, Cmiss_field_id field, double* coords, double time)
{	
/*	FE_region* fe_region = Cmiss_region_get_FE_region(cmiss_region);
	fe_region = FE_region_get_data_FE_region(fe_region);
	
	if (!fe_region)
	{
		std::cout << "fe_region is null" << std::endl;
	}*/
	
//	int node_identifier = FE_region_get_next_FE_node_identifier(fe_region, /*start*/1);
//	std::cout << "node id = " << node_identifier << std::endl;
	
//	if (Cmiss_node_id node = /*ACCESS(FE_node)*/(CREATE(FE_node)(node_identifier, fe_region, (struct FE_node *)NULL)))
//	{
//		if (/*ACCESS(FE_node)*/(FE_region_merge_FE_node(fe_region, node)))
//		{
/*			int return_code;
			struct FE_field *fe_field;
			struct FE_node_field_creator *node_field_creator;
			struct LIST(FE_field) *fe_field_list;

			if (field && node)
			{
				if (field && (fe_field_list=
					Computed_field_get_defining_FE_field_list(field)))
				{
					if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
						(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
						(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
						fe_field_list)) && (3 >= get_FE_field_number_of_components(
						fe_field)) && (FE_VALUE_VALUE == get_FE_field_value_type(fe_field)))
					{*/
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

struct Viewer_frame_element_constraint_function_data
{
	struct FE_element *element, *found_element;
//	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *coordinate_field;
}; 

static int Viewer_frame_element_constraint_function(double *point,
	void *void_data)
/*******************************************************************************
LAST MODIFIED : 14 February 2008

DESCRIPTION : need to find the point of intersection between picking ray and obj
==============================================================================*/
{
	int return_code = 0;
	//struct Viewer_frame_element_constraint_function_data *data;

// 	if (point && (data = (struct Viewer_frame_element_constraint_function_data *)void_data))
// 	{
// 		data->found_element = data->element;
// 		return_code = Computed_field_find_element_xi(data->coordinate_field,
// 			point, /*number_of_values*/3, /*time ok to be 0 since slices don't move wrt time*/ 0.0, &(data->found_element), 
// 			data->xi, /*element_dimension*/2, 
// 			(struct Cmiss_region *)NULL, /*propagate_field*/0, /*find_nearest_location*/1);
// 		Computed_field_evaluate_in_element(data->coordinate_field,
// 			data->found_element, data->xi, /*time*/0.0, (struct FE_element *)NULL,
// 			point, (FE_value *)NULL);
// 	}
// 	else
// 	{
// 		display_message(ERROR_MESSAGE,
// 			"Node_tool_element_constraint_function.  Invalid argument(s)");
// 		return_code=0;
// 	}

	return (return_code);
} /* Node_tool_element_constraint_function */

Cmiss_element_id Cmiss_get_ray_intersection_point(Cmiss_scene_viewer_id scene_viewer, double x, double y, double* node_coordinates, Cmiss_field_id* field)
{
	int return_code = 0;

	//GLint viewport[4];
	
	//glGetIntegerv(GL_VIEWPORT,viewport);
	//double viewport_left   = (double)(viewport[0]);
	//double viewport_bottom = (double)(viewport[1]);
	//double viewport_width  = (double)(viewport[2]);
	//double viewport_height = (double)(viewport[3]);
	
	//double centre_x = x;
	/* flip y as x event has y=0 at top of window, increasing down */
//	double centre_y = viewport_height-y-1.0;
	
//	std::cout << viewport_height <<"," <<centre_y<< std::endl;
	
// 	GLdouble modelview_matrix[16], window_projection_matrix[16];
// 
// 	Scene_viewer_get_modelview_matrix(scene_viewer, modelview_matrix);
// 	Scene_viewer_get_window_projection_matrix(scene_viewer, window_projection_matrix);
// 	
// 	
// 	double size_x = 7.0;//FIX
// 	double size_y = 7.0;
// 	
// 	struct Interaction_volume *interaction_volume = create_Interaction_volume_ray_frustum(
// 					modelview_matrix,window_projection_matrix,
// 					viewport_left,viewport_bottom,viewport_width,viewport_height,
// 					centre_x,centre_y,size_x,size_y);
// 	
// 	FE_element* nearest_element;
// 	struct LIST(Scene_picked_object) *scene_picked_object_list;
// 	struct Scene_picked_object *scene_picked_object2;
// 	struct GT_element_group *gt_element_group_element;
// 	struct GT_element_settings *gt_element_settings_element;
// 	
// 	struct Scene* scene = Scene_viewer_get_scene(scene_viewer);
// 	struct Graphics_buffer* graphics_buffer = Scene_viewer_get_graphics_buffer(scene_viewer);
// 	
// 	if (scene_picked_object_list=
// 		Scene_pick_objects(scene,interaction_volume,graphics_buffer))
// 	{
// 		nearest_element = (struct FE_element *)NULL;

//		nearest_element=Scene_picked_object_list_get_nearest_element(
//			scene_picked_object_list,(struct Cmiss_region *)NULL,
//			/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
//			/*select_lines_enabled*/0, &scene_picked_object2,
//			&gt_element_group_element,&gt_element_settings_element);
//		
//		DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
//	}
	
	// Find the intersection of the element and the interaction volume /
//	Computed_field* nearest_element_coordinate_field = (struct Computed_field *)NULL;
//	if (nearest_element)
//	{
//		if (!(nearest_element_coordinate_field = 
//				GT_element_settings_get_coordinate_field(gt_element_settings_element)))
//		{
//			nearest_element_coordinate_field = 
//				GT_element_group_get_default_coordinate_field(
//				gt_element_group_element);
//		}

		//Test
//		*field = nearest_element_coordinate_field;

//		double node_coordinates[3];

//		Viewer_frame_element_constraint_function_data constraint_data;
//		constraint_data.element = nearest_element;
//		constraint_data.found_element = nearest_element;
//		constraint_data.coordinate_field = nearest_element_coordinate_field;
//		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
//		{
//			constraint_data.xi[i] = 0.5;
//		}
//		return_code = Interaction_volume_get_placement_point(interaction_volume,
//			node_coordinates, Viewer_frame_element_constraint_function,
//			&constraint_data);
//	}

//	if (return_code)
//	{
//		return nearest_element;
//	}
//	else
//	{ 
//		return (Cmiss_element_id)0;
//	}
	return 0;
}

#include "CAPMath.h"

Cmiss_node_id Cmiss_create_or_select_node_from_screen_coords(Cmiss_scene_viewer_id scene_viewer, double x, double y, double time, Point3D& coords)
{
	int return_code = 0;

	GLint viewport[4];
	
	glGetIntegerv(GL_VIEWPORT,viewport);
	double viewport_left   = (double)(viewport[0]);
	double viewport_bottom = (double)(viewport[1]);
	double viewport_width  = (double)(viewport[2]);
	double viewport_height = (double)(viewport[3]);
	
	double centre_x = x;
	/* flip y as x event has y=0 at top of window, increasing down */
	double centre_y = viewport_height-y-1.0;
	
//	std::cout << viewport_height <<"," <<centre_y<< std::endl;
	
	//GLdouble modelview_matrix[16], window_projection_matrix[16];

	//Scene_viewer_get_modelview_matrix(scene_viewer, modelview_matrix);
	//Scene_viewer_get_window_projection_matrix(scene_viewer, window_projection_matrix);
	
	
	double size_x = 7.0;//FIX
	double size_y = 7.0;
	
	//struct Interaction_volume *interaction_volume = create_Interaction_volume_ray_frustum(
	//				modelview_matrix,window_projection_matrix,
	//				viewport_left,viewport_bottom,viewport_width,viewport_height,
	//				centre_x,centre_y,size_x,size_y);
	
	//FE_element* nearest_element;
	//struct FE_node *picked_node;
	//struct LIST(Scene_picked_object) *scene_picked_object_list;
	//struct Scene_picked_object *scene_picked_object, *scene_picked_object2;
	//struct GT_element_group *gt_element_group, *gt_element_group_element;
	//struct GT_element_settings *gt_element_settings, *gt_element_settings_element;
	
	//struct Scene* scene = Scene_viewer_get_scene(scene_viewer);
	//struct Graphics_buffer* graphics_buffer = Scene_viewer_get_graphics_buffer(scene_viewer);
	
	//if (scene_picked_object_list=
	//	Scene_pick_objects(scene,interaction_volume,graphics_buffer))
	//{
	//	nearest_element = (struct FE_element *)NULL;

	//	picked_node=Scene_picked_object_list_get_nearest_node(
	//										scene_picked_object_list,1 /* use_data */,
	//										(struct Cmiss_region *)NULL,&scene_picked_object,
	//										&gt_element_group,&gt_element_settings);
		
	//	nearest_element=Scene_picked_object_list_get_nearest_element(
	//		scene_picked_object_list,(struct Cmiss_region *)NULL,
	//		/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
	//		/*select_lines_enabled*/0, &scene_picked_object2,
	//		&gt_element_group_element,&gt_element_settings_element);
		
		/* Reject the previously picked node if the element is nearer */
	//	if (picked_node && nearest_element)
	//	{
	//		if (Scene_picked_object_get_nearest(scene_picked_object) >
	//			Scene_picked_object_get_nearest(scene_picked_object2))
	//		{
	//			picked_node = (struct FE_node *)NULL;
	//		}
	//	}	
	//	DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
	//}
	
	/* Find the intersection of the element and the interaction volume */
/*	Computed_field* nearest_element_coordinate_field = (struct Computed_field *)NULL;
	if (picked_node)
	{
		return picked_node; // Cmiss_node = FE_node;
	}
	else if (nearest_element)
	{
		if (!(nearest_element_coordinate_field = 
				GT_element_settings_get_coordinate_field(gt_element_settings_element)))
		{
			nearest_element_coordinate_field = 
				GT_element_group_get_default_coordinate_field(
				gt_element_group_element);
		}


//		double node_coordinates[3];

		Viewer_frame_element_constraint_function_data constraint_data;
		constraint_data.element = nearest_element;
		constraint_data.found_element = nearest_element;
		constraint_data.coordinate_field = nearest_element_coordinate_field;
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			constraint_data.xi[i] = 0.5;
		}
		double node_coordinates[3];
		return_code = Interaction_volume_get_placement_point(interaction_volume,
			node_coordinates, Viewer_frame_element_constraint_function,
			&constraint_data);
		
		if (return_code)
		{
			Cmiss_region* region = Cmiss_element_get_region(nearest_element); // this doesn't work with groups
			std::cout << "region = " << Cmiss_region_get_path(region) << std::endl;
			
			coords.x = node_coordinates[0], coords.y = node_coordinates[1], coords.z = node_coordinates[2];
			std::cout << "debug: intersection point = " << coords <<  std::endl;
			
			double coordArray[3];
			coordArray[0] = coords.x;coordArray[1] = coords.y;coordArray[2] = coords.z;
			
			return Cmiss_create_data_point_at_coord(region, nearest_element_coordinate_field, (double*)coordArray, time);
		}
	}*/
	
	return 0;
}

Cmiss_node_id Cmiss_select_node_from_screen_coords(Cmiss_scene_viewer_id scene_viewer, double x, double y, double time, Point3D& coords)
{
	int return_code = 0;

	GLint viewport[4];
	
	glGetIntegerv(GL_VIEWPORT,viewport);
	double viewport_left   = (double)(viewport[0]);
	double viewport_bottom = (double)(viewport[1]);
	double viewport_width  = (double)(viewport[2]);
	double viewport_height = (double)(viewport[3]);
	
	double centre_x = x;
	/* flip y as x event has y=0 at top of window, increasing down */
	double centre_y = viewport_height-y-1.0;
	
//	std::cout << viewport_height <<"," <<centre_y<< std::endl;
	
	//GLdouble modelview_matrix[16], window_projection_matrix[16];

/*	Scene_viewer_get_modelview_matrix(scene_viewer, modelview_matrix);
	Scene_viewer_get_window_projection_matrix(scene_viewer, window_projection_matrix);*/
	
	
	double size_x = 7.0;//FIX
	double size_y = 7.0;
	
/*	struct Interaction_volume *interaction_volume = create_Interaction_volume_ray_frustum(
					modelview_matrix,window_projection_matrix,
					viewport_left,viewport_bottom,viewport_width,viewport_height,
					centre_x,centre_y,size_x,size_y);*/
	
	//FE_element* nearest_element;
	Cmiss_node_id picked_node = 0;
// 	struct LIST(Scene_picked_object) *scene_picked_object_list;
	//struct Scene_picked_object *scene_picked_object, *scene_picked_object2;
	//struct GT_element_group *gt_element_group, *gt_element_group_element;
	//struct GT_element_settings *gt_element_settings, *gt_element_settings_element;
	
// 	struct Scene* scene = Scene_viewer_get_scene(scene_viewer);
// 	struct Graphics_buffer* graphics_buffer = Scene_viewer_get_graphics_buffer(scene_viewer);
	
/*	if (scene_picked_object_list=
		Scene_pick_objects(scene,interaction_volume,graphics_buffer))*/
	//{
	//	nearest_element = (struct FE_element *)NULL;

//		picked_node=Scene_picked_object_list_get_nearest_node(
//											scene_picked_object_list,1 /* use_data */,
//											(struct Cmiss_region *)NULL,&scene_picked_object,
//											&gt_element_group,&gt_element_settings);
		
//		nearest_element=Scene_picked_object_list_get_nearest_element(
//			scene_picked_object_list,(struct Cmiss_region *)NULL,
//			/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
//			/*select_lines_enabled*/0, &scene_picked_object2,
//			&gt_element_group_element,&gt_element_settings_element);
		
		/* Reject the previously picked node if the element is nearer */
//		if (picked_node && nearest_element)
//		{
//			if (Scene_picked_object_get_nearest(scene_picked_object) >
//				Scene_picked_object_get_nearest(scene_picked_object2))
//			{
//				picked_node = (struct FE_node *)NULL;
//			}
//		}	
//		DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
	//}
	
	/* Find the intersection of the element and the interaction volume */
	//Computed_field* nearest_element_coordinate_field = (struct Computed_field *)NULL;
	
	return picked_node; // Cmiss_node = FE_node;
}

int Cmiss_move_node_to_screen_coords(Cmiss_scene_viewer_id scene_viewer, Cmiss_node_id node, double x, double y, double time, Point3D& coords)
{
	double node_coordinates[3];
	Cmiss_field_id field;
	Cmiss_element_id element = Cmiss_get_ray_intersection_point(scene_viewer, x, y, node_coordinates, &field);
	
	coords.x = node_coordinates[0], coords.y = node_coordinates[1], coords.z = node_coordinates[2];
	double coordArray[3];
	coordArray[0] = coords.x;coordArray[1] = coords.y;coordArray[2] = coords.z;
	//if (Cmiss_field_set_values_at_node( field, node, time , 3 , (double*) coordArray))
	//{
	//	return 1;
	//}
		
	return 0;
}

Cmiss_region_id Cmiss_get_slice_region(Cmiss_scene_viewer_id scene_viewer, double x, double y, double* node_coordinates, Cmiss_region_id region)
{
	int return_code = 0;

	GLint viewport[4];
	
	glGetIntegerv(GL_VIEWPORT,viewport);
	double viewport_left   = (double)(viewport[0]);
	double viewport_bottom = (double)(viewport[1]);
	double viewport_width  = (double)(viewport[2]);
	double viewport_height = (double)(viewport[3]);
	
	double centre_x = x;
	/* flip y as x event has y=0 at top of window, increasing down */
	double centre_y = viewport_height-y-1.0;
	
//	std::cout << viewport_height <<"," <<centre_y<< std::endl;
	
	//GLdouble modelview_matrix[16], window_projection_matrix[16];

//	Scene_viewer_get_modelview_matrix(scene_viewer, modelview_matrix);
//	Scene_viewer_get_window_projection_matrix(scene_viewer, window_projection_matrix);
	
	
	double size_x = 7.0;//FIX
	double size_y = 7.0;
	
//	struct Interaction_volume *interaction_volume = create_Interaction_volume_ray_frustum(
//					modelview_matrix,window_projection_matrix,
//					viewport_left,viewport_bottom,viewport_width,viewport_height,
//					centre_x,centre_y,size_x,size_y);
	
//	FE_element* nearest_element;
//	struct LIST(Scene_picked_object) *scene_picked_object_list;
//	struct Scene_picked_object *scene_picked_object2;
//	struct GT_element_group *gt_element_group_element;
//	struct GT_element_settings *gt_element_settings_element;
	
//	struct Scene* scene = Scene_viewer_get_scene(scene_viewer);
//	struct Graphics_buffer* graphics_buffer = Scene_viewer_get_graphics_buffer(scene_viewer);
	
//	if (scene_picked_object_list=
//		Scene_pick_objects(scene,interaction_volume,graphics_buffer))
//	{
//		nearest_element = (struct FE_element *)NULL;

//		nearest_element=Scene_picked_object_list_get_nearest_element(
//			scene_picked_object_list,region,
//			/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
//			/*select_lines_enabled*/0, &scene_picked_object2,
//			&gt_element_group_element,&gt_element_settings_element);
		
//		DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
//	}
	
	/* Find the intersection of the element and the interaction volume */
//	Computed_field* nearest_element_coordinate_field = (struct Computed_field *)NULL;
//	if (nearest_element)
//	{
//		if (!(nearest_element_coordinate_field = 
//				GT_element_settings_get_coordinate_field(gt_element_settings_element)))
//		{
//			nearest_element_coordinate_field = 
//				GT_element_group_get_default_coordinate_field(
//				gt_element_group_element);
//		}

		//Test
		//*field = nearest_element_coordinate_field;

//		double node_coordinates[3];

//		Viewer_frame_element_constraint_function_data constraint_data;
//		constraint_data.element = nearest_element;
//		constraint_data.found_element = nearest_element;
//		constraint_data.coordinate_field = nearest_element_coordinate_field;
//		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
//		{
//			constraint_data.xi[i] = 0.5;
//		}
//		return_code = Interaction_volume_get_placement_point(interaction_volume,
//			node_coordinates, Viewer_frame_element_constraint_function,
//			&constraint_data);
//	}

//	if (return_code)
//	{
//		Cmiss_region_id region = GT_element_group_get_Cmiss_region(gt_element_group_element);
//		return region;
//	}
//	else
//	{
//		return (Cmiss_region_id)0;
//	}

	return 0;
}
