/*
 * CmguiManager.h
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#ifndef CMGUIMANAGER_H_
#define CMGUIMANAGER_H_

/** A singleton class to ease the interaction with the Cmgui API
 * 
 */
#include <cassert>
#include <string>
#include <vector>
#include <boost/tr1/memory.hpp>

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_graphics_material.h"
#include "api/cmiss_context.h"
}

#include "DICOMImage.h"

class wxPanel;
struct Scene_object;	

namespace cap
{

class CAPMaterial;

class CmguiManager
{
public:
	static CmguiManager* CreateCmguiManager()
	{
		if (instance_ == 0)
		{
			instance_ = new CmguiManager();
		}
		
		return instance_;
	}
	
	~CmguiManager();
	
	Cmiss_context_id GetCmissContext() const
	{
		return cmissContext_;
	}
	
	Cmiss_scene_viewer_id CreateSceneViewer(wxPanel* panel, std::string const& sceneName = "") const;
	
	void* LoadCmissTexture(std::string const& filename) const;
	
	/**
	 * Create a texture from list of images.  The dicoms listed in the images vector are 
	 * read in to the existing field_image.  That is the field_image has already been created 
	 * but it does not yet have any images read into it.
	 * 
	 * \param field_image the listed dicoms in images will be read into this field.
	 * \param images the vector of dicom images to read into the image field.
	 */
	void CreateCmissImageTexture(Cmiss_field_image_id field_image,const DICOMPtr& dicom_image) const;
	
	/**
	 * Read in the rectangular model files into the scene viewer.
	 */
	void ReadRectangularModelFiles(std::string const& modelName, std::string const& sceneName = "") const;
	
	/**
	 *  This method creates a cmiss material that uses shaders.
	 * 
	 * \param materialName the name of the material to create.
	 * \returns a boost shared pointer to a CAPMaterial object.
	 */
	std::tr1::shared_ptr<CAPMaterial> CreateCAPMaterial(std::string const& materialName) const;
	
	/**
	 * Get the next name in the field module that isn't currently in use.
	 * The supplied name is used as a base and numbers starting from one 
	 * are appended to produce a field name.  The field module is used to
	 * check that the field name is not currently in use, if it is the next 
	 * name in the series is checked until a field name not currently in use 
	 * is found.
	 * 
	 * \param field_module the field module to search for field names.
	 * \param name the base name of field.
	 * \returns a string formed from the base name with a number appended
	 */
	std::string GetNextNameInSeries(Cmiss_field_module_id field_module, std::string name);
	
	/**
	 * Get the field module for the given region.  The receiver of the field
	 * module will need to destroy the handle they receive.
	 * 
	 * \param regionName the region name to get the field module from.
	 * \returns an accessed field module for the region or 0 on failure.
	 */
	Cmiss_field_module_id GetFieldModuleForRegion(const std::string& regionName);
	
	// TODO move the following method out to a more suitable class
	Scene_object* AssignMaterialToObject(Cmiss_scene_viewer_id scene_viewer,
			Cmiss_graphics_material_id material, std::string const& regionName) const;

	/**
	 * Create a surface in the given region and uses the supplied material as a texture for the 
	 * surface created.
	 * 
	 * \param regionName is the region name given as a string.
	 * \param material the material to use on the created surface.
	 */
	void CreateTextureImageSurface( std::string const& regionName, Cmiss_graphics_material_id material) const;
	
	/**
	 * Create a unit square element in the given region with coordinates "coordinates".
	 * 
	 * \param regionName the name of the region to create the unit square element in.
	 */
	void CreatePlaneElement(const std::string& regionName);
	
	/**
	 * Resize a plane element (square finite elemet type) defined in the given region with
	 * coordinates "coordinates". This will resize the element only it is assumned that the
	 * element has been created already.
	 * 
	 * \param regionName the name of the region to resize the square element in.
	 * \param width the width to set the element to.
	 * \param height the height to set the element to.
	 */
	void ResizePlaneElement(const std::string& regionName, int width, int height);
	
	//void DestroyTexture(Cmiss_texture_id) const;
	void DestroyImageStack() const;
private:
	explicit CmguiManager();
	
	static CmguiManager* instance_;
	Cmiss_context_id cmissContext_;

};

} // end namespace cap
#endif /* CMGUIMANAGER_H_ */
