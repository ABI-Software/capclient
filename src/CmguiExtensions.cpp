/*
 * CmguiExtensions.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#include "CmguiExtensions.h"
#include "CmguiManager.h"
#include <math.h>
#include <iostream>

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_scene_viewer_private.h"
#include "general/debug.h"
#include "graphics/scene_viewer.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"
#include "graphics/scene.h"

//#include "command/cmiss.h"
#include "general/debug.h"
#include "general/object.h"

#include "time/time_keeper.h"

#include "api/cmiss_region.h"
#include "finite_element/import_finite_element.h"
#include "api/cmiss_texture.h"
#include "graphics/material.h"
#include "general/manager.h"
//#include "finite_element/finite_element.h"
#include "time/time.h"
	
#include "command/cmiss.h"
}

Cmiss_scene_viewer_id create_Cmiss_scene_viewer_wx(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	wxPanel* panel,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified
<port> window handle.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_Carbon);
	/* Not implemented yet */
//	USE_PARAMETER(minimum_colour_buffer_depth);
//	USE_PARAMETER(minimum_accumulation_buffer_depth);
//	USE_PARAMETER(minimum_depth_buffer_depth);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_wx(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			panel,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth, (Graphics_buffer *)NULL);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_Carbon.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
}

#ifdef LATER
int Viewer_view_all(Cmiss_scene_viewer_id scene_viewer)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
{

	int return_code;


	struct Scene* scene = Scene_viewer_get_scene(scene_viewer);

	if (scene_viewer && scene)
	{
		return_code = 1;
		double centre_x,centre_y,centre_z,clip_factor,radius,
			size_x,size_y,size_z,width_factor;

		Scene_get_graphics_range(scene,
			&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z);
		radius = 0.5*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);

		double left, right, bottom, top, near_plane, far_plane;
		if (0 == radius)
		{
			/* get current "radius" from first scene viewer */
			Scene_viewer_get_viewing_volume(scene_viewer,
				&left, &right, &bottom, &top, &near_plane, &far_plane);
			radius = 0.5*(right - left);
		}
		else
		{
			/*???RC width_factor should be read in from defaults file */
			width_factor = 1.05;
			/* enlarge radius to keep image within edge of window */
			radius *= width_factor;
		}

		/*???RC clip_factor should be read in from defaults file: */
		clip_factor = 10.0;

		return_code = Scene_viewer_set_view_simple(
			scene_viewer, centre_x, centre_y, centre_z,
			radius, 40, clip_factor*radius);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_all.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}
#endif

