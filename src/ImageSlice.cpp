/*
 * ImageSlice.cpp
 *
 *  Created on: May 19, 2009
 *      Author: jchu014
 */

extern "C"
{
#include "command/cmiss.h"
#include "graphics/scene_viewer.h"
#include "api/cmiss_texture.h"
#include "graphics/material.h"
#include "graphics/element_group_settings.h"
#include "graphics/scene.h"
#include "graphics/glyph.h"
#include "graphics/colour.h"
#include "graphics/material.h"
}

#include "Config.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
#include "ImageSlice.h"

#include <iostream>
#include <sstream>
#include <fstream>

//#include <stdio.h>
using namespace std;

ImageSlice::ImageSlice(const string& name)
	: 
	sliceName_(name),
	oldIndex_(-1),
	isVisible_(true)
{
	// Initialize the texture used to pass brightness and contrast to fragment shader
	string brightnessAndContrastTextureName(name + "_BrightnessAndContrast"); 
	brightnessAndContrastTexture_ = CREATE(Texture)(brightnessAndContrastTextureName.c_str());
	unsigned char source_pixels[4] = {255,25,0,255};//TEST
	if (!Cmiss_texture_set_pixels(brightnessAndContrastTexture_,
		1 /*int width */, 1/*int height*/, 1/*int depth*/,
		4 /*int number_of_components*/, 1 /*int number_of_bytes_per_component*/,
		4 /*int source_width_bytes*/, source_pixels))
	{
		//Error
		cout << "ImageSlice::ImageSlice() Error setting pixel value to brightnessAndContrastTexture_" << endl;
	}
	
//	unsigned char temp[4];
//	unsigned char fill[1] = {0};
//	Cmiss_texture_get_pixels(brightnessAndContrastTexture_,
//		0, 0, 0,
//		1, 1, 1,
//		0, 0, 
//		fill,
//		4, temp);
	
	//printf("%d %d %d %d\n", temp[0], temp[1], temp[2], temp[3]);
	//int dim;
	//Texture_get_dimension(brightnessAndContrastTexture_, &dim);
	//cout << "dim = " << dim << endl;
	
	this->LoadImagePlaneModel();
	this->LoadTextures();
	this->TransformImagePlane();
	this->InitializeDataPointGraphicalSetting();
}

ImageSlice::~ImageSlice()
{
	//TODO clean up material and textures
}

void ImageSlice::SetVisible(bool visibility)
{
	if (visibility)
	{
		isVisible_ = true;
		Scene_object_set_visibility(sceneObject_, g_VISIBLE);
	}
	else
	{
		isVisible_ = false;
		Scene_object_set_visibility(sceneObject_, g_INVISIBLE);
	}
	return;
}

