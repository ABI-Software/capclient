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

namespace cap
{

class ModelFile;
class DataPoint;
class HeartModel;
class SceneViewerPanel;

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
	XMLFileHandler(ModelFile& xmlFile);

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
	 * Gets the labelled slices.
	 *
	 * @return	The labelled slices.
	 */
	LabelledSlices GetLabelledSlices() const;

	/**
	 * Gets the modelling points.
	 *
	 * @return	The modelling points.
	 */
	ModellingPoints GetModellingPoints() const;

	/**
	 * Adds a provenance detail. 
	 *
	 * @param	comment	The comment.
	 */
	void AddProvenanceDetail(std::string const& comment);

	/**
	 * Clears the xmlFile of all input and output information.
	 */
	void Clear();
	
private:
	ModelFile& xmlFile_;   /**< The xml file */
};

} // namespace cap

#endif /* CAPXMLFILEHANDLER_H_ */
