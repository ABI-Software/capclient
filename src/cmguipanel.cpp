/*
 * CmguiPanel.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include <fstream>
#include <iostream>

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
CmguiPanel::CmguiPanel(Cmiss_context_id cmissContext, const std::string& name, wxPanel* panel)
	: cmissSceneViewer_(CreateSceneViewer(cmissContext, name, panel))
{
}
	
CmguiPanel::~CmguiPanel()
{
	std::cout << "CmguiPanel::~CmguiPanel()" << std::endl;
	Cmiss_scene_viewer_destroy(&cmissSceneViewer_);
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

Cmiss_scene_viewer_id CmguiPanel::CreateSceneViewer(Cmiss_context_id cmissContext,  const std::string& sceneName, wxPanel* panel) const
{
	std::cout << "CmguiPanel::" << __func__ << std::endl;
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
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1 );
	
	Cmiss_region_destroy(&root_region);
	Cmiss_graphics_filter_destroy(&graphics_filter);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_scene_destroy(&scene);
	
	Cmiss_scene_viewer_set_interactive_tool_by_name(sceneViewer, "transform_tool");
	Cmiss_interactive_tool_id itool = Cmiss_scene_viewer_get_current_interactive_tool(sceneViewer);
	Cmiss_interactive_tool_execute_command(itool, "");
//	struct Interactive_tool * intTool = Scene_viewer_get_interactive_tool(sceneViewer);
//	Interactive_tool_transform_set_free_spin(intTool, 0);

	//Cmiss_scene_viewer_package_destroy(&package);
	//Cmiss_scene_viewer_destroy(&sceneViewer);
	
	return sceneViewer;
}


/*
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

		//if (!Cmiss_region_modify_g_element(region, scene,settings, 0, -1))
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
}*/

//void CmguiPanel::DestroyTexture(Cmiss_texture_id tex) const
//{
//	DESTROY(Texture)(&tex);
//}
} // end namespace cap
