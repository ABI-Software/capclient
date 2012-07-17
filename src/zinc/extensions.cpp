
#include "utils/misc.h"
#include "zinc/extensions.h"

#include "utils/filesystem.h"
#include "utils/debug.h"

#include <sstream>

extern "C"
{
#include <zn/zinc_configure.h>
#include <zn/cmiss_status.h>
#include <zn/cmiss_core.h>
#include <zn/cmiss_stream.h>
#include <zn/cmiss_field_image.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_time.h>
#include <zn/cmiss_field_group.h>
#include <zn/cmiss_field_finite_element.h>
#include <zn/cmiss_node.h>
#include <zn/cmiss_element.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_graphics_filter.h>
#include <zn/cmiss_scene.h>
#include <zn/cmiss_graphics_material.h>
#include <zn/cmiss_field_arithmetic_operators.h>
#include <zn/cmiss_field_logical_operators.h>
#include <zn/cmiss_field_composite.h>
#include <zn/cmiss_field_conditional.h>
}

class wxPanel;

void Cmiss_region_list_children(Cmiss_region_id region)
{
	Cmiss_region_id curr = Cmiss_region_get_first_child(region);
	dbgn("Region list : [");
	while (curr)
	{
		char *name = Cmiss_region_get_name(curr);
		dbgn(name);
		Cmiss_deallocate(name);
		Cmiss_region_id new_curr = Cmiss_region_get_next_sibling(curr);
		Cmiss_region_destroy(&curr);
		curr = new_curr;
		if (curr)
			dbgn(", ");
	}
	dbg("]");
	//Cmiss_region_destroy(&region);
}

Cmiss_scene_viewer_id Cmiss_context_create_scene_viewer(Cmiss_context_id cmissContext,  const std::string& sceneName, wxPanel* panel)
{
	Cmiss_scene_viewer_package_id package = Cmiss_context_get_default_scene_viewer_package(cmissContext);
	Cmiss_scene_viewer_id sceneViewer = Cmiss_scene_viewer_create_wx(package, panel, CMISS_SCENE_VIEWER_BUFFERING_DOUBLE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE, 8, 8, 8);
	
	std::string sceneTitle = sceneName;
	if (sceneTitle.empty())
	{
		sceneTitle = std::string("default");
	}
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext);
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_graphics_module_enable_renditions(graphics_module, root_region);
	Cmiss_graphics_filter_id graphics_filter = Cmiss_graphics_module_create_filter_visibility_flags(graphics_module);
	Cmiss_scene_id scene = Cmiss_graphics_module_create_scene(graphics_module);
	Cmiss_scene_set_filter(scene, graphics_filter);
	Cmiss_scene_set_region(scene, root_region);
	Cmiss_scene_set_name(scene, sceneTitle.c_str());
	Cmiss_scene_viewer_set_scene(sceneViewer, scene);
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1);
	
	Cmiss_region_destroy(&root_region);
	Cmiss_graphics_filter_destroy(&graphics_filter);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_scene_destroy(&scene);
	//Cmiss_scene_viewer_package_destroy(&package); // scene viewer package does not need destroying
	
	return sceneViewer;
}

Cmiss_field_image_id Cmiss_field_module_create_image_texture(Cmiss_field_module_id field_module, const std::string& filename)
{
	Cmiss_field_id temp_field = Cmiss_field_module_create_image(field_module, 0, 0);
	std::string name = "tex_" + cap::GetFileNameWOE(filename);
	Cmiss_field_set_name(temp_field, name.c_str());
	Cmiss_field_image_id field_image = Cmiss_field_cast_image(temp_field);
	Cmiss_field_destroy(&temp_field);
	Cmiss_stream_information_id stream_information = Cmiss_field_image_create_stream_information(field_image);
	
	/* Read image data from a file */
	Cmiss_stream_resource_id stream = Cmiss_stream_information_create_resource_file(stream_information, filename.c_str());
	int r = Cmiss_field_image_read(field_image, stream_information);
	if (r == CMISS_OK)
	{
		Cmiss_field_image_set_filter_mode(field_image, CMISS_FIELD_IMAGE_FILTER_LINEAR);
		
		Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS, 1/*dicom_image->GetImageWidthMm()*/);
		Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS, 1/*dicom_image->GetImageHeightMm()*/);
		Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS, 1);
	}
	else
	{
		dbg("Cmiss_field_module_create_image_texture failed to read image from stream : '" + filename + "'");
		Cmiss_field_image_destroy(&field_image);
	}
	
	Cmiss_stream_resource_destroy(&stream);
	Cmiss_stream_information_destroy(&stream_information);
	
	return field_image;
}

