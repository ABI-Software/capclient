/*
 * CmguiExtensions.h
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#ifndef CMGUIEXTENSIONS_H_
#define CMGUIEXTENSIONS_H_

class wxPanel;
//#define FE_VALUE_IS_DOUBLE

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "time/time_keeper.h"
#include "api/cmiss_element.h"
}

int Cmiss_region_read_file_with_time(struct Cmiss_region *region, char *file_name, 
		struct Time_keeper* time_keeper, float time);

Cmiss_node_id Cmiss_create_data_point_at_coord(struct Cmiss_region *region, Cmiss_field_id field, double* coords, float time);

Cmiss_element_id Cmiss_get_ray_intersection_point(double x, double y, double* coords, Cmiss_field_id* field);

namespace cap
{
	class Point3D;
	class DataPoint;
}

Cmiss_node_id Cmiss_create_or_select_node_from_screen_coords(double x, double y, float time, cap::Point3D& coords);

Cmiss_node_id Cmiss_select_node_from_screen_coords(double x, double y, float time, cap::Point3D& coords);

int Cmiss_move_node_to_screen_coords(Cmiss_node_id node, double x, double y, float time, cap::Point3D& coords);

Cmiss_node_id Cmiss_node_set_visibility_field(cap::DataPoint& dataPoint, float startTime, float endTime, bool visibility);

Cmiss_region_id Cmiss_get_slice_region(double x, double y, double* node_coordinates, Cmiss_region_id region);
#endif /* CMGUIEXTENSIONS_H_ */
