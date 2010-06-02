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
#include "finite_element/finite_element_region.h"
}

#include "Config.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
#include "ImageSlice.h"
#include "FileSystem.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

//#include <stdio.h>

int Material_set_material_program_strings(struct Graphical_material *material_to_be_modified,
	char *vertex_program_string, char *fragment_program_string);//not defined in material.h

int Cmiss_region_modify_g_element(struct Cmiss_region *region,
	struct Scene *scene, struct GT_element_settings *settings,
	int delete_flag, int position);  // should add this to a header file somewhere

using namespace std;

namespace cap
{

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
		oldIndex_ = -1; //this forces rebinding & redraw of the texture
		SetTime(time_);
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
	time_ = time; // store time for later use
	
	Cmiss_context_id context = CmguiManager::getInstance().getCmissContext();

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
	
	Cmiss_region* root_region = Cmiss_context_get_default_region(context);
	//Got to find the child region first!!
	Cmiss_region* region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	// use the same material for selected material
	GT_element_settings_set_selected_material(settings, material_);

	if(!GT_element_settings_set_material(settings, material_))
	{
		//Error;
		cout << "GT_element_settings_set_material() returned 0" << endl;
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(context);
		struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		

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
	Cmiss_context_id context = CmguiManager::getInstance().getCmissContext();
	
	char filename[256];
	string& name = sliceName_;
	
	Cmiss_region* region = Cmiss_context_get_default_region(context);
	
	// Read in ex files that define the element used to represent the image slice
	// TODO these should be done programatically
	sprintf(filename, "%stemplates/%s.exnode", CAP_DATA_DIR, name.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exnode" << std::endl;
	}
	
	sprintf(filename, "%stemplates/%s.exelem", CAP_DATA_DIR, name.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exelem" << std::endl;
	}
		
	material_ = create_Graphical_material(name.c_str());

	// Initialize shaders that are used for adjusting brightness and contrast
	
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
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(context);
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	if (!scene)
	{
		cout << "Can't find scene" << endl;
	}

	Cmiss_region* root_region = Cmiss_context_get_default_region(context);
	//Got to find the child region first!!
	cout << "Subregion name = " << name << "\n";
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, name.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	// use the same material for selected material
	GT_element_settings_set_selected_material(settings, material_);

	if(!GT_element_settings_set_material(settings, material_))
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

	
	// cache the sceneObject for convenience
	sceneObject_ = Scene_get_scene_object_with_Cmiss_region(scene, region);
	return;
}

void ImageSlice::LoadTextures()
{
	string dir_path(CAP_DATA_DIR);
	dir_path.append("images/");
	dir_path.append(sliceName_);
	
	FileSystem fs(dir_path);
	
	const vector<string>& filenames = fs.getAllFileNames();
	
	vector<string>::const_iterator itr = filenames.begin();
	vector<string>::const_iterator end = filenames.end();

	Cmiss_context_id context = CmguiManager::getInstance().getCmissContext();
	for (; itr != end; ++itr)
	{
		const string& filename = *itr;
		string fullpath(dir_path);
		fullpath.append("/");
		fullpath.append(filename);
		
		//temp
		
		string texture_path = fullpath;
		
//		string filename_jpg = filename.substr(0, filename.length() - 3);
//		
//		string jpg_path = "./Data/images/jpg/";
//		jpg_path.append(filename_jpg);
//		jpg_path.append("jpg");
//		
//		ifstream jpg_stream;
//		jpg_stream.open(jpg_path.c_str());
//		if (jpg_stream.is_open())
//		{
//			cout << "filename = " << jpg_path << endl;
//			texture_path = jpg_path;
//		}
		
		
		string textureName(sliceName_);
		textureName.append(filename);
		
		Cmiss_region_id region = Cmiss_context_get_default_region(context);
		Cmiss_field_module_id field_module =  Cmiss_region_get_field_module(region);

		Cmiss_field_id field_in = Cmiss_field_module_create_image(field_module, NULL, NULL);
		Cmiss_field_image_id image_field_in = Cmiss_field_cast_image(field_in);
		
		/* Read image data from a file */
		Cmiss_field_image_read_file(image_field_in, texture_path.c_str());
		
//		Cmiss_field_id field_rescale = Cmiss_field_create_image(NULL, NULL);
//		int result = Cmiss_field_set_type_rescale_intensity_image_filter(field_rescale,
//				field_in, 0.0 /* min */, 1.0 /* max */);
//		
//		if (!result) 
//		{
//			cout << "Cant set rescale_intensity_image_filter is null" << endl; 
//		}
//		
//		Cmiss_field_id field_out = Cmiss_field_create_image(NULL, field_rescale);
//		Cmiss_field_image_id image_field_out = Cmiss_field_image_cast(field_out);
//		
//		if (!image_field_out) 
//		{
//			cout << "image_field_out is null" << endl; 
//		}
//		
//		Cmiss_texture_id texture_id = Cmiss_field_image_get_texture(image_field_out);
		Cmiss_texture_id texture_id = Cmiss_field_image_get_texture(image_field_in);
		
		Cmiss_texture_set_filter_mode(texture_id, CMISS_TEXTURE_FILTER_LINEAR);
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

	// Read in plane shift info if it exists for this slice
	string filePath(CAP_DATA_DIR);
	filePath.append("images/");
	filePath.append(sliceName_);
	filePath.append(".txt");
	
	ifstream planeShiftInfoFile(filePath.c_str());
	
	if (planeShiftInfoFile.is_open())
	{
		cout << "Plane shift info file present - " << filePath << endl;
		
		planeShiftInfoFile >> plane->tlc >> plane->trc >> plane->blc;
		
		Vector3D v = plane->trc - plane->tlc;
		
		plane->brc = plane->blc + v;
		
		cout << "corrected tlc = " << plane->tlc <<endl;
		cout << "corrected trc = " << plane->trc <<endl;
		cout << "corrected blc = " << plane->blc <<endl;
		cout << "corrected brc = " << plane->brc <<endl;
		
		planeShiftInfoFile.close();
	}
	
	int nodeNum = 1;

	Cmiss_context_id context = CmguiManager::getInstance().getCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(context);
	//Got to find the child region first!!
	Cmiss_region* region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
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

Cmiss_field* ImageSlice::CreateVisibilityField()
{
	Cmiss_context_id context = CmguiManager::getInstance().getCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(context);
	Cmiss_region* region;
	region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str());
	CM_field_type cm_field_type = CM_GENERAL_FIELD;
	char* name = "visibility";
	Coordinate_system coordinate_system;
	coordinate_system.type = RECTANGULAR_CARTESIAN;
	Value_type value_type = FE_VALUE_VALUE;
	const int number_of_components = 1;
	char* component_names[] = {"visibility"};
	
	FE_region_get_FE_field_with_properties(
		Cmiss_region_get_FE_region(region),
		name, GENERAL_FE_FIELD,
		/*indexer_field*/(struct FE_field *)NULL, /*number_of_indexed_values*/0,
		cm_field_type, &coordinate_system,
		value_type, number_of_components, component_names,
		/*number_of_times*/0, /*time_value_type*/UNKNOWN_VALUE,
		/*external*/(struct FE_field_external_information *)NULL);
	
	manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
	Computed_field* field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("visibility",cfm);
	
	return field;
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

	//Initialze fields needed for time-dependent visibility control

	Cmiss_field* visibilityField = CreateVisibilityField();
	GT_element_settings_set_visibility_field(settings, visibilityField);
	
	GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(sceneObject_);
	GT_element_group_add_settings(gt_element_group, settings, 0);
}

void ImageSlice::WritePlaneInfoToFile(const std::string& filepath) const
{
	Cmiss_context_id context = CmguiManager::getInstance().getCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(context);

	//Got to find the child region first!!
	Cmiss_region* region;
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
	}
	
	Cmiss_node* node = Cmiss_region_get_node(region, "1");
	Point3D blc;
	if (node) {
		FE_node_get_position_cartesian(node, 0, &(blc.x), &(blc.y), &(blc.z), 0);
	}

	Point3D tlc;
	if (node = Cmiss_region_get_node(region, "3"))
	{
		FE_node_get_position_cartesian(node, 0, &(tlc.x), &(tlc.y), &(tlc.z), 0);
	}
	else
	{
		std::cout << "Error:\n";
	}

	Point3D trc;
	if (node = Cmiss_region_get_node(region, "4"))
	{
		FE_node_get_position_cartesian(node, 0, &(trc.x), &(trc.y), &(trc.z), 0);
	}
	
	std::ofstream outFile(filepath.c_str());
	//std::cout << sliceName_ << "\n";
//	std::cout << "tlc";
//	std::cout << std::setw(12) << tlc.x << "i";
//	std::cout << std::setw(12) << tlc.y << "j";
//	std::cout << std::setw(12) << tlc.z << "k";
//	std::cout << std::endl;
	
	outFile << "tlc";
	outFile << std::setw(12) << tlc.x << "i";
	outFile << std::setw(12) << tlc.y << "j";
	outFile << std::setw(12) << tlc.z << "k";
	outFile << std::endl;
	
	outFile << "trc";
	outFile << std::setw(12) << trc.x << "i";
	outFile << std::setw(12) << trc.y << "j";
	outFile << std::setw(12) << trc.z << "k";
	outFile << std::endl;
	
	outFile << "blc";
	outFile << std::setw(12) << blc.x << "i";
	outFile << std::setw(12) << blc.y << "j";
	outFile << std::setw(12) << blc.z << "k";
	outFile << std::endl;

	outFile.close();
}

} // end namespace cap
