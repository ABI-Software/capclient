#ifndef TEXTURESLICE_H
#define TEXTURESLICE_H

#include <boost/shared_ptr.hpp>
#include <vector>

extern "C"
{
#include <api/cmiss_graphics_material.h>
#include <api/cmiss_field_image.h>
}

#include "material.h"

namespace cap
{

class TextureSlice
{

public:
	TextureSlice(boost::shared_ptr<Material> material, std::vector<Cmiss_field_image_id> fieldImages);
	virtual ~TextureSlice();
	
	Cmiss_graphics_material_id GetCmissMaterial() const {return material_->GetCmissMaterial(); }
	void ChangeTexture(Cmiss_field_image_id fieldImage) {material_->ChangeTexture(fieldImage); }
	
private:
	boost::shared_ptr<Material> material_;
	std::vector<Cmiss_field_image_id> fieldImages_;
};

}

#endif // TEXTURESLICE_H
