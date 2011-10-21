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
class CAPModelLVPS4X4;
class CmguiPanel;

class CAPXMLFileHandler
{
public:
	CAPXMLFileHandler(CAPXMLFile& xmlFile);
	/**
	 *  Populate member fields of CAPXMLFile from infomation obtained from
	 *  SlicesWithImages, vector<DataPoint> and CAPModelLVPS4X4
	 *  so an libxml2 tree can be generated and written to a file
	 */
	void ConstructCAPXMLFile(const LabelledSlices& labelledSlices, 
							std::vector<DataPoint> const& dataPoints,
							CAPModelLVPS4X4 const& model);
	
	/**
	 *  Translate the infomation stored in CAPXMLFile into the form to be
	 *  consumed by the client
	 *  (i.e SlicesWithImages, vector<DataPoint> and CAPModelLVPS4X4)
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

	std::vector<DataPoint> GetDataPoints(CmguiPanel *cmguiManager) const;

	void AddProvenanceDetail(std::string const& comment);
	
private:
	CAPXMLFile& xmlFile_;
};

} // namespace cap

#endif /* CAPXMLFILEHANDLER_H_ */
