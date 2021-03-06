/*
 * XMLFileHandler.cpp
 *
 *  Created on: Aug 13, 2010
 *      Author: jchu014
 */

#include "io/xmlfilehandler.h"

#include <wx/wx.h>
//#include <wx/dir.h> // FIXME move this out to a separate function/class
#include <wx/dirdlg.h>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <assert.h>
#include <functional>
#include <time.h>
#include <float.h>

#include "capclientconfig.h"
#include "utils/debug.h"
#include "io/modelfile.h"
#include "dicomimage.h"
#include "model/heart.h"
#include "zinc/sceneviewerpanel.h"
#include "utils/filesystem.h"
#include "platforminfo.h"
#include "contour.h"
#include "logmsg.h"
#include "utils/misc.h"

namespace cap
{

XMLFileHandler::XMLFileHandler(ModelFile& xmlFile)
	: xmlFile_(xmlFile)
{}

void XMLFileHandler::Clear()
{
	xmlFile_.ClearInputAndOutput();
}

void XMLFileHandler::AddLabelledSlices(const LabelledSlices& labelledSlices)
{
	if (labelledSlices.empty())
	{
		dbg("XMLFileHandler::AddLabelledSlices : No dicom files to construct ModelFile from");
		return;
	}

	std::string studyIUid = labelledSlices[0].GetDICOMImages()[0]->GetStudyInstanceUID();
	xmlFile_.SetStudyInstanceUID(studyIUid);
	std::string const& filename = xmlFile_.GetFilename();
	size_t positionOfLastSlash = filename.find_last_of("/\\");
//	std::cout << "positionOfLastSlash = " << positionOfLastSlash << std::endl;
	std::string name = filename.substr(positionOfLastSlash+1);
	xmlFile_.SetName(name);
	xmlFile_.SetChamber("LV");

	// CAPXMLInput
	int slice = 0;

	typedef std::pair<Vector3D, Vector3D> Orientation;
	BOOST_FOREACH(const LabelledSlice& labelledSlice, labelledSlices)
	{
		std::string const& label = labelledSlice.GetLabel();
		std::vector<DICOMPtr> const& dicomFiles = labelledSlice.GetDICOMImages();

		int frame = 0;
		BOOST_FOREACH(DICOMPtr const& dicomFile, dicomFiles)
		{
			ModelFile::Image image;
			image.sopiuid = dicomFile->GetSopInstanceUID();
			image.seriesiuid = dicomFile->GetSeriesInstanceUID();
			image.label = label;
			image.frame = frame++;
			image.slice = slice;
			image.imageOrientation = boost::shared_ptr<Orientation>();
			image.imagePosition = boost::shared_ptr<Point3D>();;
			image.points = std::vector<ModelFile::Point>();
			// if the images have been shifted for mis-registraion correction,
			// put the new position and orientation in each image element
			// TODO : This is really a per-slice attribute rather than per image.
			//        Need to change the xml file schema accordingly ??

			if (dicomFile->IsShifted())
			{
				Point3D const& pos = dicomFile->GetImagePosition();
				image.imagePosition = boost::make_shared<Point3D>(pos);
			}

			if (dicomFile->IsRotated())
			{
				Orientation ori = dicomFile->GetImageOrientation();
				image.imageOrientation = boost::make_shared<Orientation>(ori);
			}

//			BOOST_FOREACH(ContourPtr& contour, dicomFile->GetContours())
//			{
//				std::string const& filename = contour->GetFilename();
//				size_t positionOfLastSlash = filename.find_last_of("/\\");
//				std::string baseName = filename.substr(positionOfLastSlash + 1);
//				int number = contour->GetContourNumber();
//				ModelFile::ContourFile contourFile = {baseName, number};
//				image.contourFiles.push_back(contourFile);
//			}

			//image.points; // FIX?
			xmlFile_.AddImage(image);
		}
		slice++;
	}

}

void XMLFileHandler::AddModellingPoints(const std::vector<ModellingPoint>& modellingPoints)
{
	std::vector<ModellingPoint>::const_iterator it = modellingPoints.begin();
	for(;it != modellingPoints.end(); ++it)
	{
		ModelFile::Point p;
		p.surface = it->GetHeartSurfaceType();
		p.type = it->GetModellingPointType();
		const Point3D& position = it->GetPosition();
		ModelFile::Value x = {position.x, "x"};
		p.values["x"] = x; //REVISE
		ModelFile::Value y = {position.y, "y"};
		p.values["y"] = y;
		ModelFile::Value z = {position.z, "z"};
		p.values["z"] = z;

		p.time = it->GetTime();

		xmlFile_.AddPoint(p);
	}
}

std::vector<std::string> XMLFileHandler::GetSopiuids()
{
	return xmlFile_.GetSopiuids();
}

LabelledSlices XMLFileHandler::GetLabelledSlices(const HashTable& uidToDICOMPtrMap) const
{
	LabelledSlices labelledSlices;
	typedef std::map<std::string, LabelledSlice > LabelledSliceMap;
	LabelledSliceMap labelledSliceMap;

	ModelFile::Input& input = xmlFile_.GetInput();
	BOOST_FOREACH(ModelFile::Image const& image, input.images)
	{
		HashTable::const_iterator dicomFileItr = uidToDICOMPtrMap.find(image.sopiuid);
		DICOMPtr dicomImage = dicomFileItr->second;
		if (image.imagePosition)
		{
			Point3D const& pos = *image.imagePosition;
			dicomImage->SetImagePosition(pos);
		}

		// Create Contour objects
//        ModelFile::StudyContours const& studyContours = input.studyContours;
//        std::vector<ModelFile::ImageContours>::const_iterator imageContours_itr =
//                std::find_if(studyContours.listOfImageContours.begin(),
//                            studyContours.listOfImageContours.end(),
//                            boost::bind(std::equal_to<std::string>(),
//                                    boost::bind(&ModelFile::ImageContours::sopiuid, _1),
//                                    image.sopiuid));
//        if (imageContours_itr != studyContours.listOfImageContours.end())
//        {
//            ModelFile::ImageContours const& imageContours = *imageContours_itr;
//            BOOST_FOREACH(ModelFile::Contour const& contour, imageContours.contours)
//            {
//                std::vector<Point3D> points;
//                points.reserve(contour.contourPoints.size());
//                BOOST_FOREACH(ModelFile::ContourPoint const& contourPoint, contour.contourPoints)
//                {
//                    points.push_back(Point3D(contourPoint.x, contourPoint.y, 0));
//                }
//                ContourPtr capContour = boost::make_shared<Contour>(contour.number, contour.transformationMatrix, points);
//                dicomImage->AddContour(capContour);
//            }
//        }

		LabelledSliceMap::iterator it = labelledSliceMap.find(image.label);
		if (it == labelledSliceMap.end())
		{
			std::vector<DICOMPtr> dicoms;
			dicoms.push_back(dicomImage);
			LabelledSlice newSlice = LabelledSlice(image.label, dicoms);
			labelledSliceMap.insert(std::make_pair(image.label, newSlice));
		}
		else
		{
			it->second.append(dicomImage);
		}

	}

	LabelledSliceMap::const_iterator cit = labelledSliceMap.begin();
	for (;cit != labelledSliceMap.end(); cit++)
	{
		labelledSlices.push_back(cit->second);
	}

	std::sort(labelledSlices.begin(), labelledSlices.end(), LabelledSortOrder());

	return labelledSlices;
}

ModellingPointDetails XMLFileHandler::GetModellingPointDetails() const
{
	ModellingPointDetails modellingPointDetails;
	ModelFile::Input& input = xmlFile_.GetInput();
	std::vector<ModelFile::Point>::const_iterator cit = input.points.begin();
	for (; cit != input.points.end(); ++cit)
	{
		const ModelFile::Point& p = *cit;
		double coords[3];
		coords[0] = (*p.values.find("x")).second.value;
		coords[1] = (*p.values.find("y")).second.value;
		coords[2] = (*p.values.find("z")).second.value;

		Point3D position(coords);
		ModellingPointDetail mp(p.type, position, p.time);

		modellingPointDetails.push_back(mp);
	}

	return modellingPointDetails;
}

void XMLFileHandler::AddStudyContours(const ModelFile::StudyContours& studyContours)
{
	xmlFile_.input_.studyContours = studyContours;
}

ModelFile::StudyContours XMLFileHandler::GetStudyContours() const
{
	ModelFile::StudyContours scs = xmlFile_.GetStudyContours();

	if (scs.studyiuid == xmlFile_.GetStudyInstanceUID())
		return scs;

	return ModelFile::StudyContours();
}

void XMLFileHandler::AddCardiacAnnotation(const CardiacAnnotation& annotation)
{
	if (xmlFile_.GetInput().images.size() == 0)
		xmlFile_.SetCardiacAnnotation(annotation);
	else
	{
		std::vector<ModelFile::Image> images = xmlFile_.GetInput().images;
		std::vector<ImageAnnotation> imageAnnotations = annotation.imageAnnotations;
		std::map<std::string, ImageAnnotation> uidPresentMap;
		std::vector<ImageAnnotation>::const_iterator cit = imageAnnotations.begin();
		for (; cit != imageAnnotations.end(); ++cit)
			uidPresentMap[cit->sopiuid] = *cit;


		CardiacAnnotation minimalAnnotation;
		minimalAnnotation.studyiuid = annotation.studyiuid;

		std::vector<ModelFile::Image>::const_iterator citImages = images.begin();
		for (; citImages != images.end(); ++citImages)
		{
			std::map<std::string, ImageAnnotation>::const_iterator citAnno =
				uidPresentMap.find(citImages->sopiuid);
			if (citAnno != uidPresentMap.end())
				minimalAnnotation.imageAnnotations.push_back(citAnno->second);
		}

		xmlFile_.SetCardiacAnnotation(minimalAnnotation);
	}
}

CardiacAnnotation XMLFileHandler::GetCardiacAnnotation() const
{
	return xmlFile_.GetCardiacAnnotation();
}

void XMLFileHandler::AddProvenanceDetail(std::string const& comment)
{
	ModelFile::ProvenanceDetail provenanceDetail;

	time_t rawtime;
	struct tm * timeinfo;
	char buffer [80];

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strftime (buffer,80,"%Y-%m-%dT%H:%M:%S",timeinfo);

	provenanceDetail.date = buffer;
	std::cout << provenanceDetail.date << '\n';

	provenanceDetail.platform = PlatformInfo::GetPlatform();
	provenanceDetail.operatingSystem = PlatformInfo::GetOSVersion();
	provenanceDetail.program = "CAP Client";
	provenanceDetail.package = "CAP";
	provenanceDetail.process = "Generated by CAP Client";
	provenanceDetail.programParams = "none";
	provenanceDetail.programVersion = CAPCLIENT_VERSION_STRING;
	provenanceDetail.step = boost::lexical_cast<std::string>(xmlFile_.documentation_.provenanceDetails.size() + 1);
	provenanceDetail.comment = comment;

	xmlFile_.documentation_.provenanceDetails.push_back(provenanceDetail);
}

std::string XMLFileHandler::GetProvenanceDetail() const
{
	std::string comment = "";
	if (xmlFile_.documentation_.provenanceDetails.size() > 0)
		comment = xmlFile_.documentation_.provenanceDetails[0].comment;

	return comment;
}

} // namespace cap
