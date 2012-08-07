

#include "capclient.h"
#include "utils/debug.h"
#include "logmsg.h"
#include "zinc/extensions.h"

#include <iostream>
#include <fstream>

namespace cap
{

CAPClient* CAPClient::instance_ = 0;

const ImagePlane& CAPClient::GetImagePlane(const std::string& label)
{
	LabelledSlices::const_iterator cit = labelledSlices_.begin();
	for (;cit != labelledSlices_.end(); cit++)
	{
		if (cit->GetLabel() == label)
			return *(cit->GetDICOMImages().at(0)->GetImagePlane());
	}

	throw std::exception();
}

void CAPClient::AttachModellingPoint(int node_id)
{
	LabelledSlices::const_iterator cit = labelledSlices_.begin();
	for (;cit != labelledSlices_.end(); cit++)
	{
		const ImagePlane *plane = cit->GetDICOMImages().at(0)->GetImagePlane();
		modeller_->AttachToIfOn(node_id, cit->GetLabel(), plane->tlc, plane->normal);
	}
}


void CAPClient::SetHeartModelTransformation(const gtMatrix& m)
{
	gui_->SetHeartModelTransformation(m);
}

void CAPClient::SetHeartModelFocalLength(double focalLength)
{
	gui_->SetHeartModelFocalLength(focalLength);
}

void CAPClient::SetHeartModelMuFromBasePlaneAtTime(const Plane& plane, double time)
{
	gui_->SetHeartModelMuFromBasePlaneAtTime(plane, time);
}

void CAPClient::SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time)
{
	gui_->SetHeartModelLambdaParamsAtTime(lambdaParams, time);
}

int CAPClient::ComputeHeartModelXi(const Point3D& position, double time, Point3D& xi) const
{
	return gui_->ComputeHeartModelXi(position, time, xi);
}

Point3D CAPClient::ConvertToHeartModelProlateSpheriodalCoordinate(int node_id, const std::string& region_name) const
{
	return gui_->ConvertToHeartModelProlateSpheriodalCoordinate(node_id, region_name);
}

int CAPClient::GetNumberOfHeartModelFrames() const
{
	return GetMinimumNumberOfFrames();
}

void CAPClient::LoadLabelledImages(const LabelledSlices& labelledSlices)
{
	gui_->CreateProgressDialog("Please wait", "Loading DICOM images", labelledSlices.size());
	gui_->RemoveTextureSlices();
	labelledSlices_ = labelledSlices;
	unsigned int shortAxisCount = 0;
	// read (or reread) in dicoms to create image textures (again) for this context.
	LabelledSlices::const_iterator it = labelledSlices.begin();
	std::vector<std::string> sliceNames;
	std::vector<bool> visibilities;
	for (int count = 0; it != labelledSlices.end(); ++it, count++)
	{
		// I want the gui to deal with this labelled slice.  The gui needs to create a scene create field images
		// create a texture for displaying field images and position the surface in the scene to the correct place.
		gui_->CreateTextureSlice(*it);
		std::string currentLabel = it->GetLabel();
		sliceNames.push_back(currentLabel);
		// Count the number of "SA" labels
		if (currentLabel.compare(0, 2, "SA") == 0)
		{
			shortAxisCount++;
		}
		if (currentLabel == "LA1")
			visibilities.push_back(true);
		else
			visibilities.push_back(false);
		if (!(count % 5))
		{
			gui_->UpdateProgressDialog(count);
		}
	}
	// Set the middle short axis slice visible.
	if (shortAxisCount > 0)
	{
		double halfShortAxisCount = shortAxisCount / 2.0;
		int visibleShortAxis = static_cast<int>(halfShortAxisCount + 0.5) - 1; // visibility index is zero based
		visibilities[visibleShortAxis] = true;
	}
	gui_->PopulateSliceList(sliceNames, visibilities);

	gui_->DestroyProgressDialog();
}

