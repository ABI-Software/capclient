#ifndef LABELLEDTEXTURE_H
#define LABELLEDTEXTURE_H

extern "C"
{
	#include <api/cmiss_field_image.h>
}

namespace cap
{
	
	class LabelledTexture : public AbstractLabelled
	{

	public:
		LabelledTexture(const std::string& label, const std::vector<Cmiss_field_image_id>& textures);
		virtual ~LabelledTexture();
		virtual LabelledTexture& operator=(const LabelledTexture& other);
		
	private:
		std::vector<Cmiss_field_image_id> textures_;
	};

}

#endif // LABELLEDTEXTURE_H
