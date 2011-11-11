/*
 * SceneViewerPanel.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include <fstream>
#include <iostream>

#include <boost/foreach.hpp>

#include <wx/app.h>

extern "C"
{
#include <configure/cmgui_configure.h>
#include <api/cmiss_field_image.h>
#include <api/cmiss_scene_viewer.h>
#include <api/cmiss_stream.h>
#include <api/cmiss_interactive_tool.h>
#include <api/cmiss_rendition.h>
#include <api/cmiss_graphic.h>
#include <api/cmiss_field_module.h>
#include <api/cmiss_field.h>
#include <api/cmiss_node.h>
#include <api/cmiss_field_finite_element.h>
}

#include "capclientconfig.h"
#include "utils/debug.h"
#include "cmgui/sceneviewerpanel.h"
#include "cmgui/extensions.h"
#include "cmgui/callbacks.h"

namespace cap
{

SceneViewerPanel::SceneViewerPanel(Cmiss_context_id cmissContext, const std::string& name, wxPanel* panel)
	: cmissSceneViewer_(Cmiss_context_create_scene_viewer(cmissContext, name, panel))
{
	SetFreeSpin(false);

	// The following crashes on my Windows 7 machine.
	//Cmiss_scene_viewer_set_transparency_mode(cmissSceneViewer_, CMISS_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT);
	//Cmiss_scene_viewer_set_transparency_layers(cmissSceneViewer_, 4);
}
	
SceneViewerPanel::~SceneViewerPanel()
{
	std::cout << "SceneViewerPanel::~SceneViewerPanel()" << std::endl;
	Cmiss_scene_viewer_destroy(&cmissSceneViewer_);
}

void SceneViewerPanel::LookHere() const
{
	Cmiss_scene_viewer_set_lookat_parameters_non_skew(
		cmissSceneViewer_, 105.824, 164.155, 5.39987,
		11.3427, -2.31351, -10.3133,
		-0.0184453, 0.104329, -0.994372 );
	//Cmiss_scene_viewer_set_near_and_far_plane(cmissSceneViewer_, 1.92056, 686.342);
	Cmiss_scene_viewer_set_view_angle(cmissSceneViewer_, 0.220906);
	Cmiss_scene_viewer_set_viewport_mode(cmissSceneViewer_, CMISS_SCENE_VIEWER_VIEWPORT_RELATIVE);
	Cmiss_scene_viewer_set_viewing_volume(cmissSceneViewer_, -15.0613, 15.0613, -15.0613, 15.0613, 1.92056, 686.342);
}

void SceneViewerPanel::SetCallback(Cmiss_scene_viewer_input_callback callback, void *callback_class, bool addFirst) const
{
	Cmiss_scene_viewer_add_input_callback(cmissSceneViewer_, callback, callback_class, addFirst ? 1 : 0);
}

void SceneViewerPanel::RemoveCallback(Cmiss_scene_viewer_input_callback callback, void *callback_class) const
{
	Cmiss_scene_viewer_remove_input_callback(cmissSceneViewer_, callback, callback_class);
}

void SceneViewerPanel::SetInteractiveTool(const std::string& tool, const std::string& command) const
{
	Cmiss_scene_viewer_set_interactive_tool_by_name(cmissSceneViewer_, tool.c_str());
	if (command.size() > 0)
	{
		Cmiss_interactive_tool_id i_tool = Cmiss_scene_viewer_get_current_interactive_tool(cmissSceneViewer_);
		Cmiss_interactive_tool_execute_command(i_tool, command.c_str());
		Cmiss_interactive_tool_destroy(&i_tool);
	}
}

void SceneViewerPanel::LookingHere() const
{
	double dof, fd, eyex, eyey, eyez, lx, ly, lz, upx, upy, upz, va, left, right, bottom, top, np, fp;
	Cmiss_scene_viewer_get_depth_of_field(cmissSceneViewer_, &dof, &fd);
	Cmiss_scene_viewer_get_lookat_parameters(cmissSceneViewer_, &eyex, &eyey, &eyez, &lx, &ly, &lz, &upx, &upy, &upz);
	Cmiss_scene_viewer_get_view_angle(cmissSceneViewer_, &va);
	Cmiss_scene_viewer_get_viewing_volume(cmissSceneViewer_, &left, &right, &bottom, &top, &np, &fp);

	dbg("dof : " + toString(dof) + ", " + toString(fd));
	dbg("eye : " + toString(eyex) + ", " + toString(eyey) + ", " + toString(eyez));
	dbg("loo : " + toString(lx) + ", " + toString(ly) + ", " + toString(lz));
	dbg("up  : " + toString(upx) + ", " + toString(upy) + ", " + toString(upz));
	dbg("va  : " + toString(va));
	dbg("vvo : " + toString(left) + ", " + toString(right) + ", " + toString(bottom)+ ", " + toString(top) + ", " + toString(np) + ", " + toString(fp));
}

void SceneViewerPanel::SetFreeSpin(bool on)
{
	Cmiss_scene_viewer_set_interactive_tool_by_name(cmissSceneViewer_, "transform_tool");
	Cmiss_interactive_tool_id i_tool = Cmiss_scene_viewer_get_current_interactive_tool(cmissSceneViewer_);
	if (on)
		Cmiss_interactive_tool_execute_command(i_tool, "free_spin");
	else
		Cmiss_interactive_tool_execute_command(i_tool, "no_free_spin");
	
	Cmiss_interactive_tool_destroy(&i_tool);
}

void SceneViewerPanel::SetViewingPlane(const ImagePlane& plane)
{
	// compute the center of the image plane, eye(camera) position and the up vector
	Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
	Point3D eye = planeCenter + (plane.normal * 500); // this seems to determine the near clip plane
	Vector3D up(plane.yside);
	up.Normalise();
	
	//Hack :: perturb direction vector a little
	eye.x *= 1.01; //HACK 1.001 makes the iso lines partially visible
	
	if (!Cmiss_scene_viewer_set_lookat_parameters_non_skew(
		cmissSceneViewer_, eye.x, eye.y, eye.z,
		planeCenter.x, planeCenter.y, planeCenter.z,
		up.x, up.y, up.z))
	{
		//Error;
	}
}

void SceneViewerPanel::SetViewingVolume(double radius)
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
	Cmiss_scene_viewer_redraw_now(cmissSceneViewer_);
}

void SceneViewerPanel::SetTumbleRate(double speed)
{
	Cmiss_scene_viewer_set_tumble_rate(cmissSceneViewer_, speed);
}

void SceneViewerPanel::ViewAll() const
{
	Cmiss_scene_viewer_view_all(cmissSceneViewer_);
}

/*
void SceneViewerPanel::AssignMaterialToObject(Cmiss_scene_viewer_id scene_viewer,
		Cmiss_graphics_material_id material, std::string const& regionName) const
{
	//using namespace std;
	
	std::cout << "SceneViewerPanel::AssignMaterialToObject" << std::endl;
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
	
	std::cout << "Leaving: SceneViewerPanel::AssignMaterialToObject" << std::endl;
	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);

	// return sceneObject for convenience REVISE!!
	return;
}*/

//void SceneViewerPanel::DestroyTexture(Cmiss_texture_id tex) const
//{
//	DESTROY(Texture)(&tex);
//}
} // end namespace cap
