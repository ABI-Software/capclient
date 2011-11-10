/*
 * Material.cpp
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
#ifdef _MSC_VER
# define snprintf _snprintf
#endif

extern "C"
{
#include <api/cmiss_graphics_module.h>
#include <api/cmiss_graphics_material.h>
#include <api/cmiss_rendition.h>
#include <api/cmiss_core.h>
}

#include "capclientconfig.h"
#include "material.h"
#include "hexified/vert.prog.h"
#include "hexified/frag.prog.h"
#include "utils/debug.h"
#include "filesystem.h"

namespace cap
{

Material::Material(const std::string& materialName, Cmiss_graphics_module_id graphics_module)
	: material_(0)
{
	//std::cout << "Material::Material" << std::endl;
	material_ = Cmiss_graphics_module_create_material(graphics_module);
	Cmiss_graphics_material_set_name(material_, materialName.c_str());
	Cmiss_graphics_material_set_attribute_integer(material_, CMISS_GRAPHICS_MATERIAL_ATTRIBUTE_IS_MANAGED, 1);

	// Initialize shaders that are used for adjusting brightness and contrast
	std::string vertex_program = FileSystem::WriteCharBufferToString(vert_prog, vert_prog_len);
	std::string fragment_program = FileSystem::WriteCharBufferToString(frag_prog, frag_prog_len);

	std::string buffer = "vertex_program_string \"" + vertex_program + "\" fragment_program_string \"" + fragment_program + "\"";
	Cmiss_graphics_material_execute_command(material_, buffer.c_str());
	Cmiss_graphics_material_execute_command(material_, "uniform_name contrast_raw uniform_value 0.5");
	Cmiss_graphics_material_execute_command(material_, "uniform_name brightness_raw uniform_value 0.5");
}

Material::~Material()
{
	char *name = Cmiss_graphics_material_get_name(material_);
	dbg(std::string(__func__) + ": " + std::string(name));
	Cmiss_deallocate(name);
	Cmiss_graphics_material_destroy(&material_);
	// TODO need to find out how to destroy material and texture properly
}

void Material::SetBrightness(float brightness)
{
	assert(brightness >= 0 && brightness <=1);
	
	std::stringstream ss;
	ss << std::setprecision(5) << "uniform_name brightness_raw uniform_value " << brightness;
	Cmiss_graphics_material_execute_command(material_, ss.str().c_str());
}

void Material::SetContrast(float contrast)
{
	assert(contrast >= 0 && contrast <=1);

	std::stringstream ss;
	ss << std::setprecision(5) << "uniform_name contrast_raw uniform_value " << contrast;
	Cmiss_graphics_material_execute_command(material_, ss.str().c_str());
}

void Material::ChangeTexture(Cmiss_field_image_id mat)
{
	assert(material_);
	Cmiss_graphics_material_set_image_field(material_, 1, mat);
}

Cmiss_graphics_material_id Material::GetCmissMaterial() const
{
	Cmiss_graphics_material_access(material_);
	return material_;
}

} //end namespace cap
