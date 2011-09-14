/*
 * CmguiPanel.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

#include <wx/app.h>

extern "C" {
#include <configure/cmgui_configure.h>
#include <api/cmiss_field_image.h>
#include <api/cmiss_scene_viewer.h>
#include <api/cmiss_scene.h>
#include <api/cmiss_stream.h>
#include <api/cmiss_graphics_filter.h>
#include <api/cmiss_interactive_tool.h>
#include <api/cmiss_rendition.h>
#include <api/cmiss_graphic.h>
#include <api/cmiss_field_module.h>
#include <api/cmiss_field.h>
#include <api/cmiss_node.h>
#include <api/cmiss_field_finite_element.h>
}

#include "cmguipanel.h"
#include "CmguiExtensions.h"
#include "CAPMaterial.h"

namespace cap
{
CmguiPanel::CmguiPanel(const std::string& name, wxPanel* panel)
	: cmissContext_(0)
	, cmissSceneViewer_(0)
{
	assert(panel);
	cmissContext_ = Cmiss_context_create(name.c_str());
	int result = Cmiss_context_enable_user_interface(cmissContext_, 0, 0, static_cast<void*>(wxTheApp));
	cmissSceneViewer_ = CreateSceneViewer(name, panel);
	assert(result == 1);
}
	
CmguiPanel::~CmguiPanel()
{
	std::cout << "CmguiPanel::~CmguiPanel()" << std::endl;
	Cmiss_context_destroy(&cmissContext_);
}

void CmguiPanel::RedrawNow() const
{
	Cmiss_scene_viewer_redraw_now(cmissSceneViewer_);
}

void CmguiPanel::SetViewingVolume(double radius)
{
	const double view_angle = 40.0, width_factor = 1.05, clip_factor = 10.0;
	double eye_distance, near_plane, far_plane;
	
	assert(cmissSceneViewer_);
	radius *= width_factor;
	eye_distance = sqrt(2.0)*radius/tan(view_angle*3.141592/360.0);
	
	far_plane = eye_distance+clip_factor*radius;
	near_plane = eye_distance-clip_factor*radius;
	if (clip_factor*radius >= eye_distance)
		near_plane = 0.01*eye_distance;
	
	Cmiss_scene_viewer_set_viewing_volume(cmissSceneViewer_, -radius, radius, -radius, radius, near_plane, far_plane);
}

void CmguiPanel::SetTumbleRate(double speed)
{
	Cmiss_scene_viewer_set_tumble_rate(cmissSceneViewer_, speed);
}

void CmguiPanel::ViewAll() const
{
	Cmiss_scene_viewer_view_all(cmissSceneViewer_);
}

Cmiss_scene_viewer_id CmguiPanel::CreateSceneViewer(const std::string& sceneName, wxPanel* panel) const
{
	std::cout << "CmguiPanel::" << __func__ << std::endl;
	Cmiss_scene_viewer_package_id package = Cmiss_context_get_default_scene_viewer_package(cmissContext_);
	Cmiss_scene_viewer_id sceneViewer = Cmiss_scene_viewer_create_wx(package, panel, CMISS_SCENE_VIEWER_BUFFERING_DOUBLE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE, 8, 8, 8);
	assert(sceneViewer);
	
	if (!sceneName.empty())
	{
		Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext_);
		Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
		Cmiss_graphics_module_enable_renditions(graphics_module, root_region);
		Cmiss_graphics_filter_id graphics_filter = Cmiss_graphics_module_create_filter_visibility_flags(graphics_module);
		Cmiss_scene_id scene = Cmiss_graphics_module_create_scene(graphics_module);
		Cmiss_scene_set_filter(scene, graphics_filter);
		Cmiss_scene_set_region(scene, root_region);
		Cmiss_scene_set_name(scene, sceneName.c_str());
		Cmiss_scene_viewer_set_scene(sceneViewer, scene);
		Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1 );
		
		Cmiss_region_destroy(&root_region);
		Cmiss_graphics_filter_destroy(&graphics_filter);
		Cmiss_graphics_module_destroy(&graphics_module);
		Cmiss_scene_destroy(&scene);
	}
	
	Cmiss_scene_viewer_set_interactive_tool_by_name(sceneViewer, "transform_tool");
	Cmiss_interactive_tool_id itool = Cmiss_scene_viewer_get_current_interactive_tool(sceneViewer);
	Cmiss_interactive_tool_execute_command(itool, "");
//	struct Interactive_tool * intTool = Scene_viewer_get_interactive_tool(sceneViewer);
//	Interactive_tool_transform_set_free_spin(intTool, 0);

	//Cmiss_scene_viewer_package_destroy(&package);
	//Cmiss_scene_viewer_destroy(&sceneViewer);
	
	return sceneViewer;
}

void CmguiPanel::CreateCmissImageTexture(Cmiss_field_image_id field_image, const DICOMPtr& dicom_image) const
{
	//std::cout << "CmguiPanel::" << __func__ << std::endl;
	Cmiss_stream_information_id stream_information =
		Cmiss_field_image_create_stream_information(field_image);
	Cmiss_stream_information_image_id image_stream_information =
		Cmiss_stream_information_cast_image(stream_information);

	/* Read image data from a file */
	Cmiss_stream_resource_id stream = Cmiss_stream_information_create_resource_file(stream_information, dicom_image->GetFilename().c_str());
	//Cmiss_field_image_set_filter_mode(field_image,	CMISS_FIELD_IMAGE_FILTER_LINEAR);
	Cmiss_field_image_read(field_image, stream_information);
	
	Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_WIDTH_PIXELS, dicom_image->GetImageWidth());
	Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_HEIGHT_PIXELS, dicom_image->GetImageHeight());
	Cmiss_field_image_set_attribute_real(field_image, CMISS_FIELD_IMAGE_ATTRIBUTE_PHYSICAL_DEPTH_PIXELS, 1);
		
	Cmiss_stream_resource_destroy(&stream);
	Cmiss_stream_information_image_destroy(&image_stream_information);
	Cmiss_stream_information_destroy(&stream_information);
}