void CAPClient::LoadCardiacAnnotations(const CardiacAnnotation& anno)
{
	assert(!anno.imageAnnotations.empty());
	cardiacAnnotation_ = anno;

	// Set some special nodes?
	BOOST_FOREACH(ImageAnnotation const& imageAnno, anno.imageAnnotations)
	{
		//std::cout << "anno: " << imageAnno.sopiuid << std::endl;
		BOOST_FOREACH(ROI const& roi, imageAnno.rOIs)
		{
			BOOST_FOREACH(Label const& label, roi.labels)
			{
				dbg(label.label);
			}
		}
	}
}

void CAPClient::LoadContours(const std::vector<ModelFile::ImageContours>& imageContours)
{
	gui_->RemoveImageContours();
	imageContours_ = imageContours;
	std::vector<ModelFile::ImageContours>::const_iterator c_it = imageContours.begin();
	for (; c_it != imageContours.end(); ++c_it)
	{
		ModelFile::ImageContours ic = *c_it;
		std::string label = "";
		int index = -1;
		LabelledSlices::const_iterator labelledSlicesIterator = labelledSlices_.begin();
		while (label.empty() && labelledSlicesIterator != labelledSlices_.end())
		{
			index = labelledSlicesIterator->IndexOf(ic.sopiuid);
			if (index >= 0)
			{
				label = labelledSlicesIterator->GetLabel();
			}
			else
				++labelledSlicesIterator;
		}
		if (!label.empty())
		{
			gui_->AddImageContours(label, ic.contours, index);
		}
		else
			LOG_MSG(LOGWARNING) << "Did not find labelled slice with sopiuid = " << ic.sopiuid;
	}
}

namespace
{

/**
 * Recursively searches the given path to generate the sopiuid to DICOMPtr map.
 *
 * @param path	The path to search for dicom files from.
 * @return A HashTable of sopiuid to DICOMPtr.
 */
HashTable GenerateSopiuidToDICOMPtrMap(std::string const& path)
{
	HashTable hashTable;
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

		std::string fullpath = path + "/" + filename;
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
			LOG_MSG(LOGERROR) << "Invalid DICOM file : '" << fullpath << "'";
		}
	}
	Cmiss_field_module_destroy(&field_module);
	Cmiss_region_destroy(&region);
	Cmiss_context_destroy(&context);

	return hashTable;
}

}

HashTable CAPClient::MapSopiuidToDICOMPtr(const std::vector<std::string>& sopiuids)
{
	gui_->CreateProgressDialog("Please wait", "Searching for DICOM images", sopiuids.size());

	HashTable uidToDICOMPtrMap = GenerateSopiuidToDICOMPtrMap(previousImageLocation_);


	std::vector<std::string>::const_iterator cit = sopiuids.begin();
	for (int count = 0; cit != sopiuids.end(); ++cit, count++)
	{
		std::string sopiuid = *cit;
		HashTable::const_iterator map_iterator = uidToDICOMPtrMap.find(sopiuid);
		while (map_iterator == uidToDICOMPtrMap.end())
		{
			//Can't locate the file. Ask the user to locate the dicom file.
			std::string dirname = gui_->ChooseDirectory(previousImageLocation_);
			if ( dirname.empty() )
			{
				gui_->DestroyProgressDialog();
				uidToDICOMPtrMap.clear();
				return uidToDICOMPtrMap;
			}
			else
			{
				previousImageLocation_ = dirname;
				HashTable newMap = GenerateSopiuidToDICOMPtrMap(dirname);
				uidToDICOMPtrMap.insert(newMap.begin(), newMap.end());
				map_iterator = uidToDICOMPtrMap.find(sopiuid);
			}
		}
		if (count % 10 == 0)
			gui_->UpdateProgressDialog(count);
	}
	gui_->UpdateProgressDialog(sopiuids.size());

	return uidToDICOMPtrMap;
}

