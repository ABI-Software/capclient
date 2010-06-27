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

struct Cmiss_texture;
typedef Cmiss_texture* Cmiss_texture_id;
struct Graphical_material;
typedef Graphical_material* Cmiss_material_id;;
struct Cmiss_graphics_module;
typedef Cmiss_graphics_module* Cmiss_graphics_module_id;

namespace cap
{

/** Wrapper class around the Cmiss_material
 *  
 */
class CAPMaterial : public boost::noncopyable
{
public:
	CAPMaterial(std::string const& materialName, Cmiss_graphics_module_id graphics_module);
	
	~CAPMaterial();
	
	void SetBrightness(float brightness);
	
	void SetContrast(float contrast);
	
	void ChangeTexture(Cmiss_texture_id tex);
	
	Cmiss_material_id GetCmissMaterial() const
	{
		return material_;
	}
	
private:
//	std::string materialName_;
	Cmiss_material_id material_;
	Cmiss_texture_id brightnessAndContrastTexture_;
	Cmiss_graphics_module_id graphicsModule_;
};

} // end namespace cap

#endif /* CAPMATERIAL_H_ */