std::tr1::shared_ptr<CAPMaterial> CmguiPanel::CreateCAPMaterial(std::string const& materialName) const
{
	Cmiss_graphics_module_id gModule = Cmiss_context_get_default_graphics_module(cmissContext_);
	// boost::make_pair is faster than shared_ptr<CAPMaterial>(new )
	return boost::make_shared<CAPMaterial>(materialName, gModule);
}

Cmiss_field_module_id CmguiPanel::GetFieldModuleForRegion(const std::string& regionName)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region_id region = Cmiss_region_find_child_by_name(root_region, regionName.c_str());
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_region_destroy(&root_region);
	Cmiss_region_destroy(&region);
	
	return field_module;
}

void CmguiPanel::ReadRectangularModelFiles(std::string const& modelName, std::string const& sceneName) const
{
	std::cout << "CmguiPanel::" << __func__ << ": " << sceneName << std::endl;
	Cmiss_region_id region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_stream_information_id stream_information = Cmiss_region_create_stream_information(region);
	Cmiss_stream_information_region_id region_stream_information = Cmiss_stream_information_cast_region(stream_information);
	
#include "ImagePreviewExnode.h"
#include "ImagePreviewExelem.h"
	Cmiss_stream_resource_id stream[2];
	stream[0] = Cmiss_stream_information_create_resource_memory_buffer(stream_information, ImagePreview_exnode, ImagePreview_exnode_len);
	stream[1] = Cmiss_stream_information_create_resource_memory_buffer(stream_information, ImagePreview_exelem, ImagePreview_exelem_len);
	Cmiss_stream_information_region_set_resource_attribute_real(region_stream_information, stream[0], CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, 0.0);
	Cmiss_stream_information_region_set_resource_attribute_real(region_stream_information, stream[1], CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, 0.0);
	Cmiss_region_read(region, stream_information);
	
	// Read in ex files that define the element used to represent the image slice
	// TODO these should be done programatically
/*	char filename[256]; // FIX 256 chars may not be sufficient
	sprintf(filename, "%stemplates/%s.exnode", CAP_DATA_DIR, modelName.c_str()); 
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - " << modelName << ".exnode" << std::endl;
	}
	
	sprintf(filename, "%stemplates/%s.exelem", CAP_DATA_DIR, modelName.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - " << modelName << ".exelem" << std::endl;
	}*/
	
	// model file name needs to be the same as its subregion name!
	Cmiss_region_id subregion = Cmiss_region_find_subregion_at_path(region, modelName.c_str());
	assert(subregion);
	
	if (sceneName != "")
	{
		//This means the model is to be loaded into the specified scene.
		//Currently cmgui doesn't provide an easy way to specify the scene to be used
		//when readin a model - it always renders them in the default scene.
		//so we have to use a hack here
		//std::string gfx_command("gfx draw as " + modelName + " group " + modelName + " scene " + sceneName);
		//Cmiss_context_execute_command(cmissContext_, gfx_command.c_str());
		//DELETE the scene object from the default scene
		//Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(cmissContext_);
		//Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		//assert(scene);
//		if (Scene_object* scene_object =
//			Scene_get_Scene_object_by_name(scene, (char*)modelName.c_str()))
//		{
//			int ret = Scene_remove_Scene_object(scene, scene_object);//cmgui only allows this for manual mode
			// For now just make the scene object invisible in the default scene.
			// TODO: use a non-default scene for the CAPClientWindow or set the default scene to use the manual mode
//			int ret = Scene_object_set_visibility(scene_object, g_INVISIBLE);
//			assert(ret);
//		}
	}

	Cmiss_stream_resource_destroy(&stream[0]);
	Cmiss_stream_resource_destroy(&stream[1]);
	Cmiss_stream_information_destroy(&stream_information);
	Cmiss_stream_information_region_destroy(&region_stream_information);
	Cmiss_region_destroy(&subregion);
	Cmiss_region_destroy(&region);
}

