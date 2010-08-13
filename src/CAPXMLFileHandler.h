/*
 * CAPXMLFileHandler.h
 *
 *  Created on: Aug 13, 2010
 *      Author: jchu014
 */

#ifndef CAPXMLFILEHANDLER_H_
#define CAPXMLFILEHANDLER_H_

//#include "CAPMath.h"
#include "CAPXMLFile.h"

namespace cap
{

//#include "CAPTypes.h"

//class CAPXMLFile;

class CAPXMLFileHandler
{
public:
	CAPXMLFileHandler(CAPXMLFile& xmlFile);
	/**
	 *  Populate member fields of CAPXMLFile from infomation obtained from
	 *  SlicesWithImages, vector<DataPoint> and CAPModelLVPS4X4
	 *  so an libxml2 tree can be generated and written to a file
	 */
	void ContructCAPXMLFile(SlicesWithImages const& dicomFiles, 
							std::vector<DataPoint> const& dataPoints,
							CAPModelLVPS4X4 const& model);
	
	/**
	 *  Translate the infomation stored in CAPXMLFile into the form to be
	 *  consumed by the client
	 *  (i.e SlicesWithImages, vector<DataPoint> and CAPModelLVPS4X4)
	 *  Also generated the instances of DICOMImage, Cmiss_texture,
	 *  DataPoint (and the Cmiss_node) and the model.
	 */
	SlicesWithImages GetSlicesWithImages(CmguiManager const& cmguiManager) const;

	std::vector<DataPoint> GetDataPoints(CmguiManager const& cmguiManager) const;

//	std::vector<std::string> GetExnodeFileNames() const;
//
//	std::string const& GetExelemFileName() const;
//
//	void GetTransformationMatrix(gtMatrix& mat) const;
//
//	double GetFocalLength() const;

//	std::vector<Image> const& GetImages() const
//	{
//		return input_.images;
//	}
private:
	CAPXMLFile& xmlFile_;
};

} // namespace cap

#endif /* CAPXMLFILEHANDLER_H_ */
