/*
 * CAPXMLFileHandler.h
 *
 *  Created on: Aug 13, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILEHANDLER_H_
#define CAPXMLFILEHANDLER_H_

#include "SliceInfo.h"
#include "labelledslice.h"
#include <string>

namespace cap
{

class CAPXMLFile;
class DataPoint;
class HeartModel;
class CmguiPanel;

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

	/**
	 *  Populate member fields of CAPXMLFile from infomation obtained from
	 *  SlicesWithImages, vector<DataPoint> and HeartModel
	 *  so an libxml2 tree can be generated and written to a file
	 */
	void ConstructCAPXMLFile(const LabelledSlices& labelledSlices, 
							std::vector<DataPoint> const& dataPoints,
							HeartModel const& model);
	
	/**
	 *  Translate the infomation stored in CAPXMLFile into the form to be
	 *  consumed by the client
	 *  (i.e SlicesWithImages, vector<DataPoint> and HeartModel)
	 *  Also generated the instances of DICOMImage, Cmiss_texture,
	 *  DataPoint (and the Cmiss_node) and the model.
	 */
	SlicesWithImages GetSlicesWithImages(CmguiPanel *cmguiManager) const;

	/**
	 * Gets the labelled slices that are define in the CAPXMLFile.
	 *
	 * @return	The labelled slices.
	 */
	LabelledSlices GetLabelledSlices() const;

	/**
	 * Gets the data points.
	 *
	 * @return	The data points.
	 */
	std::vector<DataPoint> GetDataPoints() const;

	/**
	 * Adds a provenance detail. 
	 *
	 * @param	comment	The comment.
	 */
	void AddProvenanceDetail(std::string const& comment);
	
private:
	CAPXMLFile& xmlFile_;   /**< The xml file */
};

} // namespace cap

#endif /* CAPXMLFILEHANDLER_H_ */
