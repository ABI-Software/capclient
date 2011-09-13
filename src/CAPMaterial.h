/*
 * CAPMaterial.h
 *
 *  Created on: Jun 23, 2010
 *      Author: jchu014
 */

#ifndef CAPMATERIAL_H_
#define CAPMATERIAL_H_

#include <boost/noncopyable.hpp>
#include <string>

extern "C" {
#include <api/cmiss_graphics_module.h>
#include <api/cmiss_graphics_material.h>
}

#include "CmguiExtensions.h"

namespace cap
{

/** 
 * Wrapper class around the Cmiss_graphics_material.  The graphics material is
 * used as a texture, by setting an image field into the material the image can
 * be displayed in a surface.  The surface needs to set it's material to make the
 * image visible.
 * 
 * With a graphics material vertex and fragment program shaders can be set to get
 * fast renditions of changes to values like brightness and contrast.
 */
class CAPMaterial : boost::noncopyable
{
public:
	/**
	 * Constructor creates a material from the provided graphics_module
	 * and gives it the provided name.
	 * Vertex and fragment programs are set to control the contrast and brightness of 
	 * the material.
	 * 
	 * \param materialName a string holding the name of the material.
	 * \param graphics_module the graphics module to create the material in.
	 */
	CAPMaterial(const std::string& materialName, Cmiss_graphics_module_id graphics_module);
	
	/**
	 * Destructor, destroy references to cmiss ids.
	 */
	~CAPMaterial();
	
	/**
	 * Set the brightness.  The value of the brightness must be between 
	 * zero and one.
	 * 
	 * \param brightness a float between zero and one, inclusive.
	 */
	void SetBrightness(float brightness);
	
	/**
	 * Set the contrast.  The value of the contrast must be between 
	 * zero and one.
	 * 
	 * \param contrast a float between zero and one, inclusive.
	 */
	void SetContrast(float contrast);
	
	/**
	 * Change the texture to the supplied field image.
	 * 
	 * \param mat the new field image to use.
	 */
	void ChangeTexture(Cmiss_field_image_id mat);
	
	/**
	 * Get the  cmiss material
	 * 
	 * \returns an accessed reference to the cmiss material, this will
	 * need to be destroyed by the receiver.
	 */
	Cmiss_graphics_material_id GetCmissMaterial() const
	{
		Cmiss_graphics_material_access(material_);
		return material_;
	}
	
private:
	Cmiss_graphics_material_id material_; /**< Cmiss graphics material */
};

} // end namespace cap

#endif /* CAPMATERIAL_H_ */
