

#include "capclient.h"
#include "utils/debug.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

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
}

int CAPClient::GetNumberOfHeartModelFrames() const
{
	return GetMinimumNumberOfFrames();
}

void CAPClient::LoadLabelledImages(const LabelledSlices& labelledSlices)
{
	gui_->ClearTextureSlices();
	labelledSlices_ = labelledSlices;
	unsigned int shortAxisCount = 0;
	// read (or reread) in dicoms to create image textures (again) for this context.
	LabelledSlices::const_iterator it;
	std::vector<std::string> sliceNames;
	std::vector<bool> visibilities;
	for (it = labelledSlices.begin(); it != labelledSlices.end(); it++)
	{
		// I want the gui to deal with this labelled slice.  The gui needs to create a scene create field images
		// create a texture for displaying field images and position the surface in the scene to the correct place.
		gui_->CreateTextureSlice(*it);
		std::string currentLabel = it->GetLabel();
		sliceNames.push_back(currentLabel);
		dbg("label : '" + currentLabel + "'");
		if (currentLabel.compare(0, 2, "SA") == 0)
		{
			shortAxisCount++;
		}
		if (currentLabel == "LA1")
			visibilities.push_back(true);
		else
			visibilities.push_back(false);
		//-- TODO: contours
	}
	double halfShortAxisCount = shortAxisCount / 2.0;
	int visibleShortAxis = static_cast<int>(halfShortAxisCount + 0.5) - 1; // visibility index is zero based
	visibilities[visibleShortAxis] = true;
	gui_->PopulateSliceList(sliceNames, visibilities);
	
	EnterImagesLoadedState();
}

void CAPClient::LoadCardiacAnnotations(const CardiacAnnotation& anno)
{
	assert(!anno.imageAnnotations.empty());
	cardiacAnnotationPtr_.reset(new CardiacAnnotation(anno));

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

void CAPClient::LoadImagesFromImageBrowserWindow(const SlicesWithImages& slices, const CardiacAnnotation& anno)
{
	//	// Reset the state of the CAPClientWindow
	//	EnterInitState(); // this re-registers the input call back -REVISE
	
	//-- Gone LoadImages(slices);
	//InitializeHeartModelTemplate(slices);
	
	// Create DataPoints if corresponding annotations exist in the CardiacAnnotation
	assert(!anno.imageAnnotations.empty());
	
	cardiacAnnotationPtr_.reset(new CardiacAnnotation(anno));
	
	// The following code should only execute when reading a pre-defined annotation file
	Cmiss_context_id cmiss_context = 0;//--gui_->GetCmissContext();
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmiss_context);
	bool apexDefined = false;
	bool baseDefined = false;
	BOOST_FOREACH(ImageAnnotation const& imageAnno, anno.imageAnnotations)
	{
		BOOST_FOREACH(ROI const& roi, imageAnno.rOIs)
		{
			BOOST_FOREACH(Label const& label, roi.labels)
			{
				if (label.label == "Apex of Heart")
				{
					if (apexDefined)
					{
						continue;
					}
					// create DataPoint		
					std::string const& sopiuid = imageAnno.sopiuid;
					BOOST_FOREACH(SliceInfo const& slice, slices)
					{
						// Find the slice that the image belongs to
						if (slice.ContainsDICOMImage(sopiuid))
						{
							std::string const& regionName = slice.GetLabel();
							std::cout << __func__ << " : regionName = " << regionName << '\n';
							// Find the region for the slice
							Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
							
							DICOMPtr const& dicom = slice.GetDICOMImages().at(0);
							double delY = dicom->GetPixelSizeX();
							double delX = dicom->GetPixelSizeY();
							std::pair<Vector3D,Vector3D> const& ori = dicom->GetImageOrientation();
							Vector3D const& ori1 = ori.first;
							Vector3D const& ori2 = ori.second;
							Point3D pos = dicom->GetImagePosition();
							
							//construct transformation matrix
							gtMatrix m;
							m[0][0] = ori1.x * delX; m[0][1] = ori2.x * delY; m[0][2] = 0; m[0][3] = pos.x;
							m[1][0] = ori1.y * delX; m[1][1] = ori2.y * delY; m[1][2] = 0; m[1][3] = pos.y;
							m[2][0] = ori1.z * delX; m[2][1] = ori2.z * delY; m[2][2] = 1; m[2][3] = pos.z;
							m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
							
							// Convert 2D coords to 3D
							Point3D beforeTrans(roi.points.at(0).x, roi.points.at(0).y, 0);
							Point3D coordPoint3D = m * beforeTrans;
							double coords[3];
							coords[0] = coordPoint3D.x;
							coords[1] = coordPoint3D.y;
							coords[2] = coordPoint3D.z;
							
							double time = 0.0;
							
							if (!region)
							{
								std::cout << __func__ << " : Can't find subregion at path : " << regionName << '\n';
								throw std::invalid_argument(std::string(__func__) + " : Can't find subregion at path : " + regionName);
							}
							Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
							Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, "coordinates_rect");
							Cmiss_node_id cmissNode = 0;//--Cmiss_create_data_point_at_coord(region, field, (double*) coords, time);
							
							assert(modeller_);
							modeller_->AddDataPoint(cmissNode, coordPoint3D, time);
							ProcessDataPointsEnteredForCurrentMode();
							Cmiss_region_destroy(&region);
							apexDefined = true;
						}
					}
				}
				else if (label.label == "Base of Heart")
				{
					if (baseDefined)
					{
						continue;
					}
					// create DataPoint		
					std::string const& sopiuid = imageAnno.sopiuid;
					BOOST_FOREACH(SliceInfo const& slice, slices)
					{
						// Find the slice that the image belongs to
						if (slice.ContainsDICOMImage(sopiuid))
						{
							std::string const& regionName = slice.GetLabel();
							std::cout << __func__ << " : regionName = " << regionName << '\n';
							// Find the region for the slice
							Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root_region, regionName.c_str());
							
							DICOMPtr const& dicom = slice.GetDICOMImages().at(0);
							double delY = dicom->GetPixelSizeX();
							double delX = dicom->GetPixelSizeY();
							std::pair<Vector3D,Vector3D> const& ori = dicom->GetImageOrientation();
							Vector3D const& ori1 = ori.first;
							Vector3D const& ori2 = ori.second;
							Point3D pos = dicom->GetImagePosition();
							
							//construct transformation matrix
							gtMatrix m;
							m[0][0] = ori1.x * delX; m[0][1] = ori2.x * delY; m[0][2] = 0; m[0][3] = pos.x;
							m[1][0] = ori1.y * delX; m[1][1] = ori2.y * delY; m[1][2] = 0; m[1][3] = pos.y;
							m[2][0] = ori1.z * delX; m[2][1] = ori2.z * delY; m[2][2] = 1; m[2][3] = pos.z;
							m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
							
							// Convert 2D coords to 3D
							Point3D beforeTrans(roi.points.at(0).x, roi.points.at(0).y, 0);
							Point3D coordPoint3D = m * beforeTrans;
							double coords[3];
							coords[0] = coordPoint3D.x;
							coords[1] = coordPoint3D.y;
							coords[2] = coordPoint3D.z;
							
							double time = 0.0;
							
							if (!region)
							{
								std::cout << __func__ << " : Can't find subregion at path : " << regionName << '\n';
							throw std::invalid_argument(std::string(__func__) + " : Can't find subregion at path : " + regionName);
							}
							Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
							Cmiss_field_id field = Cmiss_field_module_find_field_by_name(field_module, "coordinates_rect");
							Cmiss_node_id cmissNode = 0;//--Cmiss_create_data_point_at_coord(region, field, (double*) coords, time);
							
							assert(modeller_);
							modeller_->AddDataPoint(cmissNode, coordPoint3D, time);
							ProcessDataPointsEnteredForCurrentMode();
							Cmiss_region_destroy(&region);
							baseDefined = true;
						}
					}
				}
			}
		}
	}
	Cmiss_region_destroy(&root_region);
	
	EnterImagesLoadedState();
}