Cmiss_field_module_id Cmiss_context_get_field_module_for_region(Cmiss_context_id cmissContext, const std::string& regionName)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	Cmiss_field_module_id field_module = 0;
	if (region != 0)
		field_module = Cmiss_region_get_field_module(region);

	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
	
	return field_module;
}

Cmiss_rendition_id Cmiss_context_get_rendition_for_region(Cmiss_context_id cmissContext, const std::string& regionName)
{
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext);
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
    Cmiss_rendition_id  rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);

	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&region);
	
	return rendition;
}

int Cmiss_context_create_region_with_nodes(Cmiss_context_id cmissContext, std::string regionName)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
	int r = 1;
	if (region == 0)
	{
		region = Cmiss_region_create_child(root_region, regionName.c_str());
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
		Cmiss_field_id coordinate_field = Cmiss_field_module_create_finite_element(field_module, 3);
		Cmiss_field_set_name(coordinate_field, "coordinates");
		double values[] = {-1e-08};
		Cmiss_field_id const_zero = Cmiss_field_module_create_constant(field_module, 1, values);
		values[0] = 0.0;
		Cmiss_field_id visibility_control_constant = Cmiss_field_module_create_constant(field_module, 1, values);
		Cmiss_field_set_name(visibility_control_constant, "visibility_control_constant_field");
		Cmiss_time_keeper_id time_keeper = Cmiss_context_get_default_time_keeper(cmissContext);
		Cmiss_field_id time_value = Cmiss_field_module_create_time_value(field_module, time_keeper);
		Cmiss_field_id visibility_value = Cmiss_field_module_create_finite_element(field_module, 1);
		r = Cmiss_field_set_name(visibility_value, "visibility_value_field");
		Cmiss_field_id diff = Cmiss_field_module_create_subtract(field_module, visibility_value, time_value);
		Cmiss_field_id abs = Cmiss_field_module_create_abs(field_module, diff);
		double err_values[] = {0.01};
		Cmiss_field_id err = Cmiss_field_module_create_constant(field_module, 1, err_values);
		Cmiss_field_id visibility_control_time = Cmiss_field_module_create_less_than(field_module, abs, err);
		//r = Cmiss_field_module_define_field(field_module, "visibility_control_field", "constant 1");
		Cmiss_field_id positive_time = Cmiss_field_module_create_greater_than(field_module, visibility_value, const_zero);
		Cmiss_field_set_name(positive_time, "positive_time");
		Cmiss_field_id if_field = Cmiss_field_module_create_if(field_module, positive_time, visibility_control_time, visibility_control_constant);
		Cmiss_field_set_name(if_field, "if_field");
		std::string label = regionName + "_label";
		std::string label_command = "string_constant \"  " + regionName + "\"";
		r = Cmiss_field_module_define_field(field_module, label.c_str(), label_command.c_str());
		//r = Cmiss_field_module_define_field(field_module, "invisible_control_field", "constant 0");
		Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(cmissContext, regionName);
		{
			Cmiss_graphic_id node_graphic = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_NODE_POINTS);
			Cmiss_graphic_set_coordinate_field(node_graphic, coordinate_field);
			std::string material = "default";
			if (regionName == "APEX" || regionName == "BASE")
				material = "light_blue";
			else if (regionName == "RV")
				material = "orange";
			else if (regionName == "BASEPLANE")
				material = "pink";

			//std::string node_command = "gfx modify g_element " + regionName + " node_points coordinate coordinates LOCAL glyph sphere general size \"10*10*10\" visibility visibility_control_field centre 0,0,0 font default select_on material " + material + " selected_material " + material + "_sel label " + label + ";";
            std::string node_command = "LOCAL glyph sphere general size \"6*6*6\" subgroup if_field centre 0,0,0 font node_label_font select_on material " + material + " selected_material " + material + "_selected label " + label;
			Cmiss_graphic_define(node_graphic, node_command.c_str());
			Cmiss_graphic_destroy(&node_graphic);
		}

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

		Cmiss_time_keeper_destroy(&time_keeper);
		Cmiss_rendition_destroy(&rendition);
		Cmiss_field_module_destroy(&field_module);
	}

	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&region);

	return r;
}

