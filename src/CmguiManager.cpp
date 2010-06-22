/*
 * CmguiManager.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "Config.h"

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
	
Cmiss_scene_viewer_id CmguiManager::CreateSceneViewer(wxPanel* panel) const
{
	Cmiss_scene_viewer_id sceneViewer = Cmiss_scene_viewer_create_wx(Cmiss_context_get_default_scene_viewer_package(cmissContext_),
			panel,
			CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
			CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
			/*minimum_colour_buffer_depth*/8,
			/*minimum_depth_buffer_depth*/8,
			/*minimum_accumulation_buffer_depth*/8);
	
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

Cmiss_material_id CmguiManager::CreateCAPMaterial(std::string const& materialName) const
{
	Cmiss_material_id material = create_Graphical_material(materialName.c_str());

	// Initialize shaders that are used for adjusting brightness and contrast
	using namespace std;
	
	stringstream vp_stream, fp_stream;
	ifstream is;
	is.open("Data/shaders/vp.txt");
	vp_stream << is.rdbuf();
	is.close();
	
	is.open("Data/shaders/fp.txt");
	fp_stream << is.rdbuf();
	is.close();
	
	if (!Material_set_material_program_strings(material, 
			(char*) vp_stream.str().c_str(), (char*) fp_stream.str().c_str())
			)
	{
		cout << "Error: cant set material program strings" << endl;
	}
	
	return material;
}

void CmguiManager::ReadRectangularModelFiles(std::string const& modelName) const
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
}

void CmguiManager::SwitchMaterialTexture(Cmiss_material_id material, 
		Cmiss_texture_id tex, std::string const& regionName) const
{
	using namespace std;
	
	if (material)
	{
		if (!Graphical_material_set_texture(material,tex))//Bug this never returns 1 (returns garbage) - always returns 0 on windows
		{
			//Error
			//cout << "Error: Graphical_material_set_texture()" << endl;
		}
//		if (!Graphical_material_set_second_texture(material_, brightnessAndContrastTexture_))
//		{
//			//Error
//		}
	}
	else
	{
		cout << __func__ << " - Error: null material pointer\n";
		return;
	}
	
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	//Got to find the child region first!!
	Cmiss_region* region;
	
//	string regionName("LA1"); //temporary!!
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
		cout << "GT_element_settings_set_material() returned 0" << endl;
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(cmissContext_);
		struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		

		if (!Cmiss_region_modify_g_element(region, scene,settings,
			/*delete_flag*/0, /*position*/-1))
		{
			 //error
			cout << "Cmiss_region_modify_g_element() returned 0" << endl;
		}
	}

	return;
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
