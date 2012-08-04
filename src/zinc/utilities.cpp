

#include "zinc/utilities.h"

extern "C"
{
#include <zn/cmiss_status.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_field_time.h>
#include <zn/cmiss_field_constant.h>
#include <zn/cmiss_field_finite_element.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_node.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_field_matrix_operators.h>
#include <zn/cmiss_field_arithmetic_operators.h>
#include <zn/cmiss_field_logical_operators.h>
#include <zn/cmiss_field_conditional.h>
//#include <zn/cmiss_node.h>
}

#include <zinc/extensions.h>
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

void SetupRegionForContour(Cmiss_context_id cmissContext, const std::string& regionName, const std::string& name)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id child_region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
//	dbg("Region(" + regionName + ") = " + cap::ToString(child_region));
	if (!child_region)
	{
		child_region = Cmiss_region_create_child(root_region, regionName.c_str());
	}
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(child_region);

	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_id coordinate_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates_contour");
	if (!coordinate_field)
	{
		coordinate_field = Cmiss_field_module_create_finite_element(field_module, /*number_of_components*/3);
		Cmiss_field_set_attribute_integer(coordinate_field, CMISS_FIELD_ATTRIBUTE_IS_COORDINATE, 1);
		Cmiss_field_set_name(coordinate_field, "coordinates_contour");
	}

	Cmiss_field_id mx_field = Cmiss_field_module_find_field_by_name(field_module, "contour_transform_mx");
	if (!mx_field)
	{
		double identity[16] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
		mx_field = Cmiss_field_module_create_constant(field_module, 16, identity);
		Cmiss_field_set_name(mx_field, "contour_transform_mx");
	}

	Cmiss_field_id coordinates_rc = Cmiss_field_module_find_field_by_name(field_module, "coordinates_rc");
	if (!coordinates_rc)
	{
		coordinates_rc = Cmiss_field_module_create_projection(field_module, coordinate_field, mx_field);
		Cmiss_field_set_attribute_integer(coordinates_rc, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
		Cmiss_field_set_name(coordinates_rc, "coordinates_rc");
	}

	Cmiss_time_keeper_id time_keeper = Cmiss_context_get_default_time_keeper(cmissContext);

	Cmiss_field_id time_value = Cmiss_field_module_find_field_by_name(field_module, "time_value");
	if (!time_value)
	{
		time_value = Cmiss_field_module_create_time_value(field_module, time_keeper);
		Cmiss_field_set_name(time_value, "time_value");
		double values[] = {-1e-08};
		Cmiss_field_id const_zero = Cmiss_field_module_create_constant(field_module, 1, values);
		values[0] = 0.0;
		Cmiss_field_id visibility_control_constant = Cmiss_field_module_create_constant(field_module, 1, values);
		Cmiss_field_id visibility_value = Cmiss_field_module_create_finite_element(field_module, 1);
		Cmiss_field_set_name(visibility_value, "visibility_value_field");
		Cmiss_field_id diff = Cmiss_field_module_create_subtract(field_module, visibility_value, time_value);
		Cmiss_field_id abs = Cmiss_field_module_create_abs(field_module, diff);
		double err_values[] = {0.01};
		Cmiss_field_id err = Cmiss_field_module_create_constant(field_module, 1, err_values);
		Cmiss_field_id visibility_control_time = Cmiss_field_module_create_less_than(field_module, abs, err);
		Cmiss_field_id positive_time = Cmiss_field_module_create_greater_than(field_module, visibility_value, const_zero);
		Cmiss_field_set_name(positive_time, "positive_time");
		Cmiss_field_id if_field = Cmiss_field_module_create_if(field_module, positive_time, visibility_control_time, visibility_control_constant);
		Cmiss_field_set_name(if_field, "if_field");
		Cmiss_field_set_attribute_integer(if_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1.0);

		Cmiss_field_destroy(&if_field);
		Cmiss_field_destroy(&const_zero);
		Cmiss_field_destroy(&positive_time);
		Cmiss_field_destroy(&coordinate_field);
		Cmiss_field_destroy(&visibility_control_constant);
		Cmiss_field_destroy(&time_value);
		Cmiss_field_destroy(&diff);
		Cmiss_field_destroy(&abs);
		Cmiss_field_destroy(&err);
		Cmiss_field_destroy(&visibility_value);
		Cmiss_field_destroy(&visibility_control_time);
	}

	Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(cmissContext, regionName);
	{
//        Cmiss_rendition_find_graphic_by_name()
		Cmiss_graphic_id data_graphic = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_DATA_POINTS);
		Cmiss_graphic_set_coordinate_field(data_graphic, coordinates_rc);
		Cmiss_graphic_set_visibility_flag(data_graphic, 1);
		Cmiss_graphic_set_name(data_graphic, name.c_str());
//        Cmiss_graphic_set_glyph(data_graphic
		std::string data_command = "LOCAL glyph sphere general size \"2*2*2\" subgroup if_field centre 0,0,0 material yellow";
		Cmiss_graphic_define(data_graphic, data_command.c_str());
		Cmiss_graphic_destroy(&data_graphic);
	}
	Cmiss_rendition_destroy(&rendition);

	Cmiss_field_destroy(&coordinates_rc);
	Cmiss_field_destroy(&mx_field);
	Cmiss_field_destroy(&coordinate_field);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&child_region);
}

