#ifndef TEXTURESLICE_H
#define TEXTURESLICE_H

#include "material.h"

extern "C"
{
#include <zn/cmiss_graphics_material.h>
#include <zn/cmiss_field_image.h>
}

#include <vector>

namespace cap
{
	/**
	 * @brief Texture slice.  A texture slice lives in the CAPClientWindow it is partnered to the
	 * LabelledSlices which live in the CAPClient.
	 */
	class TextureSlice
	{

	public:

		/**
		 * Constructor.
		 *
		 * @param	material   	The material.
		 * @param	fieldImages	The field images.
		 */
		TextureSlice(Material *material, std::vector<Cmiss_field_image_id> fieldImages);

		/**
		 * Destructor.
		 */
		virtual ~TextureSlice();

		/**
		 * Gets the cmiss material.
		 *
		 * @return	The cmiss material.
		 */
		Cmiss_graphics_material_id GetCmissMaterial() const {return material_->GetCmissMaterial(); }

		/**
		 * Sets a contrast.
		 *
		 * @param	contrast	The contrast.
		 */
		void SetContrast(float contrast) {material_->SetContrast(contrast); }

		/**
		 * Sets the brightness.
		 *
		 * @param	brightness	The brightness.
		 */
		void SetBrightness(float brightness) {material_->SetBrightness(brightness); }

		/**
		 * Change texture to use the given field image.
		 *
		 * @param	fieldImage	The field image.
		 */
		void ChangeTexture(Cmiss_field_image_id fieldImage) {material_->ChangeTexture(fieldImage); }

		/**
		 * Change texture to use the field image at the given index.  If the
		 * index is not valid the texture will not be changed.
		 *
		 * @param	index	Zero-based index into the field image stack.
		 */
		void ChangeTexture(unsigned int index);

		/**
		 * Change texture nearest to the given value.  The value is expected
		 * to be within the range [0, 1] and will be pegged to this range.
		 *
		 * @param	value	The value.
		 */
		void ChangeTextureNearestTo(double value);
		
	private:
		Material *material_;  /**< The material */
		std::vector<Cmiss_field_image_id> fieldImages_; /**< The field images */
};

}

#endif // TEXTURESLICE_H
