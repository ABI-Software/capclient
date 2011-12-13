/*
 * CAPXMLFileHandler.cpp
 *
 *  Created on: Aug 13, 2010
 *      Author: jchu014
 */

#include <wx/wx.h>
#include <wx/dir.h> // FIXME move this out to a separate function/class

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/unordered_map.hpp>
#include <boost/algorithm/string/predicate.hpp>

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
#include <api/cmiss_field_module.h>
#include <api/cmiss_region.h>
}

#include "capclientconfig.h"
#include "utils/debug.h"
#include "CAPXMLFileHandler.h"
#include "CAPXMLFile.h"
#include "dicomimage.h"
#include "model/heart.h"
#include "datapoint.h"
#include "cmgui/sceneviewerpanel.h"
#include "utils/filesystem.h"
#include "PlatformInfo.h"
#include "CAPContour.h"
#include "logmsg.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

namespace cap
{

CAPXMLFileHandler::CAPXMLFileHandler(CAPXMLFile& xmlFile)
	: xmlFile_(xmlFile)
{}

void CAPXMLFileHandler::ConstructCAPXMLFile(const LabelledSlices& labelledSlices,
									const std::vector<DataPoint>& dataPoints,
									const HeartModel& heartModel)
{
	if (labelledSlices.empty())
	{
		std::cout << __func__ << ": No dicom files to construct CAPXMLFile from\n";
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
	xmlFile_.ClearInputAndOutput();
	BOOST_FOREACH(const LabelledSlice& labelledSlice, labelledSlices)
	{
		std::string const& label = labelledSlice.GetLabel();
		std::vector<DICOMPtr> const& dicomFiles = labelledSlice.GetDICOMImages();
		
		int frame = 0;
		BOOST_FOREACH(DICOMPtr const& dicomFile, dicomFiles)
		{
			CAPXMLFile::Image image;
			image.sopiuid = dicomFile->GetSopInstanceUID();
			image.seriesiuid = dicomFile->GetSeriesInstanceUID();
			image.label = label;
			image.frame = frame++;
			image.slice = slice;
			image.imageOrientation = boost::shared_ptr<Orientation>();
			image.imagePosition = boost::shared_ptr<Point3D>();;
			image.points = std::vector<CAPXMLFile::Point>();
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
////				CAPXMLFile::ContourFile contourFile = {baseName, number};
////				image.contourFiles.push_back(contourFile);
//			}
			
			//image.points; // FIX?
			xmlFile_.AddImage(image);
		}
		slice++;
	}
	
	BOOST_FOREACH(DataPoint const& dataPoint, dataPoints)
	{	
		CAPXMLFile::Point p;
		p.surface = dataPoint.GetSurfaceType();
		p.type = dataPoint.GetDataPointType();
		Point3D const& coord = dataPoint.GetCoordinate();
		CAPXMLFile::Value x = {coord.x, "x"};
		p.values["x"] = x; //REVISE
		CAPXMLFile::Value y = {coord.y, "y"};
		p.values["y"] = y;
		CAPXMLFile::Value z = {coord.z, "z"};
		p.values["z"] = z;
		
		std::string const& sliceName = dataPoint.GetSliceName();
		double time = dataPoint.GetTime();
		// time is normalized between 0 and 1, so we can find the frame number from it.
		
		LabelledSlices::const_iterator itr = std::find_if(labelledSlices.begin(), labelledSlices.end(), 
															boost::bind(std::equal_to<std::string>(),
																		boost::bind(&LabelledSlice::GetLabel, _1),
																		sliceName));
		assert(itr != labelledSlices.end());
		
		std::vector<DICOMPtr> const& dicomFilesWithMatchingSliceName = itr->GetDICOMImages();
		size_t numFrames = dicomFilesWithMatchingSliceName.size();
		
		// CHECK for correctness!!
		double frameDuration = (double) 1.0 / numFrames;
		double frameFloat = time / frameDuration;
		size_t frame = static_cast<size_t>(frameFloat);
		if ((frameFloat - frame) > 0.5)
		{
			frame++;
		}
		
		size_t frameNumber = std::min(frame, numFrames);
		std::string sopiuid = dicomFilesWithMatchingSliceName.at(frameNumber)->GetSopInstanceUID();
		
		CAPXMLFile::Input& input = xmlFile_.GetInput();
		std::vector<CAPXMLFile::Image>::iterator image_itr = std::find_if(input.images.begin(), input.images.end(),
				boost::bind(std::equal_to<std::string>() , boost::bind(&CAPXMLFile::Image::sopiuid, _1), sopiuid));
		assert(image_itr != input.images.end());
		image_itr->points.push_back(p); // FIXME replace with AddPointToImage
	}
	
	// Output
	CAPXMLFile::Output& output = xmlFile_.GetOutput();
	output.elemFileName = heartModel.GetExelemFileName();
	output.focalLength = heartModel.GetFocalLength();
	output.interval = 1.0/heartModel.GetNumberOfModelFrames();// 1.0 = 1 cardiac cycle (normalised) - FIX
	gtMatrix const& gtTrans = heartModel.GetLocalToGlobalTransformation();
	std::stringstream transformMatrixStream;
	transformMatrixStream <<
			gtTrans[0][0] << " " << gtTrans[0][1] << " " << gtTrans[0][2] << " " << gtTrans[0][3] << " " <<
			gtTrans[1][0] << " " << gtTrans[1][1] << " " << gtTrans[1][2] << " " << gtTrans[1][3] << " " <<
 			gtTrans[2][0] << " " << gtTrans[2][1] << " " << gtTrans[2][2] << " " << gtTrans[2][3] << " " <<
			gtTrans[3][0] << " " << gtTrans[3][1] << " " << gtTrans[3][2] << " " << gtTrans[3][3];
	output.transformationMatrix = transformMatrixStream.str();

	std::vector<std::string> const& modelFiles = heartModel.GetExnodeFileNames();
	// assume the model files are sorted by the frame number
	for (size_t i = 0; i < modelFiles.size(); i++)
	{
		CAPXMLFile::Exnode exnode;
		exnode.exnode = modelFiles[i];
		exnode.frame = i;
//		output.exnodes.push_back(frame);
		xmlFile_.AddExnode(exnode);
	}
}
void CAPXMLFileHandler::Clear()
{
	xmlFile_.ClearInputAndOutput();
}

void CAPXMLFileHandler::AddLabelledSlices(const LabelledSlices& labelledSlices)
{
	if (labelledSlices.empty())
	{
		dbg("CAPXMLFileHandler::AddLabelledSlices : No dicom files to construct CAPXMLFile from");
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
			CAPXMLFile::Image image;
			image.sopiuid = dicomFile->GetSopInstanceUID();
			image.seriesiuid = dicomFile->GetSeriesInstanceUID();
			image.label = label;
			image.frame = frame++;
			image.slice = slice;
			image.imageOrientation = boost::shared_ptr<Orientation>();
			image.imagePosition = boost::shared_ptr<Point3D>();;
			image.points = std::vector<CAPXMLFile::Point>();
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
//				CAPXMLFile::ContourFile contourFile = {baseName, number};
//				image.contourFiles.push_back(contourFile);
//			}
			
			//image.points; // FIX?
			xmlFile_.AddImage(image);
		}
		slice++;
	}
	
}

void CAPXMLFileHandler::AddModellingPoints(const std::vector<ModellingPoint>& modellingPoints)
{
	std::vector<ModellingPoint>::const_iterator it = modellingPoints.begin();
	for(;it != modellingPoints.end(); ++it)
	{
		CAPXMLFile::Point p;
		p.surface = UNDEFINED_HEART_SURFACE_TYPE;
		p.type = it->GetModellingPointType();
		const Point3D& position = it->GetPosition();
		CAPXMLFile::Value x = {position.x, "x"};
		p.values["x"] = x; //REVISE
		CAPXMLFile::Value y = {position.y, "y"};
		p.values["y"] = y;
		CAPXMLFile::Value z = {position.z, "z"};
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
	std::vector<std::string> const& filenames = FileSystem::GetAllFileNamesRecursive(path);
	BOOST_FOREACH(std::string const& filename, filenames)
	{
		// Skip files that are known not to be dicom files
		if (boost::iends_with(filename, ".exnode") ||
			boost::iends_with(filename, ".exelem") ||
			boost::iends_with(filename, ".xml"))
		{
			continue;
		}
		
		std::string fullpath = path + filename;
		try
		{
			DICOMPtr image(new DICOMImage(fullpath));
			image->ReadFile();
			hashTable.insert(std::make_pair(image->GetSopInstanceUID(), image));
		}
		catch (std::exception& e)
		{
			std::cout << __func__ << ": Invalid DICOM file - " << filename << ", " << e.what() << std::endl;
			LOG_MSG(LOGERROR) << "Invalid DICOM file : '" << fullpath << "' (" << e.what() << ")";
		}
	}

	return hashTable;
}

} // unnamed namespace

LabelledSlices CAPXMLFileHandler::GetLabelledSlices() const
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

