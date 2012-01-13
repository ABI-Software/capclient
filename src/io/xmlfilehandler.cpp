/*
 * XMLFileHandler.cpp
 *
 *  Created on: Aug 13, 2010
 *      Author: jchu014
 */

#include "io/xmlfilehandler.h"

#include <wx/wx.h>
#include <wx/dir.h> // FIXME move this out to a separate function/class

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <assert.h>
#include <functional>
#include <time.h>
#include <float.h>

extern "C"
{
#include <zn/cmiss_status.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_region.h>
}

#include "capclientconfig.h"
#include "utils/debug.h"
#include "io/modelfile.h"
#include "dicomimage.h"
#include "model/heart.h"
#include "cmgui/sceneviewerpanel.h"
#include "utils/filesystem.h"
#include "PlatformInfo.h"
#include "CAPContour.h"
#include "logmsg.h"
#include "utils/misc.h"
#include "cmgui/extensions.h"

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
		p.surface = UNDEFINED_HEART_SURFACE_TYPE;
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

namespace
{

	boost::unordered_map<std::string, DICOMPtr> GenerateSopiuidToFilenameMap(std::string const& path)
	{
		boost::unordered_map<std::string, DICOMPtr> hashTable;
		std::vector<std::string> const& filenames = GetAllFileNamesRecursive(path);
		Cmiss_context_id context = Cmiss_context_create("temp");
		Cmiss_region_id region = Cmiss_context_get_default_region(context);
		Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
	
		BOOST_FOREACH(std::string const& filename, filenames)
		{
			// Skip files that are known not to be dicom files
			if (EndsWith(filename, ".exnode") ||
				EndsWith(filename, ".exelem") ||
				EndsWith(filename, ".xml"))
			{
				continue;
			}
			
			std::string fullpath = path + filename;
			Cmiss_field_image_id image_field = Cmiss_field_module_create_image_texture(field_module, fullpath);
			if (image_field != 0)
			{
				DICOMPtr image(new DICOMImage(fullpath));
				if (image->Analyze(image_field))
					hashTable.insert(std::make_pair(image->GetSopInstanceUID(), image));
				else
					Cmiss_field_image_destroy(&image_field);
			}
			if (image_field != 0)
				Cmiss_field_image_destroy(&image_field);
			else
			{
				std::cout << __func__ << ": Invalid DICOM file - '" << filename << "'" << std::endl;
				LOG_MSG(LOGERROR) << "Invalid DICOM file : '" << fullpath << "'";
			}
		}
		Cmiss_field_module_destroy(&field_module);
		Cmiss_region_destroy(&region);
		Cmiss_context_destroy(&context);

		return hashTable;
	}

} // unnamed namespace

LabelledSlices XMLFileHandler::GetLabelledSlices() const
{
	LabelledSlices labelledSlices;
	const std::string& filename = xmlFile_.GetFilename();
	// Search for the dicom files in the same dir as the xml dir first.
	// TODO: put last location of dicom image into XMLFile
	size_t positionOfLastSlash = filename.find_last_of("/\\");
	std::string pathToXMLFile = filename.substr(0, positionOfLastSlash+1);

	typedef boost::unordered_map<std::string, DICOMPtr> HashTable;
	HashTable uidToFilenameMap = GenerateSopiuidToFilenameMap(pathToXMLFile);

	typedef std::map<std::string, LabelledSlice > LabelledSliceMap;
	LabelledSliceMap labelledSliceMap;

	ModelFile::Input& input = xmlFile_.GetInput();
	BOOST_FOREACH(ModelFile::Image const& image, input.images)
	{
		HashTable::const_iterator dicomFileItr = uidToFilenameMap.find(image.sopiuid);
		while (dicomFileItr == uidToFilenameMap.end())
		{
			//Can't locate the file. Ask the user to locate the dicom file.
			dbg("No matching filename in the sopiuid to filename map");

			wxString defaultPath = wxGetCwd();

			const wxString& dirname = wxDirSelector(wxT("Choose the folder that contains the images"), defaultPath);
			if ( !dirname.empty() )
			{
				std::cout << __func__ << " - Dir name: " << dirname.c_str() << '\n';
				wxString dirfname = dirname + wxT("/");
				HashTable newMap = GenerateSopiuidToFilenameMap(std::string(dirfname.mb_str()));
				uidToFilenameMap.insert(newMap.begin(), newMap.end());
				dicomFileItr = uidToFilenameMap.find(image.sopiuid);
			}
			else
			{
				// User cancelled the operation. return empty set
				dbg("User cancelled returning empty set.");
				labelledSlices.clear();
				return labelledSlices;
			}
		}
		DICOMPtr dicomImage = dicomFileItr->second;
		if (image.imagePosition)
		{
			Point3D const& pos = *image.imagePosition;
			dicomImage->SetImagePosition(pos);
		}

		// Create Contour objects
		ModelFile::StudyContours const& studyContours = input.studyContours;
		std::vector<ModelFile::ImageContours>::const_iterator imageContours_itr =
				std::find_if(studyContours.listOfImageContours.begin(),
							studyContours.listOfImageContours.end(),
							boost::bind(std::equal_to<std::string>(),
									boost::bind(&ModelFile::ImageContours::sopiuid, _1),
									image.sopiuid));
		if (imageContours_itr != studyContours.listOfImageContours.end())
		{
			ModelFile::ImageContours const& imageContours = *imageContours_itr;
			BOOST_FOREACH(ModelFile::Contour const& contour, imageContours.contours)
			{
				std::vector<Point3D> points;
				points.reserve(contour.contourPoints.size());
				BOOST_FOREACH(ModelFile::ContourPoint const& contourPoint, contour.contourPoints)
				{
					points.push_back(Point3D(contourPoint.x, contourPoint.y, 0));
				}
				ContourPtr capContour = boost::make_shared<CAPContour>(contour.number, contour.transformationMatrix, points);
				dicomImage->AddContour(capContour);
			}
		}

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

ModellingPoints XMLFileHandler::GetModellingPoints() const
{
	ModellingPoints modellingPoints;
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
		ModellingPoint mp(p.type, 0, -1, position, p.time);
		modellingPoints.push_back(mp);
	}

	return modellingPoints;
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

} // namespace cap