void CmguiPanel::CreateTextureImageSurface(std::string const& regionName,
	Cmiss_graphics_material_id material) const
{
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	//Got to find the child region first!!
	std::cout << "Subregion name = " << regionName << std::endl;
	Cmiss_region_id region = 0;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region << std::endl;
		return;
	}
	
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_id coordinates = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_field_id xi = Cmiss_field_module_find_field_by_name(field_module, "xi");
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext_);
	Cmiss_graphics_material_id sel_material = Cmiss_graphics_module_create_material(graphics_module);
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
	Cmiss_graphic_id surface = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_SURFACES);
	//	Cmiss_graphics_mat
	//Cmiss_graphic_set_coordinate_system(surface, CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL);
	Cmiss_graphic_set_coordinate_field(surface, coordinates);	
	Cmiss_graphic_set_material(surface, material);
	Cmiss_graphic_set_selected_material(surface, sel_material);
	Cmiss_graphic_set_texture_coordinate_field(surface, coordinates);
	
	//Cmiss_graphic_destroy(&surface);
	Cmiss_field_destroy(&coordinates);
	Cmiss_field_destroy(&xi);
	Cmiss_field_module_destroy(&field_module);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_rendition_destroy(&rendition);
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

void CmguiPanel::CreatePlaneElement(const std::string& regionName)
{
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
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
	Cmiss_node_template_finalise(node_template1);
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
	Cmiss_element_basis_id cubic_basis = Cmiss_mesh_create_element_basis(
		mesh, CMISS_BASIS_FUNCTION_LINEAR_LAGRANGE);
	const int cube_local_node_indexes[element_node_count] = { 1, 2, 3, 4};
	Cmiss_element_template_define_field_simple_nodal(element_template, coordinates_field,
													 /*component_number*/-1, cubic_basis, element_node_count, cube_local_node_indexes);
	Cmiss_element_basis_destroy(&cubic_basis);
	int result = Cmiss_element_template_finalise(element_template);
	std::cout << "Cmiss_element : " << result << std::endl;
	
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

void CmguiPanel::ResizePlaneElement(const std::string& regionName, int width, int height)
{
	//std::cout << "CmguiPanel::ResizePlaneElement - " << regionName << " " << width << " " << height << std::endl;
	const int element_node_count = 4;
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
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

void CmguiPanel::AssignMaterialToObject(Cmiss_scene_viewer_id scene_viewer,
		Cmiss_graphics_material_id material, std::string const& regionName) const
{
	//using namespace std;
	
	std::cout << "CmguiPanel::AssignMaterialToObject" << std::endl;
	//Cmiss_scene_id scene = 0;
	//if (scene_viewer)
	//{
		//Cmiss_scene_viewer_get_scene_name(scene_viewer);
		//scene = Scene_viewer_get_scene(scene_viewer);
	//}
	//else // use default scene
	//{
		//Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(cmissContext_);
		//scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	//}
		
	//if (!scene)
	//{
	//	cout << "Can't find scene" << endl;
	//}

	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	//Got to find the child region first!!
	std::cout << "Subregion name = " << regionName << std::endl;
	Cmiss_region_id region = 0;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region << std::endl;
		return;
	}
	// Create a texture coordinate field
	Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	Cmiss_field_id field_slider_value = Cmiss_field_module_create_field(field_module, "slider_value", "constant 0");
	Cmiss_field_id field_texture_coords = Cmiss_field_module_create_field(field_module, "texture_coords", "composite coordinates_rect.x coordinates_rect.y slider_value");
	Cmiss_field_id coordinates = Cmiss_field_module_find_field_by_name(field_module, "coordinates_rect");
	Cmiss_field_id xi = Cmiss_field_module_find_field_by_name(field_module, "xi");
	

	// Create a surface for the material supplied and texture coordinate field created
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext_);
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, region);
	Cmiss_graphic_id surface = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_SURFACES);
