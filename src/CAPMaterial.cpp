/*
 * CAPMaterial.cpp
 *
 *  Created on: Jun 24, 2010
 *      Author: jchu014
 */
#include "CAPMaterial.h"

#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>

extern "C"
{
#include "api/cmiss_graphics_module.h"
#include "graphics/material.h"
#include "graphics/cmiss_rendition.h"
}

namespace cap
{

CAPMaterial::CAPMaterial(std::string const& materialName, Cmiss_graphics_module_id graphics_module)
	:
//	materialName_(materialName),
	material_(0),
	brightnessAndContrastTexture_(0),
	graphicsModule_(graphics_module)
{	
	material_ = Cmiss_graphics_module_create_material(graphics_module);
	Cmiss_material_set_name(material_, materialName.c_str());

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
	
	if (!Material_set_material_program_strings(material_, 
			(char*) vp_stream.str().c_str(), (char*) fp_stream.str().c_str()))
	{
		cout << "Error: cant set material program strings" << endl;
	}
	
	// Initialize the texture used to pass brightness and contrast to fragment shader
	string brightnessAndContrastTextureName(materialName + "_BrightnessAndContrast"); 
	brightnessAndContrastTexture_ = CREATE(Texture)(brightnessAndContrastTextureName.c_str());
	Cmiss_texture_manager *texture_manager = Cmiss_graphics_module_get_texture_manager(graphics_module);
	ADD_OBJECT_TO_MANAGER(Texture)(brightnessAndContrastTexture_, texture_manager);
	
	unsigned char source_pixels[4] = {128,128,0,255};//Fisrt two bytes define the brightness and contrast respectively
	if (!Cmiss_texture_set_pixels(brightnessAndContrastTexture_,
		1 /*int width */, 1/*int height*/, 1/*int depth*/,
		4 /*int number_of_components*/, 1 /*int number_of_bytes_per_component*/,
		4 /*int source_width_bytes*/, source_pixels))
	{
		//Error
		cout << "ImageSlice::ImageSlice() Error setting pixel value to brightnessAndContrastTexture_" << endl;
	}

	Graphical_material_set_second_texture(material_, brightnessAndContrastTexture_);
}

CAPMaterial::~CAPMaterial()
{
	std::cout << __func__ << '\n';
	// Destroy member fields
	// TODO need to find out how to destroy material and texture properly	
	
//	int pers = Cmiss_material_get_persistent(material_);
//	std::cout << "Pers = " << pers << '\n';
	
	Cmiss_material_set_persistent(material_, 0);
	Cmiss_material_id tempMaterial1 = material_;
	Cmiss_material_id tempMaterial2 = material_;
	Cmiss_material_destroy(&tempMaterial1);
	Cmiss_material_destroy(&tempMaterial2);
	Cmiss_material_destroy(&material_);
	
//	Cmiss_texture_id tex1 = brightnessAndContrastTexture_;
//	DEACCESS(Texture)(&tex1);
	struct MANAGER(Texture) * texture_manager = 
			Cmiss_graphics_module_get_texture_manager(graphicsModule_);
	REMOVE_OBJECT_FROM_MANAGER(Texture)(brightnessAndContrastTexture_, texture_manager);
//	DESTROY(Texture)(&brightnessAndContrastTexture_);
}

void CAPMaterial::SetBrightness(float brightness)
{
	unsigned char pixels[4];
	unsigned char fill[1] = {0};
	Cmiss_texture_get_pixels(brightnessAndContrastTexture_,
		0, 0, 0,
		1, 1, 1,
		0, 0, 
		fill,
		4, pixels);
	
	assert(brightness >= 0 && brightness <=1);
	pixels[0] = static_cast<unsigned char>(255.0 * brightness);
	
	if (!Cmiss_texture_set_pixels(brightnessAndContrastTexture_,
		1 /*int width */, 1/*int height*/, 1/*int depth*/,
		4 /*int number_of_components*/, 1 /*int number_of_bytes_per_component*/,
		4 /*int source_width_bytes*/, pixels))
	{
		//Error
		std::cout << "ImageSlice::SetBrightness() Error setting pixel value to brightnessAndContrastTexture_\n";
	}
	Texture_notify_change(brightnessAndContrastTexture_);
	// The call to Graphical_material_set_second_texture seems to be necessary to make cmgui respond to the change of texture
	Graphical_material_set_second_texture(material_, brightnessAndContrastTexture_);
}

void CAPMaterial::SetContrast(float contrast)
{
	unsigned char pixels[4];
	unsigned char fill[1] = {0};
	Cmiss_texture_get_pixels(brightnessAndContrastTexture_,
		0, 0, 0,
		1, 1, 1,
		0, 0, 
		fill,
		4, pixels);
	
	assert(contrast >= 0 && contrast <=1);
	pixels[1] = static_cast<unsigned char>(255.0 * contrast);
	
	if (!Cmiss_texture_set_pixels(brightnessAndContrastTexture_,
		1 /*int width */, 1/*int height*/, 1/*int depth*/,
		4 /*int number_of_components*/, 1 /*int number_of_bytes_per_component*/,
		4 /*int source_width_bytes*/, pixels))
	{
		//Error
		std::cout << "ImageSlice::SetContrast() Error setting pixel value to brightnessAndContrastTexture_\n";
	}
	Texture_notify_change(brightnessAndContrastTexture_);
	// The call to Graphical_material_set_second_texture seems to be necessary to make cmgui respond to the change of texture
	Graphical_material_set_second_texture(material_, brightnessAndContrastTexture_);
}

void CAPMaterial::ChangeTexture(Cmiss_texture_id tex)
{	
	if (material_)
	{			
		Graphical_material_set_texture(material_,tex);
//		if (!Cmiss_material_set_texture(material_,tex))
//		{
//			//Error
//			std::cout << "Error: Cmiss_material_set_texture()\n";
//		}
	}
	else
	{
		std::cout << __func__ << " - Error: null material pointer\n";
	}
}


} //end namespace cap