void SetContourTransform(Cmiss_context_id cmissContext, const std::string& regionName, const double *transform)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id child_region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(child_region);
	Cmiss_field_module_begin_change(field_module);

	Cmiss_field_id mx_field = Cmiss_field_module_find_field_by_name(field_module, "contour_transform_mx");
	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_assign_real(mx_field, cache, 16, transform);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_field_destroy(&mx_field);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&child_region);
}

void AddContourPoint(Cmiss_context_id cmissContext, const std::string& regionName, double time, double x, double y)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id child_region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(child_region);

	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_id coordinate_contour_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates_contour");
	Cmiss_field_id visibility_value_field = Cmiss_field_module_find_field_by_name(field_module, "visibility_value_field");
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_data");
	Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodeset);
	Cmiss_node_template_define_field(node_template, coordinate_contour_field);
	Cmiss_node_template_define_field(node_template, visibility_value_field);
	Cmiss_node_id node = Cmiss_nodeset_create_node(nodeset, -1, node_template);
	double position_values[3] = {x, y, 0.0};
	Cmiss_field_cache_set_time(field_cache, time);
	Cmiss_field_cache_set_node(field_cache, node);
	Cmiss_field_assign_real(coordinate_contour_field, field_cache, 3, position_values);
	double time_values[1] = {time};
	Cmiss_field_assign_real(visibility_value_field, field_cache, 1, time_values);
	Cmiss_field_module_end_change(field_module);

	Cmiss_node_destroy(&node);
	Cmiss_field_destroy(&visibility_value_field);
	Cmiss_field_destroy(&coordinate_contour_field);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_node_template_destroy(&node_template);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&child_region);
	Cmiss_region_destroy(&root_region);
}

void RemoveContourFromRegion(Cmiss_context_id cmissContext, const std::string& regionName)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id child_region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(child_region);

	Cmiss_field_module_begin_change(field_module);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_data");
	Cmiss_nodeset_group_id nodeset_group = Cmiss_nodeset_cast_group(nodeset);
	Cmiss_nodeset_group_remove_all_nodes(nodeset_group);

	Cmiss_field_id if_field = Cmiss_field_module_find_field_by_name(field_module, "if_field");
	Cmiss_field_set_attribute_integer(if_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 0.0);

	Cmiss_field_destroy(&if_field);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_nodeset_group_destroy(&nodeset_group);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&child_region);
	Cmiss_region_destroy(&root_region);
}

void SetLabelStateField(Cmiss_context_id cmissContext, std::string regionName, bool value)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext, regionName.c_str());
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_id label_state = Cmiss_field_module_find_field_by_name(field_module, "label_state");
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	double state[] = {0.0};
	if (value)
		state[0] = 1.0;

	Cmiss_field_assign_real(label_state, field_cache, 1, state);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_destroy(&label_state);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_destroy(&field_module);
}
