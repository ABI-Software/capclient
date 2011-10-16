
#include "textureslice.h"

namespace cap
{

TextureSlice::TextureSlice(boost::shared_ptr<Material> material, std::vector<Cmiss_field_image_id> fieldImages)
	: material_(material)
	, fieldImages_(fieldImages)
{

}

TextureSlice::~TextureSlice()
{

}

void TextureSlice::ChangeTexture(unsigned int index)
{
	if (index >= 0 && index < fieldImages_.size())
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


