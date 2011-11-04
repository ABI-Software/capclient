#ifndef LABELLEDSLICES_H
#define LABELLEDSLICES_H

#include <vector>

#include "abstractlabelled.h"
#include "DICOMImage.h"

namespace cap
{
class LabelledSlice;
/**
 * Defines an alias representing the vector< labelled slice>
 */
typedef std::vector<LabelledSlice> LabelledSlices ;

/**
 * Labelled slice. 
 */
class LabelledSlice : public AbstractLabelled
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
	std::vector<DICOMPtr> dicomImages_; /**< The dicom images */
};

}

#endif // LABELLEDSLICES_H
