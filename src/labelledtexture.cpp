
#include <string>
#include <vector>

#include "labelledtexture.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

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


