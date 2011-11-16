
#include "textureslice.h"
#include "utils/debug.h"

namespace cap
{

TextureSlice::TextureSlice(boost::shared_ptr<Material> material, std::vector<Cmiss_field_image_id> fieldImages)
	: material_(material)
	, fieldImages_(fieldImages)
{

}

TextureSlice::~TextureSlice()
{
	dbg("TextureSlice::~TextureSlice()");
	std::vector<Cmiss_field_image_id>::iterator it = fieldImages_.begin();
	for (; it != fieldImages_.end(); it++)
		Cmiss_field_image_destroy(&(*it));

	fieldImages_.clear();
}

void TextureSlice::ChangeTexture(unsigned int index)
{
	if (index < fieldImages_.size())
	{
		material_->ChangeTexture(fieldImages_.at(index));
	}
}

void TextureSlice::ChangeTextureNearestTo(double value)
{
	if (value < 0.0) value = 0.0;
	if (value > 1.0) value = 1.0;

	unsigned int index = static_cast<unsigned int>(value*(fieldImages_.size()-1)+0.5);
	ChangeTexture(index);
}

}


