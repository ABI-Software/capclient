
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

}