void CAPClient::OpenModel(const std::string& filename)
{
	// Clear out anything old.
	ModelFile xmlFile;
	xmlFile.ReadFile(filename);

	if (previousImageLocation_.length() == 0)
		previousImageLocation_ = GetPath(filename);

	XMLFileHandler xmlFileHandler(xmlFile);

	HashTable map = MapSopiuidToDICOMPtr(xmlFileHandler.GetSopiuids());
	if (map.empty())
	{
		LOG_MSG(LOGERROR) << "Can't locate image files - failed to load model '" << filename << "'";
		return;
	}
	LabelledSlices labelledSlices = xmlFileHandler.GetLabelledSlices(map);


	cardiacAnnotation_ = xmlFileHandler.GetCardiacAnnotation();

	LoadLabelledImages(labelledSlices);
	ResetModel();

	comment_ = xmlFileHandler.GetProvenanceDetail();
	ModelFile::StudyContours studyContours = xmlFileHandler.GetStudyContours();
	LoadContours(studyContours.listOfImageContours);

	ModellingPointDetails modellingPointDetails = xmlFileHandler.GetModellingPointDetails();
	std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
	// If exnodeFileNames is empty then this is a version 2.0.0 model file.
	if (exnodeFileNames.empty())
	{
		if(!modellingPointDetails.empty())
		{
			// This means no output element is defined
			// Setting the modelling points should put the CAPClient into the correct state
			gui_->ProcessModellingPointDetails(modellingPointDetails);
		}
		return;
	}

	const std::string& exelemFileName = xmlFile.GetExelemFileName();

	std::string xmlFilename = filename;
	size_t positionOfLastSlash = xmlFilename.find_last_of("/\\");
	std::string modelFilePath = xmlFilename.substr(0, positionOfLastSlash);

	assert(exnodeFileNames.size() == GetMinimumNumberOfFrames());
	gtMatrix m;
	xmlFile.GetTransformationMatrix(m);

	std::string title = labelledSlices.at(0).GetDICOMImages().at(0)->GetPatientID() + " - " + xmlFile.GetFilename();
	std::vector<std::string>::const_iterator cit = exnodeFileNames.begin();
	std::vector<std::string> fullExnodeFileNames;
	for(; cit != exnodeFileNames.end(); cit++)
	{
		fullExnodeFileNames.push_back(modelFilePath + "/" + *cit);
	}
	std::string fullExelemFileName = modelFilePath + "/" + exelemFileName;

	gui_->CreateHeartModel();
	gui_->LoadHeartModel(fullExelemFileName, fullExnodeFileNames);
	gui_->SetHeartModelFocalLength(xmlFile.GetFocalLength());
	gui_->SetHeartModelTransformation(m);
	gui_->ProcessModellingPointDetails(modellingPointDetails);
	modeller_->InitialiseBezierLambdaParams();

	gui_->SetTitle(wxString(title.c_str(),wxConvUTF8));

	UpdateMII();

	// Set the gui into modelling mode because version 1.0.0 project files don't list
	// modelling points, they are implicit in the heart model transform.
	ChangeModellingMode(GUIDEPOINT);
}

void CAPClient::OpenImageBrowser()
{
	ImageBrowser::CreateImageBrowser(previousImageLocation_, cardiacAnnotation_, this);
}

void CAPClient::SaveModel(const std::string& dirname, const std::string& userComment)
{
	ModelFile xmlFile;

	XMLFileHandler xmlFileHandler(xmlFile);

	xmlFileHandler.AddLabelledSlices(labelledSlices_);
	std::vector<ModellingPoint> modellingPoints = modeller_->GetModellingPoints();
	xmlFileHandler.AddModellingPoints(modellingPoints);
	xmlFileHandler.AddProvenanceDetail(userComment);

	std::string dirnameStr(dirname.c_str());
	size_t positionOfLastSlash = dirnameStr.find_last_of("/\\");
	std::string modelName = dirnameStr.substr(positionOfLastSlash + 1);
	xmlFile.SetName(modelName);
	std::string xmlFilename = dirname + '/' + modelName + ".xml";
	dbg("xmlFilename = " + xmlFilename);

	if (cardiacAnnotation_.IsValid())
	{
		xmlFile.SetCardiacAnnotation(cardiacAnnotation_);
	}

	xmlFile.WriteFile(xmlFilename);
}

void CAPClient::OnExportToCmgui(const std::string& dirname)
{
	unsigned int numberOfModelFrames = GetMinimumNumberOfFrames();

	if (numberOfModelFrames > 0)
	{
		gui_->WriteHeartModel(dirname, numberOfModelFrames);
	}
}