void CAPClient::OpenModel(const std::string& filename)
{
	cardiacAnnotationPtr_.reset(0);

	CAPXMLFile xmlFile(filename);
	dbg("Start reading xml file");
	xmlFile.ReadFile();
	
	CAPXMLFileHandler xmlFileHandler(xmlFile);
	LabelledSlices labelledSlices = xmlFileHandler.GetLabelledSlices();
	//--const SlicesWithImages& slicesWithImages = xmlFileHandler.GetSlicesWithImages(cmguiManager_);
	//--SlicesWithImages slicesWithImages;
	if (labelledSlices.empty())
	{
		dbg("Can't locate image files");
		return;
	}
	
	// TODO: Hide CAPClient icon in scene viewer
	// TODO: clean up existing
	LoadLabelledImages(labelledSlices);
	//TODO: Load cardiac annotations
	// LoadCardiacAnnotations(cardiacAnnotations);
	
	std::vector<DataPoint> dataPoints = xmlFileHandler.GetDataPoints();
	std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
	dbg("number of exnodeFilenames = " + toString(exnodeFileNames.size()));
	if (exnodeFileNames.empty())
	{
		if(!dataPoints.empty())
		{
			// This means no output element is defined
			InitializeHeartModelTemplate();
			CreateModeller();
			
			dbg("Mode = " + toString(modeller_->GetCurrentMode()) + ", num dataPoints = " + toString(dataPoints.size()));
			modeller_->SetDataPoints(dataPoints);
			// FIXME memory is prematurely released when ok button is pressed from the following window
			// Suppress this feature for now
			//			ImageBrowserWindow *frame = new ImageBrowserWindow(slicesWithImages, cmguiManager_, *this);
			//			frame->Show(true);
			
			//HACK : uncommenting the following will enable models to be constructed from model files with
			// only the input element defined.
			
			Modeller::ModellingModeEnum mode = modeller_->GetCurrentMode();
			gui_->UpdateModeSelectionUI(mode);
			dbg( "Mode = " + toString(mode));
			if (mode == Modeller::GUIDEPOINT)
			{
				EnterModelLoadedState();
			}
		}
		
		return;
	}
	
	const std::string& exelemFileName = xmlFile.GetExelemFileName();
	
	std::string xmlFilename = filename;
	size_t positionOfLastSlash = xmlFilename.find_last_of("/\\");
	std::string modelFilePath = xmlFilename.substr(0, positionOfLastSlash);
	dbg("modelFilePath = " + modelFilePath);
	
	int numberOfModelFrames = exnodeFileNames.size();
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
	gui_->SetHeartModelFocalLength(xmlFile.GetFocalLength());
	gui_->LoadHeartModel(fullExelemFileName, fullExnodeFileNames);
	gui_->SetHeartModelTransformation(m);

	gui_->SetTitle(wxString(title.c_str(),wxConvUTF8));

	//--modeller_->SetDataPoints(dataPoints);

	UpdateMII();
	
	EnterModelLoadedState();
	gui_->UpdateModeSelectionUI(Modeller::GUIDEPOINT);
}

