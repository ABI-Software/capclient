/*
 * CAPMaterial.h
 *
 *  Created on: Jun 23, 2010
 *      Author: jchu014
 */

#ifndef CAPMATERIAL_H_
#define CAPMATERIAL_H_

#include "CmguiExtensions.h"

#include <boost/noncopyable.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

extern "C"
{
#include "graphics/material.h"
}

struct Cmiss_texture;
typedef Cmiss_texture* Cmiss_texture_id;
struct Graphical_material;
typedef Graphical_material* Cmiss_material_id;;

namespace cap
{

/** Wrapper class around the Cmiss_material
 *  
 */
class CAPMaterial : public boost::noncopyable
{
public:
	explicit CAPMaterial(std::string const& materialName)
	:
	regionName_(materialName),
	material_(0),
	brightnessAndContrastTexture_(0)
	{
		if (materialName.empty())
		{
			return;
		}
		
		// User has to make sure each material has correct name as it is also used 
		// for the region to which this material is applied.
		material_ = create_Graphical_material(materialName.c_str());

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
				(char*) vp_stream.str().c_str(), (char*) fp_stream.str().c_str())
				)
		{
			cout << "Error: cant set material program strings" << endl;
		}
		
		// Initialize the texture used to pass brightness and contrast to fragment shader
		string brightnessAndContrastTextureName(materialName + "_BrightnessAndContrast"); 
		brightnessAndContrastTexture_ = CREATE(Texture)(brightnessAndContrastTextureName.c_str());
		unsigned char source_pixels[4] = {128,128,0,255};//TEST
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
	
	~CAPMaterial()
	{
		// Destroy member fields
//		DEACCESS(Graphical_material)(&material_);
//		DESTROY(Texture)(&brightnessAndContrastTexture_);
	}
	
	void SetBrightness(float brightness)
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
			std::cout << "ImageSlice::SetBrightness() Error setting pixel value to brightnessAndContrastTexture_\n";
		}
		Texture_notify_change(brightnessAndContrastTexture_);
	}
	
	void SetContrast(float contrast)
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
			std::cout << "ImageSlice::SetContrast() Error setting pixel value to brightnessAndContrastTexture_\n";
		}
		Texture_notify_change(brightnessAndContrastTexture_);
	}
	
	void SwitchTexture(Cmiss_texture_id tex); // needs cmiss_context to locate the root region!
	
	Cmiss_material_id GetCmissMaterial() const
	{
		return material_;
	}
	
private:
	std::string regionName_;
	Cmiss_material_id material_;
	Cmiss_texture_id brightnessAndContrastTexture_;
};

} // end namespace cap

#endif /* CAPMATERIAL_H_ */
