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
#include "finite_element/finite_element_region.h"
}

#include "Config.h"
#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "DICOMImage.h"
#include "ImageSlice.h"
//#include "FileSystem.h"
#include "CAPMaterial.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

//#include <stdio.h>

using namespace std;

namespace cap
{

class ImageSlice::CmguiImpl
{
public:
	std::string sliceName_;
	Scene_object* sceneObject_;
	CmguiManager const& cmguiManager_;
	std::tr1::shared_ptr<CAPMaterial> material_;
	
	CmguiImpl(CmguiManager const& cmguiManager, std::string const& sliceName)
	:
		cmguiManager_(cmguiManager),
		sliceName_(sliceName)
	{
		this->LoadImagePlaneModel();
//		this->TransformImagePlane();
		
		this->InitializeDataPointGraphicalSetting();
	}
	
	~CmguiImpl()
	{
		Cmiss_region_id root = Cmiss_context_get_default_region(cmguiManager_.GetCmissContext());
		Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root, sliceName_.c_str());
		if(!region)
		{
			//error
			std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
		}
		Cmiss_region_remove_child(root, region);
		std::cout << __func__ << '\n';
		Cmiss_region_destroy(&region);
		Cmiss_region_destroy(&root);
	}
	
	void LoadImagePlaneModel()
	{		
		cmguiManager_.ReadRectangularModelFiles(sliceName_);			
		material_ = cmguiManager_.CreateCAPMaterial(sliceName_);
		// Assign material & cache the sceneObject for convenience
		sceneObject_ = cmguiManager_.AssignMaterialToObject(0, material_->GetCmissMaterial(), sliceName_);
		return;
	}
	
	void SetVisible(bool visibility)
	{
		GT_visibility_type visible = visibility ? g_VISIBLE : g_INVISIBLE;
		Scene_object_set_visibility(sceneObject_, visible);
	}
	
	Cmiss_field* CreateVisibilityField()
	{
		Cmiss_context_id cmissContext_ = cmguiManager_.GetCmissContext();
		Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
		Cmiss_region* region;
		region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str());
		CM_field_type cm_field_type = CM_GENERAL_FIELD;
		char* name = (char*)"visibility";
		Coordinate_system coordinate_system;
		coordinate_system.type = RECTANGULAR_CARTESIAN;
		Value_type value_type = FE_VALUE_VALUE;
		const int number_of_components = 1;
		char* component_names[] = {(char*)"visibility"};
		
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
		
		Cmiss_region_destroy(&region);
		Cmiss_region_destroy(&root_region);

		return field;
	}

	void InitializeDataPointGraphicalSetting()
	{	
		GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_DATA_POINTS);
		Graphical_material* material = create_Graphical_material("DataPoints");//TODO Need to clean up consider using Material_package
		Graphical_material* materialSelected = create_Graphical_material("DataPointsSelected");
		
		Colour green = {0,1,0}; //BGR
		Colour yellow = {0,1,1}; //BGR
		Graphical_material_set_diffuse(material, &yellow);
		Graphical_material_set_diffuse(materialSelected, &green);
		GT_element_settings_set_select_mode(settings, GRAPHICS_SELECT_ON);
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
	
	void ChangeTexture(Cmiss_texture* tex)
	{
		material_->ChangeTexture(tex);
		return ;
	}

	void SetBrightness(float brightness)
	{
		material_->SetBrightness(brightness);
	}
	
	void SetContrast(float contrast)
	{
		material_->SetContrast(contrast);
	}
	
	Point3D GetTopLeftCornerPosition()
	{
		Cmiss_context_id cmissContext_ = cmguiManager_.GetCmissContext();
		Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);

		//Got to find the child region first!!
		Cmiss_region* region;
		if(!(region = Cmiss_region_find_subregion_at_path(root_region, sliceName_.c_str())))
		{
			//error
			std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
		}

		Cmiss_node* node;
		Point3D tlc;
		if (node = Cmiss_region_get_node(region, "3"))
		{
			FE_node_get_position_cartesian(node, 0, &(tlc.x), &(tlc.y), &(tlc.z), 0);
		}
		else
		{
			std::cout << "Error:\n";
			throw std::exception();
		}
		
		Cmiss_region_destroy(&region);
		Cmiss_region_destroy(&root_region);
		
		return tlc;
	}
	
	void TransformImagePlane(ImagePlane* plane)
	{
		int nodeNum = 1;

		Cmiss_context_id cmissContext_ = cmguiManager_.GetCmissContext();
		Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
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

		Cmiss_region_destroy(&region);
		Cmiss_region_destroy(&root_region);
	}
};