	CAPXMLFile::Input& input = xmlFile_.GetInput();
	BOOST_FOREACH(CAPXMLFile::Image const& image, input.images)
	{
		HashTable::const_iterator dicomFileItr = uidToFilenameMap.find(image.sopiuid);
		while (dicomFileItr == uidToFilenameMap.end())
		{
			//Can't locate the file. Ask the user to locate the dicom file.
			dbg("No matching filename in the sopiuid to filename map");

			wxString currentWorkingDir = wxGetCwd();
			wxString defaultPath = currentWorkingDir.Append(wxT("/Data"));

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
		dbg(image.label);
		DICOMPtr dicomImage = dicomFileItr->second;
		if (image.imagePosition)
		{
			Point3D const& pos = *image.imagePosition;
			dicomImage->SetImagePosition(pos);
		}

		// Create Contour objects
		CAPXMLFile::StudyContours const& studyContours = input.studyContours;
		std::vector<CAPXMLFile::ImageContours>::const_iterator imageContours_itr =
				std::find_if(studyContours.listOfImageContours.begin(),
							studyContours.listOfImageContours.end(),
							boost::bind(std::equal_to<std::string>(),
									boost::bind(&CAPXMLFile::ImageContours::sopiuid, _1),
									image.sopiuid));
		if (imageContours_itr != studyContours.listOfImageContours.end())
		{
			CAPXMLFile::ImageContours const& imageContours = *imageContours_itr;
			BOOST_FOREACH(CAPXMLFile::Contour const& contour, imageContours.contours)
			{
				std::vector<Point3D> points;
				points.reserve(contour.contourPoints.size());
				BOOST_FOREACH(CAPXMLFile::ContourPoint const& contourPoint, contour.contourPoints)
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

SlicesWithImages CAPXMLFileHandler::GetSlicesWithImages(SceneViewerPanel *cmguiManager) const
{
	SlicesWithImages dicomSlices;

	const std::string& filename = xmlFile_.GetFilename();
	size_t positionOfLastSlash = filename.find_last_of("/\\");
	// Search for the dicom files in the same dir as the xml dir first.
	std::string pathToXMLFile = filename.substr(0, positionOfLastSlash+1);

	typedef boost::unordered_map<std::string, DICOMPtr> HashTable;
	HashTable uidToFilenameMap = GenerateSopiuidToFilenameMap(pathToXMLFile);
//	std::cout << "GenerateSopiuidToFilenameMap\n";

	// Populate SlicesWithImages
	typedef std::map<std::string, std::vector<DICOMPtr> > DICOMImageMapWithSliceNameAsKey;
	DICOMImageMapWithSliceNameAsKey dicomMap;
	
	std::map<int, size_t > numberOfFrameForSlice;
	
	CAPXMLFile::Input& input = xmlFile_.GetInput();
	BOOST_FOREACH(CAPXMLFile::Image const& image, input.images)
	{
		HashTable::const_iterator dicomFileItr = uidToFilenameMap.find(image.sopiuid);
		while (dicomFileItr == uidToFilenameMap.end())
		{
			//Can't locate the file. Ask the user to locate the dicom file.
			dbg("No matching filename in the sopiuid to filename map");

			wxString currentWorkingDir = wxGetCwd();
			wxString defaultPath = currentWorkingDir.Append(wxT("/Data"));

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
				dicomSlices.clear();
				return dicomSlices;
			}
		}

		DICOMPtr dicomImage = dicomFileItr->second;
		// Image Position
		if (image.imagePosition)
		{
			Point3D const& pos = *image.imagePosition;
			dicomImage->SetImagePosition(pos);
		}
		
		//TODO handle cases where image label is not present
		DICOMImageMapWithSliceNameAsKey::iterator itr = dicomMap.find(image.label);
		if (itr == dicomMap.end())
		{
			std::vector<DICOMPtr> v(1, dicomImage);
			dicomMap.insert(std::make_pair(image.label, v));
			
			// compute number of frames for each slice
			size_t numberOfFrames = std::count_if(input.images.begin(), input.images.end(),
					boost::bind(&CAPXMLFile::Image::slice, _1) == image.slice);
			numberOfFrameForSlice.insert(std::make_pair(image.slice, numberOfFrames));
		}
		else
		{
			itr->second.push_back(dicomImage);
		}		
		
		// Create Contour objects
		CAPXMLFile::StudyContours const& studyContours = input.studyContours;
		std::vector<CAPXMLFile::ImageContours>::const_iterator imageContours_itr =
				std::find_if(studyContours.listOfImageContours.begin(),
							studyContours.listOfImageContours.end(),
							boost::bind(std::equal_to<std::string>(),
									boost::bind(&CAPXMLFile::ImageContours::sopiuid, _1),
									image.sopiuid));
		if (imageContours_itr != studyContours.listOfImageContours.end())
		{
			CAPXMLFile::ImageContours const& imageContours = *imageContours_itr;
			BOOST_FOREACH(CAPXMLFile::Contour const& contour, imageContours.contours)
			{
				std::vector<Point3D> points;
				points.reserve(contour.contourPoints.size());
				BOOST_FOREACH(CAPXMLFile::ContourPoint const& contourPoint, contour.contourPoints)
				{
					points.push_back(Point3D(contourPoint.x, contourPoint.y, 0));
				}
				ContourPtr capContour = boost::make_shared<CAPContour>(contour.number, contour.transformationMatrix, points);
				dicomImage->AddContour(capContour);
			}
		}
	}

	BOOST_FOREACH(DICOMImageMapWithSliceNameAsKey::value_type& labelAndImages, dicomMap)
	{
		std::string const& label = labelAndImages.first;
		std::vector<DICOMPtr>& images = labelAndImages.second;

		std::vector<Cmiss_field_image_id> textures;
		//Cmiss_field_image_id image_volume = 0;
		//cmguiManager->Cmiss_field_module_create_image_texture(image_volume, images);
		SliceInfo sliceInfo(label, images, textures /** TODO: requires fixing */);
		dicomSlices.push_back(sliceInfo);
	}

	std::sort(dicomSlices.begin(), dicomSlices.end(), SliceInfoSortOrder()); // make Short axes appear first

	return dicomSlices;
}

std::vector<DataPoint> CAPXMLFileHandler::GetDataPoints() const
{
	std::map<std::string, size_t> labelToNumframesMap;
	CAPXMLFile::Input& input = xmlFile_.GetInput();
	BOOST_FOREACH(CAPXMLFile::Image const& image, input.images)
	{
		std::map<std::string, size_t>::iterator itr = labelToNumframesMap.find(image.label);
		if (itr == labelToNumframesMap.end())
		{
			labelToNumframesMap.insert(std::make_pair(image.label, 1));
		}
		else
		{
			itr->second++;
		}
	}

	Cmiss_context_id cmiss_context = 0; //-- TODO: fix this cmguiManager->GetCmissContext();
	Cmiss_region_id root_region = 0; //-- TODO: fix this Cmiss_context_get_default_region(cmiss_context);
	//--assert(root_region);
	std::vector<DataPoint> dataPoints;
	BOOST_FOREACH(CAPXMLFile::Image const& image, input.images)
	{
		double numFrames = static_cast<double>(labelToNumframesMap[image.label]);

		Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, image.label.c_str());
		BOOST_FOREACH(CAPXMLFile::Point const& p, image.points)
		{
			double coords[3];
			coords[0] = (*p.values.find("x")).second.value;
			coords[1] = (*p.values.find("y")).second.value;
			coords[2] = (*p.values.find("z")).second.value;

			double time = static_cast<double>(image.frame) / numFrames;

			if (!region)
			{
				dbg(std::string(__func__) + " : Can't find subregion at path : " + image.label);
				continue;
			}
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
			Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, "coordinates_rect");
			dbg("**** Requires implementation");
			//--Cmiss_node_id cmissNode = Cmiss_create_data_point_at_coord(region, field, (double*) coords, time);

			Point3D coordPoint3D(static_cast<Real>(coords[0]), static_cast<Real>(coords[1]), static_cast<Real>(coords[2]));
			//--dataPoints.push_back(DataPoint(cmissNode, coordPoint3D, p.type, time));
			dataPoints.push_back(DataPoint(0, coordPoint3D, p.type, time));
		}
		Cmiss_region_destroy(&region);
	}
	Cmiss_region_destroy(&root_region);

	return dataPoints;
}

ModellingPoints CAPXMLFileHandler::GetModellingPoints() const
{
	ModellingPoints modellingPoints;
	CAPXMLFile::Input& input = xmlFile_.GetInput();
	std::vector<CAPXMLFile::Point>::const_iterator cit = input.points.begin();
	for (; cit != input.points.end(); ++cit)
	{
		const CAPXMLFile::Point& p = *cit;
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

void CAPXMLFileHandler::AddProvenanceDetail(std::string const& comment)
{
	CAPXMLFile::ProvenanceDetail provenanceDetail;
	
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