void ImageSlice::SetTime(double time)
{
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();

	int index = static_cast<int>(time * textures_.size()); // -1
	//boundary checks
	if (index >= textures_.size())
	{
		index = textures_.size() - 1;
	}
	else if (index < 0)
	{
		index = 0;
	}
	
	//update texture only when it is necessary
	if (index == oldIndex_|| !isVisible_)
	{
		return; 
	}
	oldIndex_ = index;
	
	//DEBUG
	//cout << "ImageSlice::setTime index = " << index << endl;
		
	Cmiss_texture* tex= textures_[index];
	
	if (material_)
	{
		if (!Graphical_material_set_texture(material_,tex))//Bug this never returns 1 (returns garbage) - always returns 0 on windows
//		if (!Graphical_material_set_texture(material_,brightnessAndContrastTexture_))
		{
			//Error
			//cout << "Error: Graphical_material_set_texture()" << endl;
		}
		if (!Graphical_material_set_second_texture(material_, brightnessAndContrastTexture_))
//		if (!Graphical_material_set_second_texture(material_, tex))
		{
			//Error
		}
	}
	else
	{
		cout << "Error: cant find material" << endl;
	}
	
	Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
	//Got to find the child region first!!
	Cmiss_region* region;
	if(!Cmiss_region_get_region_from_path(root_region, sliceName_.c_str(), &region))
	{
		//error
		std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	//hack
	GT_element_settings_set_selected_material(settings, material_);

	if(!GT_element_settings_set_material(settings, material_))
	{
		//Error;
		cout << "GT_element_settings_set_material() returned 0" << endl;
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(root_region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
		struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		
		int Cmiss_region_modify_g_element(struct Cmiss_region *region,
			struct Scene *scene, struct GT_element_settings *settings,
			int delete_flag, int position);  // should add this to a header file somewhere

		 if (!Cmiss_region_modify_g_element(region, scene,settings,
			/*delete_flag*/0, /*position*/-1))
		 {
			 //error
			 cout << "Cmiss_region_modify_g_element() returned 0" << endl;
		 }
	}

	return ;
}

void ImageSlice::SetBrightness(float brightness)
{
	unsigned char pixels[4];
	unsigned char fill[1] = {0};
	Cmiss_texture_get_pixels(brightnessAndContrastTexture_,
		0, 0, 0,
		1, 1, 1,
		0, 0, 
		fill,
		4, pixels);
	
	pixels[0] = 255.0 * brightness;
	
	if (!Cmiss_texture_set_pixels(brightnessAndContrastTexture_,
		1 /*int width */, 1/*int height*/, 1/*int depth*/,
		4 /*int number_of_components*/, 1 /*int number_of_bytes_per_component*/,
		4 /*int source_width_bytes*/, pixels))
	{
		//Error
		cout << "ImageSlice::SetBrightness() Error setting pixel value to brightnessAndContrastTexture_" << endl;
	}
	Texture_notify_change(brightnessAndContrastTexture_);
}

void ImageSlice::SetContrast(float contrast)
{
	unsigned char pixels[4];
	unsigned char fill[1] = {0};
	Cmiss_texture_get_pixels(brightnessAndContrastTexture_,
		0, 0, 0,
		1, 1, 1,
		0, 0, 
		fill,
		4, pixels);
	
	pixels[1] = 255.0 * contrast;
	
	if (!Cmiss_texture_set_pixels(brightnessAndContrastTexture_,
		1 /*int width */, 1/*int height*/, 1/*int depth*/,
		4 /*int number_of_components*/, 1 /*int number_of_bytes_per_component*/,
		4 /*int source_width_bytes*/, pixels))
	{
		//Error
		cout << "ImageSlice::SetContrast() Error setting pixel value to brightnessAndContrastTexture_" << endl;
	}
	Texture_notify_change(brightnessAndContrastTexture_);
}

#include "CmguiExtensions.h"

void ImageSlice::LoadImagePlaneModel()
{
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	
	char filename[256];
	string& name = sliceName_;
	
	Cmiss_region* region = Cmiss_command_data_get_root_region(command_data);
	
	// Read in ex files that define the element used to represent the image slice
	// TODO these should be done programatically
	sprintf(filename, "%stemplates/%s.exnode", prefix, name.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exnode" << std::endl;
	}
	
	sprintf(filename, "%stemplates/%s.exelem", prefix, name.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exelem" << std::endl;
	}
		
	material_ = create_Graphical_material(name.c_str());

	// Initialize shaders that are used for adjusting brightness and contrast
	int Material_set_material_program_strings(struct Graphical_material *material_to_be_modified,
		char *vertex_program_string, char *fragment_program_string);//not defined in material.h
	
	stringstream vp_stream, fp_stream;
	ifstream is;
	is.open("Data/shaders/vp.txt");
	vp_stream << is.rdbuf();
	is.close();
	
	is.open("Data/shaders/fp.txt");
	fp_stream << is.rdbuf();
	is.close();
	
//	cout << "SHADERS:" << endl << vp_stream.str().c_str() << endl << fp_stream.str().c_str() << endl;
	if (!Material_set_material_program_strings(material_, 
			(char*) vp_stream.str().c_str(), (char*) fp_stream.str().c_str())
			)
	{
		cout << "Error: cant set material program strings" << endl;
	}
	
//	Material_package* material_package = Cmiss_command_data_get_material_package(command_data);
//	Material_package_manage_material(material_package, material_);
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	if (!scene)
	{
		cout << "Can't find scene" << endl;
	}

	Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
	//Got to find the child region first!!
	if(!Cmiss_region_get_region_from_path(root_region, name.c_str(), &region))
	{
		//error
		std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	//hack
	GT_element_settings_set_selected_material(settings, material_);

	if(!GT_element_settings_set_material(settings, material_))
	{
		//Error;
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(root_region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		int Cmiss_region_modify_g_element(struct Cmiss_region *region,
			struct Scene *scene, struct GT_element_settings *settings,
			int delete_flag, int position);  // should add this to a header file somewhere

		 if (!Cmiss_region_modify_g_element(region, scene,settings,
			/*delete_flag*/0, /*position*/-1))
		 {
			 //error
		 }
	}

	
	// cache the sceneObject for convenience
	sceneObject_ = Scene_get_scene_object_with_Cmiss_region(scene, region);
	return;
}

#include "FileSystem.h"

void ImageSlice::LoadTextures()
{
	string dir_path(prefix);
	dir_path.append("images/");
	dir_path.append(sliceName_);
	
	FileSystem fs(dir_path);
	
	const vector<string>& filenames = fs.getAllFileNames();
	
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	struct Cmiss_texture_manager* manager = Cmiss_command_data_get_texture_manager(command_data);
	struct IO_stream_package* io_stream_package = Cmiss_command_data_get_IO_stream_package(command_data);

	vector<string>::const_iterator itr = filenames.begin();
	vector<string>::const_iterator end = filenames.end();

	//char fullpath[256]; //FIX
	for (; itr != end; ++itr)
	{
		const string& filename = *itr;
		//sprintf(fullpath, "%s/%s", dir_path.c_str(),  filename.c_str()); 
		string fullpath(dir_path);
		fullpath.append("/");
		fullpath.append(filename);
		
		Cmiss_texture_id texture_id = Cmiss_texture_manager_create_texture_from_file(
			manager, filename.c_str(), io_stream_package, fullpath.c_str());
		if (!texture_id)
		{
			cout <<"ERROR:: cant create texture from file" << endl;
		}

		textures_.push_back(texture_id);
		
		images_.push_back(new DICOMImage(fullpath));
	}	
	return;
}

void ImageSlice::TransformImagePlane()
{
	// Now get the necessary info from the DICOM header
	
	DICOMImage& dicomImage = *images_[0]; //just use the first image in the slice
	ImagePlane* plane = dicomImage.GetImagePlaneFromDICOMHeaderInfo();
	if (!plane)
	{
		cout << "ERROR !! plane is null"<<endl;
	}
	else
	{
		cout << plane->tlc << endl;
		imagePlane_ = plane;
	}

	int nodeNum = 81; // HACK FIX
	
	if (sliceName_=="SA2")
	{
		nodeNum += 300;
	}
	else if (sliceName_=="SA3")
	{
		nodeNum += 400;
	}
	else if (sliceName_=="SA4")
	{
		nodeNum += 500;
	}
	else if (sliceName_=="SA5")
	{
		nodeNum += 600;
	}
	else if (sliceName_=="SA6")
	{
		nodeNum += 700;
	}
	else if (sliceName_ =="LA1")
	{
		nodeNum += 1100;
	}
	else if (sliceName_ =="LA2")
	{
		nodeNum += 1200;
	}
	else if (sliceName_ =="LA3")
	{
		nodeNum += 1300;
	}

	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
	//Got to find the child region first!!
	Cmiss_region* region;
	if(!Cmiss_region_get_region_from_path(root_region, sliceName_.c_str(), &region))
	{
		//error
		std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
	}
	
	char nodeName[256]; //FIX
	sprintf(nodeName,"%d", nodeNum);
	Cmiss_node* node = Cmiss_region_get_node(region, nodeName);
	if (node) {
		FE_node_set_position_cartesian(node, 0, plane->blc.x, plane->blc.y, plane->blc.z);
	}
	else
	{
		cout << nodeName << endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node = Cmiss_region_get_node(region, nodeName))
	{
		FE_node_set_position_cartesian(node, 0, plane->brc.x, plane->brc.y, plane->brc.z);
	}
	else
	{
		cout << nodeName << endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node = Cmiss_region_get_node(region, nodeName))
	{
		FE_node_set_position_cartesian(node, 0, plane->tlc.x, plane->tlc.y, plane->tlc.z);
	}
	else
	{
		cout << nodeName << endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node = Cmiss_region_get_node(region, nodeName))
	{
		FE_node_set_position_cartesian(node, 0, plane->trc.x, plane->trc.y, plane->trc.z);
	}
	else
	{
		cout << nodeName << endl;
	}
}

const ImagePlane& ImageSlice::GetImagePlane() const
{
	return *(imagePlane_);
}

void ImageSlice::InitializeDataPointGraphicalSetting()
{	
	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_DATA_POINTS);
	Graphical_material* material = create_Graphical_material("DataPoints");//TODO Need to clean up consider using Material_package
	Graphical_material* materialSelected = create_Graphical_material("DataPointsSelected");
	
	Colour green = {0,1,0}; //BGR
	Colour yellow = {0,1,1}; //BGR
	Graphical_material_set_diffuse(material, &yellow);
	Graphical_material_set_diffuse(materialSelected, &green);
	GT_element_settings_set_material(settings,material);
	GT_element_settings_set_selected_material(settings, materialSelected);

	//Glyphs
	GT_object *glyph, *old_glyph;
	Glyph_scaling_mode glyph_scaling_mode;
	Triple glyph_centre,glyph_scale_factors,glyph_size;
	Computed_field *orientation_scale_field, *variable_scale_field; ;
	glyph=make_glyph_sphere("sphere",12,6);
	
	Triple new_glyph_size;
	new_glyph_size[0] = 2, new_glyph_size[1] = 2, new_glyph_size[2] = 2;
	
	if (!(GT_element_settings_get_glyph_parameters(settings,
		 &old_glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
		 &orientation_scale_field, glyph_scale_factors,
		 &variable_scale_field) &&
		GT_element_settings_set_glyph_parameters(settings,glyph,
		 glyph_scaling_mode, glyph_centre, new_glyph_size,
		 orientation_scale_field, glyph_scale_factors,
		 variable_scale_field)))
	{
		cout << "No glyphs defined" << endl;
	}

	GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(sceneObject_);
	GT_element_group_add_settings(gt_element_group, settings, 0);
	
	//Initialze fields needed for time-dependent visibility control
}