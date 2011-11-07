
#include <sstream>

extern "C"
{
#include <configure/cmgui_configure.h>
#include <api/cmiss_stream.h>
#include <api/cmiss_field_image.h>
#include <api/cmiss_context.h>
#include <api/cmiss_region.h>
#include <api/cmiss_field_module.h>
#include <api/cmiss_graphic.h>
#include <api/cmiss_rendition.h>
#include <api/cmiss_field.h>
#include <api/cmiss_field_finite_element.h>
#include <api/cmiss_node.h>
#include <api/cmiss_element.h>
#include <api/cmiss_field_module.h>
#include <api/cmiss_graphics_filter.h>
#include <api/cmiss_scene.h>
}

#include "DICOMImage.h"
#include "filesystem.h"

#include "utilities.h"
#include "utils/debug.h"

class wxPanel;

Cmiss_scene_viewer_id Cmiss_context_create_scene_viewer(Cmiss_context_id cmissContext,  const std::string& sceneName, wxPanel* panel)
{
	assert(panel);
	Cmiss_scene_viewer_package_id package = Cmiss_context_get_default_scene_viewer_package(cmissContext);
	Cmiss_scene_viewer_id sceneViewer = Cmiss_scene_viewer_create_wx(package, panel, CMISS_SCENE_VIEWER_BUFFERING_DOUBLE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE, 8, 8, 8);
	assert(sceneViewer);
	
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
	//Cmiss_scene_viewer_package_destroy(&package); // identifier not found
	
	return sceneViewer;
}

Cmiss_field_image_id Cmiss_field_module_create_image_texture(Cmiss_field_module_id field_module, const cap::DICOMPtr& dicom_image)
{
	//std::cout << "CmguiPanel::" << __func__ << std::endl;
	Cmiss_field_id temp_field = Cmiss_field_module_create_image(field_module, 0, 0);
	std::string name = "tex_" + cap::FileSystem::GetFileNameWOE(dicom_image->GetFilename()); //-- GetNextNameInSeries(field_module, "tex_");
	Cmiss_field_set_name(temp_field, name.c_str());
	Cmiss_field_image_id field_image = Cmiss_field_cast_image(temp_field);
	Cmiss_field_destroy(&temp_field);
	Cmiss_stream_information_id stream_information = Cmiss_field_image_create_stream_information(field_image);
	Cmiss_stream_information_region_id stream_information_region = Cmiss_stream_information_cast_region(stream_information);
	Cmiss_stream_information_image_id image_stream_information = Cmiss_stream_information_cast_image(stream_information);
	
	/* Read image data from a file */
	Cmiss_stream_resource_id stream = Cmiss_stream_information_create_resource_file(stream_information, dicom_image->GetFilename().c_str());
	Cmiss_stream_information_region_set_attribute_real(stream_information_region, CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, 0.0);
	//Cmiss_field_image_set_filter_mode(field_image,	CMISS_FIELD_IMAGE_FILTER_LINEAR);
	Cmiss_field_image_read(field_image, stream_information);
	
	Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS, 1/*dicom_image->GetImageWidthMm()*/);
	Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS, 1/*dicom_image->GetImageHeightMm()*/);
	Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS, 1);
	
	Cmiss_stream_resource_destroy(&stream);
	Cmiss_stream_information_image_destroy(&image_stream_information);
	Cmiss_stream_information_destroy(&stream_information);
	Cmiss_stream_information_region_destroy(&stream_information_region);
	
	return field_image;
}

Cmiss_field_module_id Cmiss_context_get_field_module_for_region(Cmiss_context_id cmissContext, const std::string& regionName)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&region);
	
	return field_module;
}

Cmiss_rendition_id Cmiss_context_get_rendition_for_region(Cmiss_context_id cmissContext, const std::string& regionName)
{
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext);
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);

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
		r = Cmiss_field_module_define_field(field_module, "coordinates", "finite_element num 3 coordinate");
		std::string clear_command = "gfx modify g_element " + regionName + " general clear;";
		std::string node_command = "gfx modify g_element " + regionName + " node_points coordinate coordinates LOCAL glyph sphere general size \"10*10*10\" centre 0,0,0 font default select_on material default selected_material default_selected;";
		if (r)
		{
			r = Cmiss_context_execute_command(cmissContext, clear_command.c_str());
		}
		if (r)
		{
			r = Cmiss_context_execute_command(cmissContext, node_command.c_str());
		}
		Cmiss_field_module_destroy(&field_module);
	}

	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&region);

	return r;
}

void CreateTextureImageSurface(Cmiss_context_id cmissContext, const std::string& regionName, Cmiss_graphics_material_id material)
{
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext);
	//Got to find the child region first!!
	dbg("Subregion name = " + regionName);
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
	//Cmiss_graphics_material_id sel_material = Cmiss_graphics_module_create_material(graphics_module);
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
	Cmiss_node_template_id node_template1 = Cmiss_nodeset_create_node_template(nodeset);
	Cmiss_node_template_define_field(node_template1, coordinates_field);
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
		Cmiss_node_id node = Cmiss_nodeset_create_node(nodeset, i+1, node_template1);
		Cmiss_field_cache_set_node(field_cache, node);
		Cmiss_field_assign_real(coordinates_field, field_cache, /*number_of_values*/3, node_coordinates[i]);
		Cmiss_node_destroy(&node);
	}
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_node_template_destroy(&node_template1);
	
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
	//std::cout << "Cmiss_element : " << result << std::endl;
	
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
	
	Cmiss_mesh_destroy(&mesh);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_destroy(&coordinates_field);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&region);
}

void ResizePlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, double width, double height)
{
	//std::cout << "CmguiPanel::ResizePlaneElement - " << regionName << " " << width << " " << height << std::endl;
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	assert(region);
	
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
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

void RepositionPlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, const cap::ImagePlane *plane)
{
	//std::cout << "CmguiPanel::ResizePlaneElement - " << regionName << " " << width << " " << height << std::endl;
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	assert(region);
	
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_id coordinates_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	double width = sqrt(DotProduct(plane->xside, plane->xside));
	double height = sqrt(DotProduct(plane->yside, plane->yside));
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
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

void RepositionPlaneElement(Cmiss_context_id cmissContext, const std::string& regionName, Real *tlc, Real *trc, Real *brc, Real *blc)
{
	//std::cout << "CmguiPanel::ResizePlaneElement - " << regionName << " " << width << " " << height << std::endl;
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	assert(region);
	
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_id coordinates_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	double node_coordinates[element_node_count][3] =
	{
		{blc[0], blc[1], blc[2]},
		{brc[0], brc[1], brc[2]},
		{tlc[0], tlc[1], tlc[2]},
		{trc[0], trc[1], trc[2]}
	};
	for (int i = 0; i < element_node_count; i++)
	{
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, i+1);
		Cmiss_field_cache_set_node(field_cache, node);
		Cmiss_field_assign_real(coordinates_field, field_cache, /*number_of_values*/3, node_coordinates[i]);
		Cmiss_node_destroy(&node);
	}
	Cmiss_field_cache_destroy(&field_cache);
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

void SetVisibilityForRegion(Cmiss_context_id cmissContext, const std::string& regionName, bool visibility)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext);
	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
	assert(region);
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext);
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
	if (visibility)
		Cmiss_rendition_set_visibility_flag(rendition, 1);
	else
		Cmiss_rendition_set_visibility_flag(rendition, 0);
}


