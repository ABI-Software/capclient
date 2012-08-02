

#include "zinc/utilities.h"

extern "C"
{
#include <zn/cmiss_status.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_field_constant.h>
#include <zn/cmiss_field_finite_element.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_node.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_field_matrix_operators.h>
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

void SetupRegionForContour(Cmiss_context_id cmissContext, const std::string& regionName)
{
    Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
    Cmiss_region_id child_region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
    dbg("Region(" + regionName + ") = " + cap::ToString(child_region));
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
        double identity[16] = {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0};
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

    Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(cmissContext, regionName);
    {
        Cmiss_graphic_id data_graphic = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_DATA_POINTS);
        Cmiss_graphic_set_coordinate_field(data_graphic, coordinates_rc);
        Cmiss_graphic_set_visibility_flag(data_graphic, 1);
        std::string data_command = "LOCAL glyph cylinder_solid general size \"0.2*1*1\" centre 0,0,0 material yellow";
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
//    Cmiss_field_assign_real(mx_field, cache, 16, transform);

    Cmiss_field_cache_destroy(&cache);
    Cmiss_field_destroy(&mx_field);
    Cmiss_field_module_end_change(field_module);

    Cmiss_field_module_destroy(&field_module);
    Cmiss_region_destroy(&root_region);
    Cmiss_region_destroy(&child_region);
}

void AddContourPoint(Cmiss_context_id cmissContext, const std::string& regionName, double x, double y)
{
    Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
    Cmiss_region_id child_region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
    Cmiss_field_module_id field_module = Cmiss_region_get_field_module(child_region);

    Cmiss_field_module_begin_change(field_module);
    Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
    Cmiss_field_id coordinate_contour_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates_contour");
    Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_data");
    Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodeset);
    Cmiss_node_template_define_field(node_template, coordinate_contour_field);
    Cmiss_node_id node = Cmiss_nodeset_create_node(nodeset, -1, node_template);
    double position_values[3] = {x, y, 0.0};
    Cmiss_field_cache_set_node(field_cache, node);
    Cmiss_field_assign_real(coordinate_contour_field, field_cache, 3, position_values);
    Cmiss_field_module_end_change(field_module);

    Cmiss_field_destroy(&coordinate_contour_field);
    Cmiss_nodeset_destroy(&nodeset);
    Cmiss_node_template_destroy(&node_template);
    Cmiss_field_cache_destroy(&field_cache);
    Cmiss_field_module_destroy(&field_module);
    Cmiss_region_destroy(&child_region);
    Cmiss_region_destroy(&root_region);
}