//	Cmiss_graphics_mat
	Cmiss_graphic_set_coordinate_system(surface, CMISS_GRAPHICS_COORDINATE_SYSTEM_LOCAL);
	Cmiss_graphic_set_coordinate_field(surface, coordinates);	
	Cmiss_graphic_set_material(surface, material);
	Cmiss_graphic_set_texture_coordinate_field(surface, xi);
	Cmiss_graphic_set_selected_material(surface, material);
	Cmiss_rendition_execute_command(rendition, "point local glyph axes general size 1.2*1.2*1.2 centre 0,0,0 font default select_on material default selected_material default_selected");
	Cmiss_graphics_material_execute_command(material, "ambient 1 0.25 0 diffuse 1 0.25 0 alpha 1.0 shininess 0.8 per_pixel texture tex_0");
	Cmiss_scene_viewer_view_all(scene_viewer);
	
	Cmiss_field_destroy(&coordinates);
	Cmiss_field_destroy(&field_slider_value);
	Cmiss_field_destroy(&field_texture_coords);
	Cmiss_graphic_destroy(&surface);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_rendition_destroy(&rendition);
	//GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	// use the same material for selected material
	//GT_element_settings_set_selected_material(settings, material);

	//if(!GT_element_settings_set_material(settings, material))
	{
		//Error;
		//std::cout << __func__ << " :Error setting material\n";
	}
	//else
	{
		//manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
		//Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		//GT_element_settings_set_texture_coordinate_field(settings,c_field);

		//if (!Cmiss_region_modify_g_element(region, scene,settings,
		//	/*delete_flag*/0, /*position*/-1))
		//{
			 //error
			//std::cout << __func__ << " :Error modifying g element\n";
		//}
	}
	
	std::cout << "Leaving: CmguiPanel::AssignMaterialToObject" << std::endl;
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);

	// return sceneObject for convenience REVISE!!
	return;
}

std::string CmguiPanel::GetNextNameInSeries(Cmiss_field_module_id field_module, std::string name)
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

//void CmguiPanel::DestroyTexture(Cmiss_texture_id tex) const
//{
//	DESTROY(Texture)(&tex);
//}
} // end namespace cap