void Cmiss_graphics_material_set_properties(Cmiss_graphics_material_id mat, std::string name, double ambient[3], double diffuse[3], double emission[3], double specular[3], double shininess, double alpha)
{
	Cmiss_graphics_material_set_name(mat, name.c_str());
	Cmiss_graphics_material_set_attribute_real3(mat, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_AMBIENT, ambient);
	Cmiss_graphics_material_set_attribute_real3(mat, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_DIFFUSE, diffuse);
	Cmiss_graphics_material_set_attribute_real3(mat, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_EMISSION, emission);
	Cmiss_graphics_material_set_attribute_real3(mat, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SPECULAR, specular);
	Cmiss_graphics_material_set_attribute_real(mat, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_SHININESS, shininess);
	Cmiss_graphics_material_set_attribute_real(mat, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_ALPHA, alpha);
}

Cmiss_field_module_id Cmiss_context_get_first_non_empty_selection_field_module(Cmiss_context_id cmissContext)
{
	Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(cmissContext, "/");
	Cmiss_field_group_id group_field = Cmiss_rendition_get_selection_group(rendition);
	Cmiss_field_group_id selected_group = Cmiss_field_group_get_first_non_empty_group(group_field);
	Cmiss_field_id selected_field = Cmiss_field_group_base_cast(selected_group); // non accessed handle
	Cmiss_field_module_id field_module = Cmiss_field_get_field_module(selected_field);

	Cmiss_field_group_destroy(&group_field);
	Cmiss_field_group_destroy(&selected_group);
	Cmiss_rendition_destroy(&rendition);

	return field_module;
}

Cmiss_node_id Cmiss_field_module_get_first_selected_node(Cmiss_field_module_id field_module)
{
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_selection.cmiss_nodes");

	Cmiss_node_iterator_id it = Cmiss_nodeset_create_node_iterator(nodeset);
	Cmiss_node_id selected_node = Cmiss_node_iterator_next(it);

	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_node_iterator_destroy(&it);

	return selected_node;
}

void CreateTextureImageSurface(Cmiss_context_id cmissContext, const std::string& regionName, Cmiss_graphics_material_id material)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	//Got to find the child region first!!
	Cmiss_region_id region = 0;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str())))
	{
		//error
		dbg("Cmiss_region_find_subregion_at_path() returned 0 : " + regionName);
		return;
	}

	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_id coordinates = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_field_id xi = Cmiss_field_module_find_field_by_name(field_module, "xi");
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext);
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
	Cmiss_graphic_id surface = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_SURFACES);

	Cmiss_graphic_set_coordinate_field(surface, coordinates);
	Cmiss_graphic_set_material(surface, material);
	Cmiss_graphic_set_selected_material(surface, material);
	Cmiss_graphic_set_texture_coordinate_field(surface, xi);
	
	Cmiss_graphic_destroy(&surface);
	Cmiss_field_destroy(&coordinates);
	Cmiss_field_destroy(&xi);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_rendition_destroy(&rendition);
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

Cmiss_node_id Cmiss_context_create_node(Cmiss_context_id cmissContext)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(root_region);
	Cmiss_field_module_begin_change(field_module);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	Cmiss_node_template_id node_template1 = Cmiss_nodeset_create_node_template(nodeset);
	Cmiss_node_id node = Cmiss_nodeset_create_node(nodeset, -1, node_template1);
	Cmiss_field_module_end_change(field_module);
	
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_node_template_destroy(&node_template1);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&root_region);

	return node;
}

Cmiss_node_id Cmiss_region_create_node(Cmiss_region_id region, double x, double y, double z)
{
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_module_begin_change(field_module);
    Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
    Cmiss_field_id coordinate_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_field_id visibility_value_field = Cmiss_field_module_find_field_by_name(field_module, "visibility_value_field");
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodeset);
	Cmiss_node_template_define_field(node_template, coordinate_field);
	Cmiss_node_template_define_field(node_template, visibility_value_field);
	Cmiss_node_id node = Cmiss_nodeset_create_node(nodeset, -1, node_template);
    double position_values[3] = {x, y, z};
    Cmiss_field_cache_set_node(field_cache, node);
    Cmiss_field_assign_real(coordinate_field, field_cache, 3, position_values);
    Cmiss_field_module_end_change(field_module);
	
	Cmiss_field_destroy(&coordinate_field);
	Cmiss_field_destroy(&visibility_value_field);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_node_template_destroy(&node_template);
    Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_module_destroy(&field_module);

	return node;
}

