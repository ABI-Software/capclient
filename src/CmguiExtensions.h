/*
 * CmguiExtensions.h
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#ifndef CMGUIEXTENSIONS_H_
#define CMGUIEXTENSIONS_H_

class wxPanel;

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "time/time_keeper.h"
#include "api/cmiss_element.h"
}

#ifdef DEL
// should go to cmiss.h
struct Material_package* Cmiss_command_data_get_material_package(
	struct Cmiss_command_data *command_data
);
#endif

Cmiss_scene_viewer_id create_Cmiss_scene_viewer_wx(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	wxPanel* panel,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
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

#ifdef LATER
int Viewer_view_all(Cmiss_scene_viewer_id *scene_viewer); //can modify to have diff FOV
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
#endif 

int Cmiss_region_read_file_with_time(struct Cmiss_region *region, char *file_name, 
		struct Time_keeper* time_keeper, float time);
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
==============================================================================*/

Cmiss_node_id Cmiss_create_node_at_coord(struct Cmiss_region *region, Cmiss_field_id field, float* coords);

Cmiss_node_id Cmiss_create_data_point_at_coord(struct Cmiss_region *region, Cmiss_field_id field, float* coords, float time);

Cmiss_element_id Cmiss_get_ray_intersection_point(double x, double y, double* coords, Cmiss_field_id* field);

#endif /* CMGUIEXTENSIONS_H_ */
