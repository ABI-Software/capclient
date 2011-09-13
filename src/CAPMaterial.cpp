/*
 * CAPMaterial.cpp
 *
 *  Created on: Jun 24, 2010
 *      Author: jchu014
 */
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <iomanip>

#include "CAPMaterial.h"

extern "C"
{
#include <api/cmiss_graphics_module.h>
#include <api/cmiss_graphics_material.h>
#include <api/cmiss_rendition.h>
}

#include "vert.prog.h"
#include "frag.prog.h"

namespace cap
{

CAPMaterial::CAPMaterial(const std::string& materialName, Cmiss_graphics_module_id graphics_module)
	: material_(0)
{
	//std::cout << "CAPMaterial::CAPMaterial" << std::endl;
	material_ = Cmiss_graphics_module_create_material(graphics_module);
	Cmiss_graphics_material_set_name(material_, materialName.c_str());
	Cmiss_graphics_material_set_attribute_integer(material_, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_IS_MANAGED, 1);

	// Initialize shaders that are used for adjusting brightness and contrast
	char *vertex_program_string = new char[vert_prog_len+1];
	snprintf(vertex_program_string, vert_prog_len+1, "%s", vert_prog);
	char *fragment_program_string = new char[frag_prog_len+1];
	snprintf(fragment_program_string, frag_prog_len+1, "%s", frag_prog);
	char *buffer = new char[51 + vert_prog_len + frag_prog_len + 1];
	snprintf(buffer, 51 + vert_prog_len + frag_prog_len + 1, "vertex_program_string \"%s\" fragment_program_string \"%s\"", vertex_program_string, fragment_program_string);
	Cmiss_graphics_material_execute_command(material_, buffer);
	Cmiss_graphics_material_execute_command(material_, "uniform_name contrast_raw uniform_value 0.5");
	Cmiss_graphics_material_execute_command(material_, "uniform_name brightness_raw uniform_value 0.5");
	delete[] buffer;
	delete[] vertex_program_string;
	delete[] fragment_program_string;
}

CAPMaterial::~CAPMaterial()
{
	std::cout << __func__ << std::endl;
	Cmiss_graphics_material_destroy(&material_);
	// TODO need to find out how to destroy material and texture properly
}

void CAPMaterial::SetBrightness(float brightness)
{
	assert(brightness >= 0 && brightness <=1);
	
	std::stringstream ss;
	ss << std::setprecision(5) << "uniform_name brightness_raw uniform_value " << brightness;
	Cmiss_graphics_material_execute_command(material_, ss.str().c_str());
}

void CAPMaterial::SetContrast(float contrast)
{
	assert(contrast >= 0 && contrast <=1);

	std::stringstream ss;
	ss << std::setprecision(5) << "uniform_name contrast_raw uniform_value " << contrast;
	Cmiss_graphics_material_execute_command(material_, ss.str().c_str());
}

void CAPMaterial::ChangeTexture(Cmiss_field_image_id mat)
{
	assert(material_);
	Cmiss_graphics_material_set_image_field(material_, 1, mat);
}


} //end namespace cap
