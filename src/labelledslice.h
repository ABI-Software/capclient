#ifndef LABELLEDSLICES_H
#define LABELLEDSLICES_H

#include <vector>

#include "abstractlabelled.h"
#include "DICOMImage.h"

namespace cap
{
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
	 * Copy constructor.
	 *
	 * @param	other	The other.
	 */
	LabelledSlice(const LabelledSlice& other);

	/**
	 * Destructor.
	 */
	virtual ~LabelledSlice();

	/**
	 * Assignment operator.
	 *
	 * @param	other	The other.
	 *
	 * @return	A shallow copy of this object.
	 */
	virtual LabelledSlice& operator=(const LabelledSlice& other);

	/**
	 * Equality operator.
	 *
	 * @param	other	The other.
	 *
	 * @return	true if the parameters are considered equivalent.
	 */
	virtual bool operator==(const LabelledSlice& other) const;

	/**
	 * Gets the dicom images.
	 *
	 * @return	The dicom images.
	 */
	const std::vector<DICOMPtr>& GetDicomImages() const { return dicomImages_; }
	
private:
	std::vector<DICOMPtr> dicomImages_; /**< The dicom images */
};

}

#endif // LABELLEDSLICES_H