int Cmiss_region_read_file_with_time(struct Cmiss_region *region, char *file_name, struct Time_keeper* time_keeper, float time)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *temp_region;
	struct IO_stream_package *io_stream_package;
	struct FE_import_time_index time_index;
	double maximum, minimum;
	time_index.time = time;

	ENTER(Cmiss_region_read_file);
	return_code = 0;
	if (region && file_name && (io_stream_package=CREATE(IO_stream_package)()))
	{
		temp_region = Cmiss_region_create_share_globals(region);
		if (read_exregion_file_of_name(temp_region,file_name,io_stream_package, &time_index))
		{
			if (Cmiss_regions_FE_regions_can_be_merged(region,temp_region))
			{
				return_code=Cmiss_regions_merge_FE_regions(region,temp_region);
				if (return_code)
				{
					/* Increase the range of the default time keepeer and set the
					   minimum and maximum if we set anything */
					maximum = Time_keeper_get_maximum(time_keeper);
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

	return(return_code);
}

Cmiss_node_id Cmiss_create_node_at_coord(struct Cmiss_region *cmiss_region, Cmiss_field_id field, float* coords)
{	
	FE_region* fe_region = Cmiss_region_get_FE_region(cmiss_region);
	if (!fe_region)
	{
		std::cout << "fe_region is null" << std::endl;
	}
	
	int node_identifier = FE_region_get_next_FE_node_identifier(fe_region, /*start*/1);
	std::cout << "node id = " << node_identifier << std::endl;
	
	if (Cmiss_node_id node = create_Cmiss_node(node_identifier, cmiss_region))
	{
		if (Cmiss_region_merge_Cmiss_node(cmiss_region, node))
		{
			if (Cmiss_field_finite_element_define_at_node(
					field,  node,
					0 /* time_sequence*/, 0/* node_field_creator*/) &&
				Cmiss_field_set_values_at_node( field, node, 0 /* time*/ , 3 , coords))
			{
				return node;
			}
		}
		else
		{
			DEACCESS(Cmiss_node)(&node);
		}
	}
	
	return 0;
}

#include "computed_field/computed_field_finite_element.h"
Cmiss_node_id Cmiss_create_data_point_at_coord(struct Cmiss_region *cmiss_region, Cmiss_field_id field, float* coords, float time)
{	
	FE_region* fe_region = Cmiss_region_get_FE_region(cmiss_region);
	fe_region = FE_region_get_data_FE_region(fe_region);
	
	if (!fe_region)
	{
		std::cout << "fe_region is null" << std::endl;
	}
	
	int node_identifier = FE_region_get_next_FE_node_identifier(fe_region, /*start*/1);
	std::cout << "node id = " << node_identifier << std::endl;
	
	if (Cmiss_node_id node = ACCESS(FE_node)(CREATE(FE_node)(node_identifier, fe_region, (struct FE_node *)NULL)))
	{
		if (/*ACCESS(FE_node)*/(FE_region_merge_FE_node(fe_region, node)))
		{
			int return_code;
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
					{
						if (node_field_creator = CREATE(FE_node_field_creator)(
							/*number_of_components*/3))
						{
							if (define_FE_field_at_node(node,fe_field,
								(struct FE_time_sequence *)NULL,
								node_field_creator))
							{
//								std::cout << "Field has been defined at data_point" << std::endl;
								if (Cmiss_field_set_values_at_node( field, node, time , 3 , coords))
								{							
									//TEST								
									manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(cmiss_region);
									Computed_field* visibilityField = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("visibility",cfm);
									
									if (!visibilityField)
									{
										display_message(ERROR_MESSAGE,
											"Cmiss_create_data_point_at_coord.  Can't find visibility field");
									}
									if (visibilityField && (fe_field_list=
														Computed_field_get_defining_FE_field_list(visibilityField)))
									{
										if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
											(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
											(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
											fe_field_list)) && (1 == get_FE_field_number_of_components(
											fe_field)) && (FE_VALUE_VALUE == get_FE_field_value_type(fe_field)))
										{
											struct FE_node_field_creator *node_field_creator2;
											
											if (node_field_creator2 = CREATE(FE_node_field_creator)(
												/*number_of_components*/1))
											{
												struct FE_time_sequence *fe_time_sequence;
												//FE_value times[] = {0.0f, 0,5, 0.7, 1.0};
												float halfTime = 1.0f/28.0f;
												float startTime = time - halfTime; 
												float endTime = time + halfTime;
												FE_value times[5];
												FE_value values[5];
												int numberOfTimes;
												
												if (time == 0.0)
												{
													times[0] = 0.0f;
													times[1] = endTime;
													times[2] = 1.0f;
													values[0] = 2;
													values[1] = 1;
													values[2] = 0;
													numberOfTimes = 3;
												}
												else if (time == 1.0)
												{
													times[0] = 0.0f;
													times[1] = startTime;
													times[2] = time;
													values[0] = 0;
													values[1] = 1;
													values[2] = 2;
													numberOfTimes = 3;
												}
												else if (startTime < 0.0)
												{
													times[0] = 0.0f;
													times[1] = time;
													times[2] = endTime;
													times[3] = 1.0f;
													values[0] = 1;
													values[1] = 2;
													values[2] = 1;
													values[3] = 0;
													numberOfTimes = 4;
												}
												else if (endTime > 1.0)
												{
													times[0] = 0.0f;
													times[1] = startTime;
													times[2] = time;
													times[3] = endTime;
													values[0] = 0;
													values[1] = 1;
													values[2] = 2;
													values[3] = 1;
													numberOfTimes = 4;
												}
												else
												{
													times[0] = 0.0f;
													times[1] = startTime;
													times[2] = time;
													times[3] = endTime;
													times[4] = 1.0f;
													values[0] = 0;
													values[1] = 1;
													values[2] = 2;
													values[3] = 1;
													values[4] = 0;
													numberOfTimes = 5;
												}
												
												if (!(fe_time_sequence = FE_region_get_FE_time_sequence_matching_series(
																		fe_region, numberOfTimes, times)))
												{
													//Error
													std::cout << "Error: " << __func__ << " can't get time_sequence" << std::endl;
												}
												
												if (define_FE_field_at_node(node,fe_field,
													/*(struct FE_time_sequence *)NULL*/ fe_time_sequence,
													node_field_creator2))
												{
													float one = 1, zero = 0 ,two = 2; //visibility_field > 1 => true
//													Cmiss_field_set_values_at_node( visibilityField, node, 0 , 1 , &zero);
//													Cmiss_field_set_values_at_node( visibilityField, node, 1 , 1 , &zero);
//													float halfTime = 1.0f/28.0f;
////													float startTime = (time - halfTime) >= 0 ? (time - halfTime) : (time - halfTime +1); 
////													Cmiss_field_set_values_at_node( visibilityField, node, startTime , 1 , &zero);
////													float endTime = (time + halfTime) < 1 ? (time + halfTime) : (time + halfTime - 1); 
////													Cmiss_field_set_values_at_node( visibilityField, node, time + halfTime , 1 , &zero);
////													Cmiss_field_set_values_at_node( visibilityField, node, time , 1 , &zero);
//											
//													Cmiss_field_set_values_at_node( visibilityField, node, 0.25 , 1 , &zero);
//													Cmiss_field_set_values_at_node( visibilityField, node, 0.75 , 1 , &zero);
//													Cmiss_field_set_values_at_node( visibilityField, node, 0.5 , 1 , &one);
//													Cmiss_field_set_values_at_node( visibilityField, node, 0.7 , 1 , &one);
//													
//													int set_FE_nodal_field_FE_values_at_time(struct FE_field *field,
//														struct FE_node *node,FE_value *values,int *number_of_values,
//														FE_value time);
													
													int number_of_values;

													for (int i = 0; i < numberOfTimes; ++i)
													{									 													
														set_FE_nodal_field_FE_values_at_time(fe_field,
														 node, &(values[i]) , &number_of_values, times[i]);
													}
											
													return node;
												}
											}
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_create_data_point_at_coord.  Failed");
								return_code=0;
							}
							DESTROY(FE_node_field_creator)(&node_field_creator);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_create_data_point_at_coord.  Unable to make creator.");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_create_data_point_at_coord.  Invalid field");
						return_code=0;
					}
					DESTROY(LIST(FE_field))(&fe_field_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_create_data_point_at_coord.  No field to define");
					return_code=0;
				}
			}
		}
		else
		{
			std::cout << "ERROR: Cant merge node to region" << std::endl; 
			DEACCESS(Cmiss_node)(&node);
		}
	}
	
	std::cout << "ERROR: Can't Create node" << std::endl; 
	return 0;
}

struct Viewer_frame_element_constraint_function_data
{
	struct FE_element *element, *found_element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *coordinate_field;
}; 

static int Viewer_frame_element_constraint_function(FE_value *point,
	void *void_data)
/*******************************************************************************
LAST MODIFIED : 14 February 2008

DESCRIPTION : need to find the point of intersection between picking ray and obj
==============================================================================*/
{
	int return_code;
	struct Viewer_frame_element_constraint_function_data *data;

	ENTER(Viewer_frame_element_constraint_function_data);
	if (point && (data = (struct Viewer_frame_element_constraint_function_data *)void_data))
	{
		data->found_element = data->element;
		return_code = Computed_field_find_element_xi(data->coordinate_field,
			point, /*number_of_values*/3, /*time ok to be 0 since slices don't move wrt time*/ 0.0, &(data->found_element), 
			data->xi, /*element_dimension*/2, 
			(struct Cmiss_region *)NULL, /*propagate_field*/0, /*find_nearest_location*/1);
		Computed_field_evaluate_in_element(data->coordinate_field,
			data->found_element, data->xi, /*time*/0.0, (struct FE_element *)NULL,
			point, (FE_value *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_element_constraint_function.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_element_constraint_function */

Cmiss_element_id Cmiss_get_ray_intersection_point(double x, double y, double* node_coordinates, Cmiss_field_id* field)
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
	
	GLdouble modelview_matrix[16], window_projection_matrix[16];

	Scene_viewer_get_modelview_matrix(CmguiManager::getInstance().getSceneViewer(), modelview_matrix);
	Scene_viewer_get_window_projection_matrix(CmguiManager::getInstance().getSceneViewer(), window_projection_matrix);
	
	
	double size_x = 7.0;//FIX
	double size_y = 7.0;
	
	struct Interaction_volume *interaction_volume = create_Interaction_volume_ray_frustum(
					modelview_matrix,window_projection_matrix,
					viewport_left,viewport_bottom,viewport_width,viewport_height,
					centre_x,centre_y,size_x,size_y);
	
	FE_element* nearest_element;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene_picked_object *scene_picked_object2;
	struct GT_element_group *gt_element_group_element;
	struct GT_element_settings *gt_element_settings_element;
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(
			CmguiManager::getInstance().getCmissCommandData());
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	struct Graphics_buffer* graphics_buffer = Scene_viewer_get_graphics_buffer(CmguiManager::getInstance().getSceneViewer());
	
	if (scene_picked_object_list=
		Scene_pick_objects(scene,interaction_volume,graphics_buffer))
	{
		nearest_element = (struct FE_element *)NULL;

		nearest_element=Scene_picked_object_list_get_nearest_element(
			scene_picked_object_list,(struct Cmiss_region *)NULL,
			/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
			/*select_lines_enabled*/0, &scene_picked_object2,
			&gt_element_group_element,&gt_element_settings_element);
		
		DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
	}
	
	/* Find the intersection of the element and the interaction volume */
	Computed_field* nearest_element_coordinate_field = (struct Computed_field *)NULL;
	if (nearest_element)
	{
		if (!(nearest_element_coordinate_field = 
				GT_element_settings_get_coordinate_field(gt_element_settings_element)))
		{
			nearest_element_coordinate_field = 
				GT_element_group_get_default_coordinate_field(
				gt_element_group_element);
		}

		//Test
		*field = nearest_element_coordinate_field;

//		double node_coordinates[3];

		Viewer_frame_element_constraint_function_data constraint_data;
		constraint_data.element = nearest_element;
		constraint_data.found_element = nearest_element;
		constraint_data.coordinate_field = nearest_element_coordinate_field;
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			constraint_data.xi[i] = 0.5;
		}
		return_code = Interaction_volume_get_placement_point(interaction_volume,
			node_coordinates, Viewer_frame_element_constraint_function,
			&constraint_data);
	}

	if (return_code)
	{
		return nearest_element;
	}
	else
	{
		return (Cmiss_element_id)0;
	}
//	return return_code;
}