void CreatePlaneElement(Cmiss_context_id cmissContext, const std::string& regionName)
{
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_create_child(root_region, regionName.c_str());
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_id coordinates_field = Cmiss_field_module_create_finite_element(field_module, /*number_of_components*/3);
	Cmiss_field_set_name(coordinates_field, "coordinates");
	Cmiss_field_set_attribute_integer(coordinates_field, CMISS_FIELD_ATTRIBUTE_IS_COORDINATE, 1);
	Cmiss_field_set_attribute_integer(coordinates_field, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	Cmiss_node_template_id node_template = Cmiss_nodeset_create_node_template(nodeset);
	Cmiss_node_template_define_field(node_template, coordinates_field);
	double node_coordinates[element_node_count][3] =
	{
		{ 0, 0, 0 },
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 1, 1, 0 }
	};
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	for (int i = 0; i < element_node_count; i++)
	{
		Cmiss_node_id node = Cmiss_nodeset_create_node(nodeset, i+1, node_template);
        Cmiss_field_cache_set_node(field_cache, node);
        Cmiss_field_assign_real(coordinates_field, field_cache, /*number_of_values*/3, node_coordinates[i]);
        Cmiss_node_destroy(&node);
	}
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_node_template_destroy(&node_template);
	
	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, /*dimension*/2);
	Cmiss_element_template_id element_template = Cmiss_mesh_create_element_template(mesh);
	Cmiss_element_template_set_shape_type(element_template, CMISS_ELEMENT_SHAPE_SQUARE);
	Cmiss_element_template_set_number_of_nodes(element_template, element_node_count);
	Cmiss_element_basis_id cubic_basis = Cmiss_field_module_create_element_basis(field_module, 2
		, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE);
	const int cube_local_node_indexes[element_node_count] = { 1, 2, 3, 4};
	Cmiss_element_template_define_field_simple_nodal(element_template, coordinates_field,
		 /*component_number*/-1, cubic_basis, element_node_count, cube_local_node_indexes);
	Cmiss_element_basis_destroy(&cubic_basis);
	
	/* create element */
	for (int i = 1; i <= element_node_count; i++)
	{
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, i);
		Cmiss_element_template_set_node(element_template, i, node);
		Cmiss_node_destroy(&node);
	}
	Cmiss_mesh_define_element(mesh, -1, element_template);
	Cmiss_element_template_destroy(&element_template);
	Cmiss_field_module_end_change(field_module);
	
	Cmiss_field_module_destroy(&field_module);
	Cmiss_mesh_destroy(&mesh);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_destroy(&coordinates_field);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&region);
}

void ResizePlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, double width, double height)
{
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_id coordinates_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	double node_coordinates[element_node_count][3] =
	{
		{ 0, 0, 0 },
		{ width, 0, 0 },
		{ 0, height, 0 },
		{ width, height, 0 }
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

std::string GetNextNameInSeries(Cmiss_field_module_id field_module, std::string name)
{
	int i = 0;
	std::string field_name;
	Cmiss_field_id field = 0;
	do
	{
		Cmiss_field_destroy(&field);
		field_name = name;
		std::ostringstream o;
		o << ++i;
		field_name.append(o.str());
		field = Cmiss_field_module_find_field_by_name(field_module, field_name.c_str());
	} while (field != 0);
	Cmiss_field_destroy(&field);
	
	return field_name;
}

void SetVisibilityForGraphicsInRegion(Cmiss_context_id cmissContext, const std::string& regionName, bool visibility)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext);
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
	Cmiss_graphic_id graphic = Cmiss_rendition_get_first_graphic(rendition);
	while (graphic)
	{
		Cmiss_graphic_set_visibility_flag(graphic, visibility ? 1 : 0);
		Cmiss_graphic_id new_graphic = Cmiss_rendition_get_next_graphic(rendition, graphic);
		Cmiss_graphic_destroy(&graphic);
		graphic = new_graphic;
	}

	Cmiss_rendition_destroy(&rendition);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}


