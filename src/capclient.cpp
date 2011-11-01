

#include "capclient.h"
#include "utils/debug.h"

namespace cap
{

CAPClient* CAPClient::instance_ = 0;

int CAPClient::GetFrameNumberForTime(double time)
{
	assert(heartModelPtr_);
	int frameNumber = heartModelPtr_->MapToModelFrameNumber(time);

	return frameNumber;
}

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

void CAPClient::LoadImages(const SlicesWithImages& slices) // name misleading?
{
	assert(!slices.empty());
	
	if(imageSet_)
	{
		imageSet_->SetVisible(false); // HACK should really destroy region
		delete imageSet_;
	}
	
	ImageSetBuilder builder(slices);
	imageSet_ = builder.build();
	imageSet_->SetVisible(true);//FIXME
	
	this->PopulateSliceList(); // fill in slice check box list
}

void CAPClient::PopulateSliceList()
{	
	const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
	std::vector<bool> visibilities;
	BOOST_FOREACH(std::string const& sliceName, sliceNames)
	{
		if (imageSet_->IsVisible(sliceName))
		{
			visibilities.push_back(true);
		}
		else
		{
			visibilities.push_back(false);
		}
	}
	gui_->PopulateSliceList(sliceNames, visibilities);
}

void CAPClient::LoadLabelledImages(const LabelledSlices& labelledSlices)
{
	gui_->ClearTextureSlices();
	labelledSlices_ = labelledSlices;
	// read (or reread) in dicoms to create image textures (again) for this context.
	LabelledSlices::const_iterator it;
	std::vector<std::string> sliceNames;
	std::vector<bool> visibilities;
	for (it = labelledSlices.begin(); it != labelledSlices.end(); it++)
	{
		// I want the gui to deal with this labelled slice.  The gui needs to create a scene create field images
		// create a texture for displaying field images and position the surface in the scene to the correct place.
		gui_->CreateTextureSlice(*it);
		sliceNames.push_back((*it).GetLabel());
		visibilities.push_back(true);
		//-- TODO: contours
	}
	
	gui_->PopulateSliceList(sliceNames, visibilities);
	
	InitializeModelTemplate(labelledSlices);
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
	
	LoadImages(slices);
	//InitializeModelTemplate(slices);
	
	// Create DataPoints if corresponding annotations exist in the CardiacAnnotation
	assert(!anno.imageAnnotations.empty());
	
	cardiacAnnotationPtr_.reset(new CardiacAnnotation(anno));
	
	// The following code should only execute when reading a pre-defined annotation file
	Cmiss_context_id cmiss_context = gui_->GetCmissContext();
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
							Cmiss_node_id cmissNode = Cmiss_create_data_point_at_coord(region, field, (double*) coords, time);
							
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
							Cmiss_node_id cmissNode = Cmiss_create_data_point_at_coord(region,
																					field, (double*) coords, time);
							
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
	
	LoadLabelledImages(labelledSlices);
	//TODO: Load cardiac annotations
	// LoadCardiacAnnotations(cardiacAnnotations);

	// TODO clean up first
	//--LoadImagesFromXMLFile(slicesWithImages);
	
	
	std::vector<DataPoint> dataPoints = xmlFileHandler.GetDataPoints();
	std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
	dbg("number of exnodeFilenames = " + toString(exnodeFileNames.size()));
	if (exnodeFileNames.empty())
	{
		// This means no output element is defined
		//--InitializeModelTemplate(slicesWithImages);
		EnterImagesLoadedState();
		
		dbg("Mode = " + toString(modeller_->GetCurrentMode()) + ", num dataPoints = " + toString(dataPoints.size()));
		modeller_->SetDataPoints(dataPoints);
		// FIXME memory is prematurely released when ok button is pressed from the following window
		// Suppress this feature for now
		//			ImageBrowserWindow *frame = new ImageBrowserWindow(slicesWithImages, cmguiManager_, *this);
		//			frame->Show(true);
		
		//HACK : uncommenting the following will enable models to be constructed from model files with
		// only the input element defined.
		
		CAPModeller::ModellingMode mode = modeller_->GetCurrentMode();
		gui_->UpdateModeSelectionUI(mode);
		dbg( "Mode = " + toString(mode));
		if (mode == CAPModeller::GUIDEPOINT)
		{
			EnterModelLoadedState();
		}
		
		return;
	}
	
	const std::string& exelemFileName = xmlFile.GetExelemFileName();
	
	//HACK FIXME
	std::string xmlFilename = filename;
	size_t positionOfLastSlash = xmlFilename.find_last_of("/\\");
	std::string modelFilePath = xmlFilename.substr(0, positionOfLastSlash);
	dbg("modelFilePath = " + modelFilePath);
	
	heartModelPtr_.reset(new HeartModel("heart"));
	assert(heartModelPtr_);
	heartModelPtr_->SetFocalLengh(xmlFile.GetFocalLength());
	int numberOfModelFrames = exnodeFileNames.size();
	heartModelPtr_->SetNumberOfModelFrames(numberOfModelFrames);

	std::vector<std::string>::const_iterator cit = exnodeFileNames.begin();
	std::vector<std::string> fullExnodeFileNames;
	for(; cit != exnodeFileNames.end(); cit++)
	{
		fullExnodeFileNames.push_back(modelFilePath + "/" + *cit);
	}
	//LoadHeartModel(modelFilePath, exnodeFileNames);
	gui_->LoadHeartModel(fullExnodeFileNames);
	UpdateStatesAfterLoadingModel();
	EnterModelLoadedState();
	//labelledSlices.at(0).GetDICOMImages().at(0)->GetPatientID();
	std::string title = labelledSlices.at(0).GetDICOMImages().at(0)->GetPatientID() + " - " + xmlFile.GetFilename();
	gui_->SetTitle(wxString(title.c_str(),wxConvUTF8));
	gtMatrix m;
	xmlFile.GetTransformationMatrix(m);
	heartModelPtr_->SetLocalToGlobalTransformation(m);
	modeller_->SetDataPoints(dataPoints);
	UpdateMII();
	
	gui_->UpdateModeSelectionUI(CAPModeller::GUIDEPOINT);
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
	
	if (ib_)
		delete ib_;
	ib_ = ImageBrowser::CreateImageBrowser(imageDirname, this);
	
	// Set annotations to the images in the ImageBrowserWindow.
	ib_->SetAnnotation(annotationFile.GetCardiacAnnotation());
}

void CAPClient::OpenImages(const std::string& imageDirname)
{
	if (ib_)
		delete ib_;
	ib_ = ImageBrowser::CreateImageBrowser(imageDirname, this);
	std::cout << "CAPClient::OpenImages show window" << std::endl;
	//ib_->ShowWindow();
}

void CAPClient::SaveModel(const std::string& dirname, const std::string& userComment)
{
	// Need to write the model files first 
	// FIXME : this is brittle code. shoule be less dependent on the order of execution
	if (mainWindowState_ == MODEL_LOADED_STATE)
	{
		std::cout << __func__ << " - Model name: " << dirname.c_str() << '\n';
		assert(heartModelPtr_);
		heartModelPtr_->WriteToFile(dirname.c_str());
	}
	
	CAPXMLFile xmlFile(dirname);
	
	//SlicesWithImages const& slicesAndImages = imageSet_->GetSlicesWithImages();
	std::vector<DataPoint> const& dataPoints = modeller_->GetDataPoints();
	CAPXMLFileHandler xmlFileHandler(xmlFile);
	xmlFileHandler.ConstructCAPXMLFile(labelledSlices_, dataPoints, *heartModelPtr_);
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
	// This method only makes sense when both the images and the model have been already loaded.
	//--const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
	LabelledSlices::const_iterator itr = labelledSlices_.begin();
	for (;itr != labelledSlices_.end();++itr)
	{
		const std::string& sliceName = itr->GetLabel();
		//char str[256];
		
		// Initialize the MII-related field and iso_scalar to some dummy values
		// This is done to set the graphical attributes that are needed for the MII rendering
		
		std::string command = "gfx define field /heart/slice_" + sliceName + " coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[1 1 1]\";";
		//sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[1 1 1]\";",
		//		sliceName.c_str() );
		Cmiss_context_execute_command(gui_->GetCmissContext(), command.c_str());
		
		command = "gfx modify g_element heart iso_surfaces exterior iso_scalar slice_" + sliceName + " iso_values 100 use_faces select_on material gold selected_material default_selected render_shaded line_width 2;";
		//sprintf((char*)str, "gfx modify g_element heart iso_surfaces exterior iso_scalar slice_%s iso_values 100 use_faces select_on material gold selected_material default_selected render_shaded line_width 2;"
		//,sliceName.c_str());
		Cmiss_context_execute_command(gui_->GetCmissContext(), command.c_str());
	}
}

void CAPClient::UpdateMII()
{
	LabelledSlices::const_iterator cit = labelledSlices_.begin();
	for (int index = 0;cit != labelledSlices_.end();cit++, index++)
	{
		const std::string& sliceName = cit->GetLabel();
		char str[256];
		
		
		const ImagePlane& plane = *(cit->GetDICOMImages().at(0)->GetImagePlane());
		assert(heartModelPtr_);
		const gtMatrix& m = heartModelPtr_->GetLocalToGlobalTransformation();
		
		gtMatrix mInv;
		inverseMatrix(m, mInv);
		transposeMatrix(mInv); // gtMatrix is column Major and our matrix functions assume row major FIX!!
		
		//Need to transform the image plane using the Local to global transformation matrix of the heart (ie to hearts local coord)
		Vector3D normalTransformed = m * plane.normal;
		sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[%f %f %f]\";",
				sliceName.c_str() ,
				normalTransformed.x, normalTransformed.y, normalTransformed.z);
		Cmiss_context_execute_command(gui_->GetCmissContext(), str);
		
		Point3D pointTLCTransformed = mInv * plane.tlc;
		double d = DotProduct((pointTLCTransformed - Point3D(0,0,0)), normalTransformed);
		heartModelPtr_->UpdateMII(index, d);
	}
}

