/*
 * XMLFileHandler.h
 *
 *  Created on: Aug 13, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILEHANDLER_H_
#define CAPXMLFILEHANDLER_H_

#include <string>

#include "labelledslice.h"
#include "model/modellingpoint.h"
#include "io/modelfile.h"

namespace cap
{

struct CardiacAnnotation;

/**
 * @brief Xml file handler.
 */
class XMLFileHandler
{
public:

	/**
	 * Constructor.
	 *
	 * @param [in,out]	xmlFile	The xml file.
	 */
	explicit XMLFileHandler(ModelFile& xmlFile);

	/**
	 * Adds labelled slices.
	 *
	 * @param	labelledSlices	The labelled slices.
	 */
	void AddLabelledSlices(const LabelledSlices& labelledSlices);

	/**
	 * Adds modelling points.
	 *
	 * @param	modellingPoints	The modelling points.
	 */
	void AddModellingPoints(const ModellingPoints& modellingPoints);

	/**
	 * Adds a provenance detail.
	 *
	 * @param	comment	The comment.
	 */
	void AddProvenanceDetail(std::string const& comment);

	/**
	 * Adds a cardiac annotation.  If the LabelledSlices have already been added then any cardiac
	 * annotations not in the current image set will be discarded.
	 *
	 * @param	annotation	The annotation.
	 */
	void AddCardiacAnnotation(const CardiacAnnotation& annotation);

	/**
	 * Adds the study contours.
	 *
	 * @param studyContours	The study contours.
	 */
	void AddStudyContours(const ModelFile::StudyContours& studyContours);

	/**
	 * Gets the labelled slices.
	 *
	 * @param uidToDICOMPtrMap  The map of uid to DICOMPtr.
	 * @return	The labelled slices.
	 */
	LabelledSlices GetLabelledSlices(const HashTable& uidToDICOMPtrMap) const;

	/**
	 * Get all the Sopiuids listed in the xmlFile_.
	 *
	 * @return A vector of all the Sopiuid strings in the file.
	 */
	std::vector<std::string> GetSopiuids();

	/**
	 * Gets the details of the modelling points.
	 *
	 * @return	The modelling point details.
	 */
	ModellingPointDetails GetModellingPointDetails() const;

	/**
	 * Gets the cardiac annotation.
	 *
	 * @return	The cardiac annotation.
	 */
	CardiacAnnotation GetCardiacAnnotation() const;

	/**
	 * Gets the first comment provenance detail.
	 * @return The comment detail string.
	 */
	std::string GetProvenanceDetail() const;

	/**
	 * Get the study contours.  If the study contours study instance
	 * uid doesn't match that of the containing input then the StudyContours
	 * will be empty.
	 *
	 * @return The study contours.
	 */
	ModelFile::StudyContours GetStudyContours() const;

	/**
	 * Clears the xmlFile of all input and output information.
	 */
	void Clear();

private:
	ModelFile& xmlFile_;   /**< The xml file */
};

} // namespace cap

#endif /* CAPXMLFILEHANDLER_H_ */
