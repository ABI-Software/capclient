/*
 * CAPXMLFileHandler.h
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

class CAPXMLFile;
class DataPoint;
class HeartModel;
class SceneViewerPanel;

/**
 * Capxml file handler. 
 */
class CAPXMLFileHandler
{
public:

	/**
	 * Constructor.
	 *
	 * @param [in,out]	xmlFile	The xml file.
	 */
	CAPXMLFileHandler(CAPXMLFile& xmlFile);

	void AddLabelledSlices(const LabelledSlices& labelledSlices);
	void AddModellingPoints(const std::vector<ModellingPoint>& modellingPoints);

	/**
	 * Gets the labelled slices that are define in the CAPXMLFile.
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
	CAPXMLFile& xmlFile_;   /**< The xml file */
};

} // namespace cap

#endif /* CAPXMLFILEHANDLER_H_ */