void CAPClient::InitializeModelTemplate(const LabelledSlices& slices)
{
	unsigned int minNumberOfFrames = GetMinimumNumberOfFrames();
	
	heartModelPtr_.reset(new HeartModel("heart"));
	assert(heartModelPtr_);
	heartModelPtr_->SetNumberOfModelFrames(minNumberOfFrames);
	//LoadTemplateHeartModel("heart", std::string(CAP_DATA_DIR) + "templates/" ); //HACK FIXME
	gui_->LoadTemplateHeartModel(minNumberOfFrames);
	UpdateStatesAfterLoadingModel();
	//		XRCCTRL(*this, "MII", wxCheckBox)->SetValue(false); FIXME
	//		XRCCTRL(*this, "Wireframe", wxCheckBox)->SetValue(false);
	gui_->EnterInitState(); // HACK to clear mii and wireframe check boxes
	heartModelPtr_->SetMIIVisibility(false);
	heartModelPtr_->SetModelVisibility(false);
}

unsigned int CAPClient::GetMinimumNumberOfFrames() const
{
	LabelledSlices::const_iterator cit = labelledSlices_.begin();
	unsigned int minFrames = cit->GetDICOMImages().size();
	dbg("frame count: " + toString(minFrames));
	for (;cit != labelledSlices_.end(); cit++)
	{
		dbg("frame count: " + toString(cit->GetDICOMImages().size()));
		if (cit->GetDICOMImages().size() < minFrames)
			minFrames = cit->GetDICOMImages().size();
	}

	return minFrames;
}

} // namespace cap

