/*
 * CAPXMLFileHandler.cpp
 *
 *  Created on: Aug 13, 2010
 *      Author: jchu014
 */

#include "CAPXMLFileHandler.h"
#include "DICOMImage.h"
#include "CAPModelLVPS4X4.h"
#include "DataPoint.h"
#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "FileSystem.h"
#include "PlatformInfo.h"

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

namespace cap
{

class EqualToSliceInfoByName 
{
	// this is needed as boost::bind and boost::tuple dont mix well
	// see http://lists.boost.org/boost-users/2007/01/24527.php
public:
	EqualToSliceInfoByName(std::string const& name)
	: sliceName_(name)
	{}
	
	bool operator() (const SliceInfo& sliceInfo) const
	{
		return (sliceName_ == sliceInfo.get<0>());
	}
	
private:
	std::string const& sliceName_;
};

CAPXMLFileHandler::CAPXMLFileHandler(CAPXMLFile& xmlFile)
:
	xmlFile_(xmlFile)
{}

void CAPXMLFileHandler::ContructCAPXMLFile(SlicesWithImages const& slicesWithImages,
									std::vector<DataPoint> const& dataPoints,
									CAPModelLVPS4X4 const& heartModel)
{
	if (slicesWithImages.empty())
	{
		std::cout << __func__ << ": No dicom files to construct CAPXMLFile from\n";
		return;
	}
	
	std::string studyIUid = slicesWithImages[0].get<1>()[0]->GetStudyInstanceUID();
	xmlFile_.SetStudyInstanceUID(studyIUid);
	std::string const& filename = xmlFile_.GetFilename();
	size_t positionOfLastSlash = filename.find_last_of("/\\");
//	std::cout << "positionOfLastSlash = " << positionOfLastSlash << std::endl;
	std::string name = filename.substr(positionOfLastSlash+1);
	xmlFile_.SetName(name);
	xmlFile_.SetChamber("LV");
	
	// CAPXMLInput
	int slice = 0;
	
	xmlFile_.ClearInputAndOutput();
	BOOST_FOREACH(SliceInfo const& sliceInfo, slicesWithImages)
	{
		std::string const& label = sliceInfo.get<0>();
		std::vector<DICOMPtr> const& dicomFiles = sliceInfo.get<1>();
		
		int frame = 0;
		BOOST_FOREACH(DICOMPtr const& dicomFile, dicomFiles)
		{
			CAPXMLFile::Image image;
			image.sopiuid = dicomFile->GetSopInstanceUID();
			image.seriesiuid = dicomFile->GetSeriesInstanceUID();
			image.label = label;
			image.frame = frame++;
			image.slice = slice;
			// if the images have been shifted for mis-registraion correction,
			// put the new position and orientation in each image element
			// TODO : This is really a per-slice attribute rather than per image.
			//        Need to change the xml file schema accordingly ??
			if (dicomFile->IsShifted())
			{
				Point3D const& pos = dicomFile->GetShiftedImagePosition();
				image.imagePosition = boost::make_shared<Point3D>(pos);
			}
			if (dicomFile->IsRotated())
			{
				typedef std::pair<Vector3D, Vector3D> Orientation;
				Orientation ori = dicomFile->GetShiftedImageOrientation();
				image.imageOrientation = boost::make_shared<Orientation>(ori);
			}

			//image.countourFiles;;
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
		// time is normailized between 0 and 1, so we can find the frame number from it.
		
		EqualToSliceInfoByName pred(sliceName);
		SlicesWithImages::const_iterator itr = std::find_if(slicesWithImages.begin(), slicesWithImages.end(), pred);
		assert(itr != slicesWithImages.end());
		
		std::vector<DICOMPtr> const& dicomFilesWithMatchingSliceName = itr->get<1>();
		size_t numFrames = dicomFilesWithMatchingSliceName.size();
		
		// CHECK for correctless!!
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

namespace
{

boost::unordered_map<std::string, DICOMPtr> GenerateSopiuidToFilenameMap(std::string const& path)
{
	boost::unordered_map<std::string, DICOMPtr> hashTable;
	FileSystem fileSystem(path);
	std::vector<std::string> const& filenames = fileSystem.getAllFileNames();
	BOOST_FOREACH(std::string const& filename, filenames)
	{
		std::string fullpath = path + filename;
		try
		{
			DICOMPtr image  = boost::make_shared<DICOMImage>(fullpath);
			hashTable.insert(std::make_pair(image->GetSopInstanceUID(), image));
		}
		catch (std::exception& e)
		{
			std::cout << __func__ << ": Invalid DICOM file - " << filename << '\n';
		}
	}

	return hashTable;
}

} // unnamed namespace

SlicesWithImages CAPXMLFileHandler::GetSlicesWithImages(CmguiManager const& cmguiManager) const
{
	SlicesWithImages dicomSlices;

	std::string const& filename = xmlFile_.GetFilename();
	size_t positionOfLastSlash = filename.find_last_of("/\\");
	std::string pathToDICOMFiles = filename.substr(0, positionOfLastSlash+1);

	typedef boost::unordered_map<std::string, DICOMPtr> HashTable;
	HashTable uidToFilenameMap = GenerateSopiuidToFilenameMap(pathToDICOMFiles);
//	std::cout << "GenerateSopiuidToFilenameMap\n";

	// Populate SlicesWithImages
	typedef std::map<std::string, std::vector<DICOMPtr> > DICOMImageMapWithSliceNameAsKey;
	DICOMImageMapWithSliceNameAsKey dicomMap;
	CAPXMLFile::Input& input = xmlFile_.GetInput();
	BOOST_FOREACH(CAPXMLFile::Image const& image, input.images)
	{
		HashTable::const_iterator dicomFileItr = uidToFilenameMap.find(image.sopiuid);
		while (dicomFileItr == uidToFilenameMap.end())
		{
			//Can't locate the file
			std::cout << "No matching filename in the sopiuid to filename map\n";

			wxString currentWorkingDir = wxGetCwd();
			wxString defaultPath = currentWorkingDir.Append("/Data");

			const wxString& dirname = wxDirSelector("Choose the folder that contains the images", defaultPath);
			if ( !dirname.empty() )
			{
				std::cout << __func__ << " - Dir name: " << dirname.c_str() << '\n';
				HashTable newMap = GenerateSopiuidToFilenameMap((dirname + "/").c_str());
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
			dicomImage->SetShiftedImagePosition(pos);
		}

		//TODO handle cases where image label is not present
		DICOMImageMapWithSliceNameAsKey::iterator itr = dicomMap.find(image.label);
		if (itr == dicomMap.end())
		{
			std::vector<DICOMPtr> v(1, dicomImage);
			dicomMap.insert(std::make_pair(image.label, v));
		}
		else
		{
			itr->second.push_back(dicomImage);
		}
	}

	BOOST_FOREACH(DICOMImageMapWithSliceNameAsKey::value_type& labelAndImages, dicomMap)
	{
		std::string const& label = labelAndImages.first;
		std::vector<DICOMPtr>& images = labelAndImages.second;

		std::vector<Cmiss_texture_id> textures;
		BOOST_FOREACH(DICOMPtr const& dicomImage, images)
		{
			Cmiss_texture_id texture_id = cmguiManager.LoadCmissTexture(dicomImage->GetFilename());
			textures.push_back(texture_id);
		}

		SliceInfo sliceInfo = boost::make_tuple(label, images, textures);
		dicomSlices.push_back(sliceInfo);
	}

	std::sort(dicomSlices.begin(), dicomSlices.end(), SliceInfoSortOrder()); // make Short axes appear first

	return dicomSlices;
}

std::vector<DataPoint> CAPXMLFileHandler::GetDataPoints(CmguiManager const& cmguiManager) const
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
			itr->second ++;
		}
	}

	Cmiss_context_id cmiss_context = cmguiManager.GetCmissContext();
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmiss_context);
	assert(root_region);
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
				std::cout << __func__ << " : Can't find subregion at path : " << image.label << '\n';
				continue;
			}
			Cmiss_field_id field = Cmiss_region_find_field_by_name(region, "coordinates_rect");
			Cmiss_node_id cmissNode = Cmiss_create_data_point_at_coord(region,
							field, (double*) coords, time);

			Point3D coordPoint3D(coords);
			dataPoints.push_back(DataPoint(cmissNode, coordPoint3D, p.type, time));
		}
		Cmiss_region_destroy(&region);
	}
	Cmiss_region_destroy(&root_region);

	return dataPoints;
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
	provenanceDetail.programVersion = "0.4"; //FIXME this is hard to maintain
	provenanceDetail.step = boost::lexical_cast<std::string>(xmlFile_.documentation_.provenanceDetails.size() + 1);
	provenanceDetail.comment = comment;

	xmlFile_.documentation_.provenanceDetails.push_back(provenanceDetail);
}

} // namespace cap
