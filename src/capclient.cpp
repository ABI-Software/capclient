

#include "capclient.h"
#include "utils/debug.h"
#include "logmsg.h"

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

void CAPClient::OpenModel(const std::string& filename)
{
	// Clear out anything old.

	ModelFile xmlFile;
	xmlFile.ReadFile(filename);
	
	XMLFileHandler xmlFileHandler(xmlFile);
	gui_->CreateProgressDialog("Please wait", "Searching for DICOM images", 10);
	LabelledSlices labelledSlices = xmlFileHandler.GetLabelledSlices();
	gui_->UpdateProgressDialog(10);

	if (labelledSlices.empty())
	{
		LOG_MSG(LOGERROR) << "Can't locate image files - failed to load model '" << filename << "'";
		return;
	}

	// Extract the location of the image files from the first dicom image
	std::vector<DICOMPtr> dicomImages = labelledSlices[0].GetDICOMImages();
	if (dicomImages.size() > 0)
	{
		previousImageLocation_ = GetPath(dicomImages[0]->GetFilename());
	}

	cardiacAnnotation_ = xmlFileHandler.GetCardiacAnnotation();

	// TODO: Hide CAPClient icon in scene viewer
	LOG_MSG(LOGINFORMATION) << "Opening model '" << filename << "'";
	LoadLabelledImages(labelledSlices);
	ResetModel();

    ModellingPointDetails modellingPointDetails = xmlFileHandler.GetModellingPointDetails();
	std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
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
	
    unsigned int numberOfModelFrames = exnodeFileNames.size();
	assert(numberOfModelFrames == GetMinimumNumberOfFrames());
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
	
	//EnterModelLoadedState();
	// Set the gui into modelling mode because version 1.0.0 project files don't list 
	//modelling points, they are implicit in the heart model transform.
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
	
	LOG_MSG(LOGINFORMATION) << "Saving model '" << xmlFilename << "'";
	if (cardiacAnnotation_.IsValid())
	{
		xmlFile.SetCardiacAnnotation(cardiacAnnotation_);
	}
	
	xmlFile.WriteFile(xmlFilename);
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
//            Point3D currentPos = dicom->GetImagePosition();
            dicom->SetImagePosition(newLocation);
//			ImagePlane *dicomPlane = dicom->GetImagePlane();
//			*dicomPlane = newLocation;
		}
		SetPreviousPosition(position);
        double d = DotProduct(newLocation, plane->normal);
        gui_->UpdateMII(regionName, plane->normal, d);
	}

}

} // namespace cap

