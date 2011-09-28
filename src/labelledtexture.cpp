
#include <string>
#include <vector>

#include "labelledtexture.h"

namespace cap
{

LabelledTexture::LabelledTexture(const std::string& label, const std::vector<Cmiss_field_image_id>& textures)
	: AbstractLabelled(label)
	, textures_(textures)
{

}

LabelledTexture::~LabelledTexture()
{

}

LabelledTexture& LabelledTexture::operator=(const LabelledTexture& other)
{
	this->label_ = other.label_;
	return *this;
}

}


