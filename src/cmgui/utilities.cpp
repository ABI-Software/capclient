

#include "cmgui/utilities.h"

extern "C"
{
#include <zn/cmiss_status.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_module.h>
//#include <zn/cmiss_node.h>
}

#include "utils/debug.h"
#include "utils/misc.h"

void RepositionPlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, const cap::ImagePlane *plane)
{
    //dbg("RepositionPlaneElement - " + regionName + " " + cap::ToString(plane->blc) + " " + cap::ToString(plane->trc));
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_id coordinates_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	double node_coordinates[element_node_count][3] =
	{
		{plane->blc.x, plane->blc.y, plane->blc.z},
		{plane->brc.x, plane->brc.y, plane->brc.z},
		{plane->tlc.x, plane->tlc.y, plane->tlc.z},
		{plane->trc.x, plane->trc.y, plane->trc.z}
	};
	for (int i = 0; i < element_node_count; i++)
	{
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, i+1);
        Cmiss_field_cache_set_node(field_cache, node);
        Cmiss_field_assign_real(coordinates_field, field_cache, /*number_of_values*/3, node_coordinates[i]);
        Cmiss_node_destroy(&node);
	}

	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_destroy(&coordinates_field);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

