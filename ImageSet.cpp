/*
 * ImageSet.cpp
 *
 *  Created on: Feb 27, 2009
 *      Author: jchu014
 */

/**
 * Image Slice
 */

#include "ImageSet.h"

extern "C"
{
#include "command/cmiss.h"
#include "graphics/scene_viewer.h"
#include "api/cmiss_region.h"
#include "api/cmiss_texture.h"
#include "graphics/material.h"
#include "graphics/element_group_settings.h"
#include "graphics/scene.h"
}

#include "Config.h"
#include "CmguiManager.h"
#include "DICOMImage.h"

#include <iostream>

using namespace std;

ImageSlice::ImageSlice(const string& name)
	: sliceName_(name)
{
	this->LoadImagePlaneModel();
	this->LoadTextures();
	this->TransformImagePlane(); 
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
//	if (index = -1)
//	{
//		index = numberOfFrames;
//	}
	
	//DEBUG
	//cout << "ImageSlice::setTime index = " << index << endl;
		
	Cmiss_texture* tex= textures_[index];
	
	if (material_)
	{
		if (!Graphical_material_set_texture(material_,tex))
		{
			//Error
			cout << "Error: Graphical_material_set_texture()" << endl;
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

#include "CmguiExtensions.h"

void ImageSlice::LoadImagePlaneModel()
{
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	
	char filename[256];
	string& name = sliceName_;
	
	Cmiss_region* region = Cmiss_command_data_get_root_region(command_data);
	
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
		
#define ADJUST_BRIGHTNESS
#ifdef ADJUST_BRIGHTNESS
	//	gfx define field tex sample_texture coordinates xi texture LA1;
		
	//	gfx define field rescaled_tex rescale_intensity_filter field tex output_min 0 output_max 1;
	//	gfx cre spectrum monochrome clear;
	//	gfx modify spectrum monochrome linear range 0 1 extend_above extend_below monochrome colour_range 0 1 ambient diffuse component 1;
	//	#create a texture using the rescaled sample_texture field
	//	gfx create texture tract linear;
	//	gfx modify texture tract width 1 height 1 distortion 0 0 0 colour 0 0 0 alpha 0 decal linear_filter resize_nearest_filter clamp_wrap specify_number_of_bytes 2 evaluate field rescaled_tex element_group LA1 spectrum monochrome texture_coordinate xi fail_material transparent_gray50;
	//
	//	# create a material containing the texture so that we can select along
	//	# with appropriate texture coordinates to visualise the 3D image
	//	gfx create material tract texture tract
#endif //ADJUST_BRIGHTNESS
		
	material_ = create_Graphical_material(name.c_str());

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
	
	vector<string> filenames = fs.getAllFileNames();
	
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	struct Cmiss_texture_manager* manager = Cmiss_command_data_get_texture_manager(command_data);
	struct IO_stream_package* io_stream_package = Cmiss_command_data_get_IO_stream_package(command_data);

	vector<string>::const_iterator itr = filenames.begin();
	vector<string>::const_iterator end = filenames.end();

	char fullpath[256]; //FIX
	for (; itr != end; ++itr)
	{
		const string& filename = *itr;
		sprintf(fullpath, "%s/%s", dir_path.c_str(),  filename.c_str()); 
		Cmiss_texture_id texture_id = Cmiss_texture_manager_create_texture_from_file(
			manager, filename.c_str(), io_stream_package, fullpath);
		
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
/** 
 * ImageSet
 */

ImageSet::ImageSet(const vector<string>& sliceNames)
:
imageSliceNames_(sliceNames)
{
	vector<string>::const_iterator itr = sliceNames.begin();
	for (;itr != sliceNames.end();++itr)
	{
		const string& name = *itr;
		
		ImageSlice* imageSlice = new ImageSlice(name);
		imageSlicesMap_[name] = imageSlice; // use exception safe container or smartpointers
	}
}

void ImageSet::SetTime(double time)
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.begin();
	ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
	for (;itr != end; ++itr)
	{
		itr->second->SetTime(time);
	}
	return;
}

#include <algorithm>

void ImageSet::SetVisible(bool visible, const std::string& sliceName)
{
	if (sliceName.length())
	{
		ImageSlicesMap::iterator itr = imageSlicesMap_.find(sliceName);
		if (itr == imageSlicesMap_.end())
		{
			//error should probably throw exception
			assert(!"No such name in the imageSliceMap_");
		}
		else
		{
			itr->second->SetVisible(visible);
		}
	}
	else //zero length name string:: set visibility for the whole set
	{
		ImageSlicesMap::iterator itr = imageSlicesMap_.begin();
		ImageSlicesMap::const_iterator end = imageSlicesMap_.end();
		for (;itr!=end;++itr)
		{
			itr->second->SetVisible(visible);
		}
	}
}

void ImageSet::SetVisible(bool visible, int index)
{
	if (imageSlicesMap_.size() <= index || index < 0)
	{
		assert(!"Index out of bound: imageSliceMap_");
	}
	else //zero length name string:: set visibility for the whole set
	{
		const std::string& name = imageSliceNames_[index];
		imageSlicesMap_[name]->SetVisible(visible);
	}
}

const ImagePlane& ImageSet::GetImagePlane(const std::string& sliceName) const
{
	ImageSlicesMap::const_iterator itr = imageSlicesMap_.find(sliceName);
	if (itr == imageSlicesMap_.end())
	{
		//error should probably throw exception
		assert(!"No such name in the imageSliceMap_");
		
		throw std::exception();
	}
	else
	{
		return itr->second->GetImagePlane();
	} 
}