ImageSlice::ImageSlice(SliceInfo const& info, CmguiManager const& cmguiManager)
	:
	sliceName_(info.GetLabel()),
	oldIndex_(-1),
	isVisible_(true),
	cmguiManager_(cmguiManager),
	images_(info.GetDICOMImages()),
	textures_(info.GetTextures())
{
	size_t numberOfFrames = images_.size();
	for (size_t frame = 0; frame < numberOfFrames; ++ frame)
	{
		std::vector<ContourPtr>& contours = images_.at(frame)->GetContours();
		BOOST_FOREACH(ContourPtr& capContour, contours)
		{
//			capContour->ReadFromExFile(cmguiManager.GetCmissContext());
			double startTime = (double)frame / (double) numberOfFrames;
			double duration = (double)1.0 / numberOfFrames;
			double endTime = startTime + duration;
			capContour->SetValidPeriod(startTime, endTime);
			capContour->SetVisibility(true);
		}
	}
	
	pImpl_ = new CmguiImpl(cmguiManager_, sliceName_);
	this->TransformImagePlane();
}

ImageSlice::~ImageSlice()
{
	delete pImpl_;
	
	BOOST_FOREACH(Cmiss_texture* tex, textures_)
	{
//		Cmiss_texture* temp = tex;
//		DEACCESS(Texture)(&temp);
		DESTROY(Texture)(&tex);
	}
}

namespace {
	size_t MapTimeToIndex(double time, size_t totalNumberOfFrames)
	{
		size_t index = static_cast<int>(time * totalNumberOfFrames); // -1
		//boundary checks
		if (index >= totalNumberOfFrames)
		{
			index = totalNumberOfFrames - 1;
		}
		else if (index < 0)
		{
			index = 0;
		}
		
		return index;
	}
}

void ImageSlice::SetVisible(bool visibility)
{
	if (visibility)
	{
		isVisible_ = true;
//		Scene_object_set_visibility(sceneObject_, g_VISIBLE);
		oldIndex_ = -1; //this forces rebinding & redraw of the texture
		SetTime(time_);
	}
	else
	{
		isVisible_ = false;
//		Scene_object_set_visibility(sceneObject_, g_INVISIBLE);
	}
	pImpl_->SetVisible(visibility);
	
	//TEST
	// Set visibility of contours
	std::for_each(images_.begin(), images_.end(), 
			boost::bind(&DICOMImage::SetContourVisibility,_1,visibility));
	
	return;
}

void ImageSlice::SetTime(double time)
{
	time_ = time; // store time for later use

	size_t index = MapTimeToIndex(time, textures_.size());
	
	//update texture only when it is necessary
	if (index == oldIndex_|| !isVisible_)
	{
		return; 
	}
	oldIndex_ = index;
	
	//DEBUG
	//cout << "ImageSlice::setTime index = " << index << endl;
		
	Cmiss_texture* tex= textures_[index];
//	material_->ChangeTexture(tex);
	pImpl_->ChangeTexture(tex);
	return ;
}

void ImageSlice::SetBrightness(float brightness)
{
//	material_->SetBrightness(brightness);
	pImpl_->SetBrightness(brightness);
}

void ImageSlice::SetContrast(float contrast)
{
//	material_->SetContrast(contrast);
	pImpl_->SetContrast(contrast);
}

void ImageSlice::TransformImagePlane()
{
	// Now get the necessary info from the DICOM header
	
	assert(!images_.empty());
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
	
	pImpl_->TransformImagePlane(imagePlane_);
}

const ImagePlane& ImageSlice::GetImagePlane() const
{
	return *(imagePlane_);
}

void ImageSlice::SetShiftedImagePosition()
{	
	Point3D tlc = pImpl_->GetTopLeftCornerPosition(); // TODO error handling
	std::for_each(images_.begin(), images_.end(), boost::bind(&DICOMImage::SetImagePlaneTLC, _1, tlc));
}

} // end namespace cap
