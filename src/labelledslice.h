#ifndef LABELLEDSLICES_H
#define LABELLEDSLICES_H

#include <string>
#include <vector>

#include "dicomimage.h"

namespace cap
{
	/**
	 * Labelled slice.
	 */
	class LabelledSlice
	{
	public:

		/**
		 * Constructor.
		 *
		 * @param	label 	The label.
		 * @param	dicoms	The dicoms.
		 */
		LabelledSlice(const std::string& label, std::vector<DICOMPtr> dicoms);

		/**
		 * Destructor.
		 */
		virtual ~LabelledSlice();

		/**
		 * Get the label for this class.
		 *
		 * \returns a const std::string reference.
		 */
		const std::string& GetLabel() const { return label_; }

		/**
		 * Get the index of the image with the given sopiuid.
		 *
		 * @param sopiuid   The sopiuid to match.
		 * @return -1 if sopiuid not found, otherwise return the index of the image.
		 */
		int IndexOf(std::string sopiuid) const;

		Matrix4x4 GetTransform() const;

		/**
		 * Gets the dicom images.
		 *
		 * @return	The dicom images.
		 */
		const std::vector<DICOMPtr>& GetDICOMImages() const { return dicomImages_; }

		/**
		 * Appends a dicomImage.
		 *
		 * @param	dicomImage	The DICOMPtr to append.
		 */
		void append(DICOMPtr dicomImage) { dicomImages_.push_back(dicomImage); }

	private:
		std::string label_; /**< string label to used to identify the item. */
		std::vector<DICOMPtr> dicomImages_; /**< The dicom images */
	};

	/**
	 * \brief Labelled sort order overrides the operator() so that
	 * we can effect the sorting of the class.  In this case
	 * we are looking to order the short axis slices first and
	 * then the long axis slices.  Also we want them to be sorted
	 * in increasing numerical order.
	 *
	 * This class is passed to the third parameter of std:sort when
	 * sorting classes derived from AbstractLabelled.
	 */
	class LabelledSortOrder
	{
	public:
		/**
		 * Overriding the operator() so that we can implement a propreitory
		 * sort order.  If the first letters of the given labels are not the
		 * same then sort reverse alphabetically, else sort on string length
		 * and then on the normal string comparison rules.
		 *
		 * This has the effect of favouring the short axis over the long axis and
		 * it makes lower numerical numbers appear first.  So that we get the
		 * correct numerical ordering.
		 */
		bool operator()(const LabelledSlice& a, const LabelledSlice& b) const
		{
			const std::string& x = a.GetLabel();
			const std::string& y = b.GetLabel();
			if (x[0] != y[0])
			{
				return x[0] > y[0];
			}

			return std::make_pair(x.length(), x) < std::make_pair(y.length(), y);
		}
	};

	/**
	 * Defines an alias representing the vector< labelled slice>
	 */
	typedef std::vector<LabelledSlice> LabelledSlices;

}

#endif // LABELLEDSLICES_H