void CAPClient::OpenAnnotation(const std::string& filename, const std::string& imageDirname)
{
	// work with the file
	dbg(std::string(__func__) + " - File name: " + filename);
	dbg(std::string(__func__) + " - Dir name: " + imageDirname);
	
	CAPAnnotationFile annotationFile(filename);
	dbg("Start reading xml file");
	annotationFile.ReadFile();
	
	// Create DICOMTable (filename -> DICOMImage map)
	// Create TextureTable (filename -> Cmiss_texture* map)
	
	// check if a valid file 
	if (annotationFile.GetCardiacAnnotation().imageAnnotations.empty())
	{
		dbg("Invalid Annotation File");
		return;
	}
	
	//		EnterInitState();
	//		cardiacAnnotationPtr_.reset(new CardiacAnnotation(annotationFile.GetCardiacAnnotation()));
	
	ImageBrowser *ib = ImageBrowser::CreateImageBrowser(imageDirname, this);
	ib->SetAnnotation(annotationFile.GetCardiacAnnotation());
	
	// Set annotations to the images in the ImageBrowserWindow.
//--	ib->SetAnnotation(annotationFile.GetCardiacAnnotation());
}

void CAPClient::OpenImages()
{
	ImageBrowser::CreateImageBrowser(previousImageLocation_, this);
	std::cout << "CAPClient::OpenImages show window" << std::endl;
}

void CAPClient::SaveModel(const std::string& dirname, const std::string& userComment)
{
	// Need to write the model files first 
	// FIXME : this is brittle code. shoule be less dependent on the order of execution
	if (mainWindowState_ == MODEL_LOADED_STATE)
	{
		std::cout << __func__ << " - Model name: " << dirname.c_str() << '\n';
		//--assert(heartModelPtr_);
		//--heartModelPtr_->WriteToFile(dirname.c_str());
	}
	
	CAPXMLFile xmlFile(dirname);
	
	std::vector<DataPoint> const& dataPoints = modeller_->GetDataPoints();
	CAPXMLFileHandler xmlFileHandler(xmlFile);
	//--xmlFileHandler.ConstructCAPXMLFile(labelledSlices_, dataPoints, *heartModelPtr_);
	xmlFileHandler.AddProvenanceDetail(userComment);
	
	std::string modelName = FileSystem::GetFileNameWOE(dirname);
	//std::string dirnameStl(dirname.c_str());
	//size_t positionOfLastSlash = dirnameStl.find_last_of("/\\");
	//std::string modelName = dirnameStl.substr(positionOfLastSlash + 1);
	xmlFile.SetName(modelName);
	std::string xmlFilename = dirname + '/' + modelName + ".xml";
	dbg("xmlFilename = " + xmlFilename);
	
	if (cardiacAnnotationPtr_)
	{
		xmlFile.SetCardiacAnnotation(*cardiacAnnotationPtr_);
	}
	
	xmlFile.WriteFile(xmlFilename);
}

void CAPClient::EnterImagesLoadedState()
{
	gui_->EnterImagesLoadedState();
	mainWindowState_ = IMAGES_LOADED_STATE;
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
		Vector3D posDiff = position - previousPosition_;
		double projDistance = DotProduct(posDiff, plane->normal);
		Vector3D delta = projDistance*plane->normal;
		ImagePlane newLocation;
		newLocation = *plane;
		newLocation.blc = newLocation.blc + delta;
		newLocation.brc = newLocation.brc + delta;
		newLocation.tlc = newLocation.tlc + delta;
		newLocation.trc = newLocation.trc + delta;
		gui_->RepositionImagePlane(regionName, &newLocation);
		BOOST_FOREACH(DICOMPtr dicom, it->GetDICOMImages())
		{
			ImagePlane *dicomPlane = dicom->GetImagePlane();
			*dicomPlane = newLocation;
		}
		SetPreviousPosition(position);
		double d = DotProduct(newLocation.tlc, newLocation.normal);
		gui_->UpdateMII(regionName, newLocation.normal, d);
	}

}

} // namespace cap

