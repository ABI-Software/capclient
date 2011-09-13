/*
 * CmguiExtensions.h
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#ifndef CMGUIEXTENSIONS_H_
#define CMGUIEXTENSIONS_H_

//class wxPanel;
//#define FE_VALUE_IS_DOUBLE

extern "C" {
#include "api/cmiss_region.h"
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_time_keeper.h"
#include "api/cmiss_element.h"
}

int Cmiss_region_read_file_with_time(struct Cmiss_region *region, char *file_name, 
									 Cmiss_time_keeper_id time_keeper, double time);

Cmiss_node_id Cmiss_create_data_point_at_coord(struct Cmiss_region *region, Cmiss_field_id field, double* coords, double time);

Cmiss_element_id Cmiss_get_ray_intersection_point(Cmiss_scene_viewer_id scene_viewer, double x, double y, double* coords, Cmiss_field_id* field);

namespace cap
{
	class Point3D;
}

Cmiss_node_id Cmiss_create_or_select_node_from_screen_coords(Cmiss_scene_viewer_id scene_viewer, double x, double y, double time, cap::Point3D& coords);

Cmiss_node_id Cmiss_select_node_from_screen_coords(Cmiss_scene_viewer_id scene_viewer, double x, double y, double time, cap::Point3D& coords);

int Cmiss_move_node_to_screen_coords(Cmiss_scene_viewer_id scene_viewer, Cmiss_node_id node, double x, double y, double time, cap::Point3D& coords);

Cmiss_node_id Cmiss_node_set_visibility_field(Cmiss_node_id node, double startTime, double endTime, bool visibility);

Cmiss_region_id Cmiss_get_slice_region(Cmiss_scene_viewer_id scene_viewer, double x, double y, double* node_coordinates, Cmiss_region_id region);

int Material_set_material_program_strings(struct Graphical_material *material_to_be_modified,
	char *vertex_program_string, char *fragment_program_string);//not defined in material.h

int Cmiss_region_modify_g_element(struct Cmiss_region *region,
	struct Scene *scene, struct GT_element_settings *settings,
	int delete_flag, int position);  // not exposed by cmgui

#endif /* CMGUIEXTENSIONS_H_ */