void CAPClient::OnExportHeartVolumes(const std::string& filename)
{
	unsigned int numberOfModelFrames = GetMinimumNumberOfFrames();

	std::ofstream volume_file(filename.c_str());
	volume_file << "time, epi volume (ml), endo volume (ml)" << std::endl;
	for (unsigned int i = 0; i < numberOfModelFrames; i++)
	{
		double time = static_cast<double>(i)/numberOfModelFrames;
		double epi = gui_->ComputeHeartVolume(EPICARDIUM, time);
		double endo = gui_->ComputeHeartVolume(ENDOCARDIUM, time);
		volume_file << (i+1) << "," << epi << "," << endo << std::endl;
	}
	volume_file.close();
}

void CAPClient::InitializeMII()
{
	LabelledSlices::const_iterator itr = labelledSlices_.begin();
	for (;itr != labelledSlices_.end();++itr)
	{
		const std::string& sliceName = itr->GetLabel();
		gui_->InitializeMII(sliceName);
	}
}

void CAPClient::UpdateMII()
{
	LabelledSlices::const_iterator cit = labelledSlices_.begin();
	for (; cit != labelledSlices_.end(); cit++)
	{
		const std::string& sliceName = cit->GetLabel();

		const ImagePlane& plane = *(cit->GetDICOMImages().at(0)->GetImagePlane());
		double d = DotProduct(plane.tlc, plane.normal);
		gui_->UpdateMII(sliceName, plane.normal, d);
	}
}

void CAPClient::InitializeHeartModelTemplate()
{
	unsigned int numberOfModelFrames = GetMinimumNumberOfFrames();

	if (numberOfModelFrames > 0)
	{
		gui_->CreateHeartModel();
		gui_->LoadTemplateHeartModel(numberOfModelFrames);
	}
}

unsigned int CAPClient::GetMinimumNumberOfFrames() const
{
	// Check to make sure we have at least some labelled slices
	if (labelledSlices_.size() == 0)
		return 0;

	LabelledSlices::const_iterator cit = labelledSlices_.begin();
	unsigned int minFrames = cit->GetDICOMImages().size();
	cit++;
	for (;cit != labelledSlices_.end(); cit++)
	{
		if (cit->GetDICOMImages().size() < minFrames)
			minFrames = cit->GetDICOMImages().size();
	}

	return minFrames;
}

void CAPClient::UpdatePlanePosition(const std::string& regionName, const Point3D& position)
{
	LabelledSlices::iterator it = labelledSlices_.begin();
	for (; it != labelledSlices_.end() && it->GetLabel() != regionName; it++)
		;

	if (it != labelledSlices_.end())
	{
		ImagePlane *plane = it->GetDICOMImages().at(0)->GetImagePlane();
		Point3D currentLocation = it->GetDICOMImages().at(0)->GetImagePosition();
		Vector3D posDiff = position - previousPosition_;
		Vector3D proj;
		if (imageShiftingNormalMode_)
		{
			// Project the movement onto the normal of the plane
			proj = DotProduct(posDiff, plane->normal)*plane->normal;
		}
		else
		{
			// Project the movement onto the plane
			proj = posDiff - DotProduct(posDiff, plane->normal)*plane->normal;
		}

		ImagePlane newPlane;
		newPlane = *plane;
		newPlane.blc = newPlane.blc - proj;
		newPlane.brc = newPlane.brc - proj;
		newPlane.tlc = newPlane.tlc - proj;
		newPlane.trc = newPlane.trc - proj;

		gui_->RepositionImagePlane(regionName, &newPlane);
		Point3D newLocation = currentLocation - proj;
		BOOST_FOREACH(DICOMPtr dicom, it->GetDICOMImages())
		{
			dicom->SetImagePosition(newLocation);
		}
		if (modeller_)
			modeller_->ImagePlaneMoved(it->GetLabel(), proj);

		SetPreviousPosition(position);
		double d = DotProduct(newLocation, plane->normal);
		gui_->UpdateMII(regionName, plane->normal, d);
	}
}

} // namespace cap

