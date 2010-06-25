/*
 * CmguiManager.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "Config.h"
#include "CAPMaterial.h"

#include <boost/make_shared.hpp>
#include <sstream>
#include <fstream>
#include <iostream>

extern "C" {
#include "command/cmiss.h"
#include "graphics/material.h"
#include "graphics/element_group_settings.h"
#include "graphics/scene.h"
}

namespace cap
{

CmguiManager::CmguiManager(Cmiss_context_id context)
	: cmissContext_(context)
{
}
	
Cmiss_scene_viewer_id CmguiManager::CreateSceneViewer(wxPanel* panel, std::string const& sceneName) const
{
	Cmiss_scene_viewer_id sceneViewer = Cmiss_scene_viewer_create_wx(Cmiss_context_get_default_scene_viewer_package(cmissContext_),
			panel,
			CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
			CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
			/*minimum_colour_buffer_depth*/8,
			/*minimum_depth_buffer_depth*/8,
			/*minimum_accumulation_buffer_depth*/8);
	
	if (!sceneName.empty())
	{
		Cmiss_context_execute_command(cmissContext_, ("gfx create scene " + sceneName + " manual_g_element").c_str());
		Cmiss_scene_viewer_set_scene_by_name(sceneViewer, sceneName.c_str());
		Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1 );
	}
	
	return sceneViewer;
}

Cmiss_texture_id CmguiManager::LoadCmissTexture(std::string const& filename) const
{
	Cmiss_region_id region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_field_module_id field_module =  Cmiss_region_get_field_module(region);

	Cmiss_field_id field = Cmiss_field_module_create_image(field_module, NULL, NULL);
	Cmiss_field_image_id image_field = Cmiss_field_cast_image(field);
	
	/* Read image data from a file */
	Cmiss_field_image_read_file(image_field, filename.c_str());
	Cmiss_texture_id texture_id = Cmiss_field_image_get_texture(image_field);
	Cmiss_texture_set_filter_mode(texture_id, CMISS_TEXTURE_FILTER_LINEAR);
	
	return texture_id;
}

std::tr1::shared_ptr<CAPMaterial> CmguiManager::CreateCAPMaterial(std::string const& materialName) const
{
	Cmiss_graphics_module_id gModule = Cmiss_context_get_default_graphics_module(cmissContext_);
	// boost::make_pair is faster than shared_ptr<CAPMaterial>(new )
	return boost::make_shared<CAPMaterial>(materialName, gModule);
}

void CmguiManager::ReadRectangularModelFiles(std::string const& modelName, std::string const& sceneName) const
{
	Cmiss_region* region = Cmiss_context_get_default_region(cmissContext_);
	
	// Read in ex files that define the element used to represent the image slice
	// TODO these should be done programatically
	char filename[256]; // FIX 256 chars may not be sufficient
	sprintf(filename, "%stemplates/%s.exnode", CAP_DATA_DIR, modelName.c_str()); 
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - " << modelName << ".exnode" << std::endl;
	}
	
	sprintf(filename, "%stemplates/%s.exelem", CAP_DATA_DIR, modelName.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - " << modelName << ".exelem" << std::endl;
	}
	
	// model file name needs to be the same as its subregion name!
	Cmiss_region* subregion = Cmiss_region_find_subregion_at_path(region, modelName.c_str());
	assert(subregion);
	
	if (sceneName != "")
	{
		//This means the model is to be loaded into the specified scene.
		//Currently cmgui doesn't provide an easy way to specify the scene to be used
		//when readin a model - it always renders them in the default scene.
		//so we have to use a hack here
		std::string gfx_command("gfx draw as " + modelName + " group " + modelName + " scene " + sceneName);
		Cmiss_context_execute_command(cmissContext_, gfx_command.c_str());
		//DELETE the scene object from the default scene??
	}
}

Scene_object* CmguiManager::AssignMaterialToObject(Cmiss_scene_viewer_id scene_viewer,
		Cmiss_material_id material, std::string const& regionName) const
{
	using namespace std;
	
	struct Scene* scene;
	if (scene_viewer)
	{
		scene = Scene_viewer_get_scene(scene_viewer);
	}
	else // use default scene
	{
		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(cmissContext_);
		scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	}
		
	if (!scene)
	{
		cout << "Can't find scene" << endl;
	}

	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	//Got to find the child region first!!
	cout << "Subregion name = " << regionName << "\n";
	Cmiss_region* region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	// use the same material for selected material
	GT_element_settings_set_selected_material(settings, material);

	if(!GT_element_settings_set_material(settings, material))
	{
		//Error;
		std::cout << __func__ << " :Error setting material\n";
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		if (!Cmiss_region_modify_g_element(region, scene,settings,
			/*delete_flag*/0, /*position*/-1))
		{
			 //error
			std::cout << __func__ << " :Error modifying g element\n";
		}
	}
	
	// return sceneObject for convenience REVISE!!
	return Scene_get_scene_object_with_Cmiss_region(scene, region);
}

} // end namespace cap
