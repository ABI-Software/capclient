

#include "capclient.h"

#include "cmguicallbacks.h"

namespace cap
{

CAPClient* CAPClient::instance_ = 0;

/**
 * TODO: remove Cmiss_scene_viewer_set_lookat_parameters_non_skew and put
 * it into gui class
 */
void CAPClient::OnSliceSelected(std::string const& sliceName)
{
	//called from OnObjectCheckListSelected
	
	const ImagePlane& plane = imageSet_->GetImagePlane(sliceName);
	
	// compute the center of the image plane, eye(camera) position and the up vector
	Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
	Point3D eye = planeCenter + (plane.normal * 500); // this seems to determine the near clip plane
	Vector3D up(plane.yside);
	up.Normalise();
	
	//Hack :: perturb direction vector a little
	eye.x *= 1.01; //HACK 1.001 makes the iso lines partially visible
	
	if (!Cmiss_scene_viewer_set_lookat_parameters_non_skew(
		gui_->GetCmissSceneViewer(), eye.x, eye.y, eye.z,
		planeCenter.x, planeCenter.y, planeCenter.z,
		up.x, up.y, up.z))
	{
		//Error;
	}
	gui_->RedrawNow();
}

int CAPClient::GetFrameNumberForTime(double time)
{
	assert(heartModelPtr_);
	int frameNumber = heartModelPtr_->MapToModelFrameNumber(time);
}

void CAPClient::OnAnimationSliderEvent(double time)
{
	if (!heartModelPtr_) //FIXME
	{
		return;
	}
		
	time = (time > 0.99) ? 0.0 : time;
	
	//Time_keeper_request_new_time(timeKeeper_, time);
		
	int frameNumber = heartModelPtr_->MapToModelFrameNumber(time);
	gui_->SetTime(time);
	gui_->UpdateFrameNumber(frameNumber);
	Refresh3DCanvas(); // forces redraw while silder is being manipulated
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

void CAPClient::LoadLabelledImagesFromImageBrowser(const std::vector<LabelledSlice>& labelledSlices, const std::vector<LabelledTexture>& labelledTextures, const CardiacAnnotation& anno)
{
	// Reset capXMLFilePtr_
	if (capXMLFilePtr_)
	{
		capXMLFilePtr_.reset(0);
	}
	std::vector<LabelledSlice>::const_iterator it;
	std::vector<std::string> sliceNames;
	std::vector<bool> visibilities;
	for (it = labelledSlices.begin(); it != labelledSlices.end(); it++)
	{
		gui_->CreateScene((*it).GetLabel());
		std::vector<Cmiss_field_image_id> fieldImages = gui_->CreateFieldImages(*it);
		gui_->ChangeTexture((*it).GetLabel(), fieldImages.at(0));
		sliceNames.push_back((*it).GetLabel());
		visibilities.push_back(true);
		//-- TODO: contours
	}
	gui_->PopulateSliceList(sliceNames, visibilities);
	BOOST_FOREACH(ImageAnnotation const& imageAnno, anno.imageAnnotations)
	{
		std::cout << "anno: " << imageAnno.sopiuid << std::endl;
		BOOST_FOREACH(ROI const& roi, imageAnno.rOIs)
		{
			BOOST_FOREACH(Label const& label, roi.labels)
			{
				std::cout << label.label << std::endl;
			}
		}
	}
	EnterImagesLoadedState();
}

void CAPClient::LoadImagesFromImageBrowserWindow(const SlicesWithImages& slices, const CardiacAnnotation& anno)
{
	// Reset capXMLFilePtr_
	if (capXMLFilePtr_)
	{
		capXMLFilePtr_.reset(0);
	}
	//	// Reset the state of the CAPClientWindow
	//	EnterInitState(); // this re-registers the input call back -REVISE
	
	LoadImages(slices);
	InitializeModelTemplate(slices);
	
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
							Point3D pos;
							if (dicom->IsShifted())
							{
								pos = dicom->GetShiftedImagePosition();
							}
							else
							{
								pos = dicom->GetImagePosition();
							}
							
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
							Point3D pos;
							if (dicom->IsShifted())
							{
								pos = dicom->GetShiftedImagePosition();
							}
							else
							{
								pos = dicom->GetImagePosition();
							}
							
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

void CAPClient::OpenModel(std::string const& filename)
{
	cardiacAnnotationPtr_.reset(0);
	//		CAPXMLFile xmlFile(filename.c_str());
	capXMLFilePtr_.reset(new CAPXMLFile(filename.c_str()));
	CAPXMLFile& xmlFile(*capXMLFilePtr_);
	std::cout << "Start reading xml file\n";
	xmlFile.ReadFile();
	
	CAPXMLFileHandler xmlFileHandler(xmlFile);
	//--const SlicesWithImages& slicesWithImages = xmlFileHandler.GetSlicesWithImages(cmguiManager_);
	SlicesWithImages slicesWithImages;
	if (slicesWithImages.empty())
	{
		std::cout << "Can't locate image files\n";
		return;
	}
	
	// TODO clean up first
	LoadImagesFromXMLFile(slicesWithImages);
	
	std::vector<DataPoint> dataPoints; //-- = xmlFileHandler.GetDataPoints(cmguiManager_);
	
	std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
	std::cout << "number of exnodeFilenames = " << exnodeFileNames.size() << '\n';
	if (exnodeFileNames.empty())
	{
		// This means no output element is defined
		InitializeModelTemplate(slicesWithImages);
		EnterImagesLoadedState();
		
		std::cout << "Mode = " << modeller_->GetCurrentMode()<< ", num dataPoints = " << dataPoints.size() << '\n';
		modeller_->SetDataPoints(dataPoints);
		// FIXME memory is prematurely released when ok button is pressed from the following window
		// Suppress this feature for now
		//			ImageBrowserWindow *frame = new ImageBrowserWindow(slicesWithImages, cmguiManager_, *this);
		//			frame->Show(true);
		
		//HACK : uncommenting the following will enable models to be constructed from model files with
		// only the input element defined.
		
		CAPModeller::ModellingMode mode = modeller_->GetCurrentMode();
		gui_->UpdateModeSelectionUI(mode);
		std::cout << "Mode = " << mode << '\n';
		if (mode == CAPModeller::GUIDEPOINT)
		{
			EnterModelLoadedState();
		}
		Refresh3DCanvas();
		
		return;
	}
	
	std::string const& exelemFileName = xmlFile.GetExelemFileName();
	
	//HACK FIXME
	std::string xmlFilename = filename.c_str();
	size_t positionOfLastSlash = xmlFilename.find_last_of("/\\");
	std::string modelFilePath = xmlFilename.substr(0, positionOfLastSlash);
	std::cout << "modelFilePath = " << modelFilePath << '\n';
	
	heartModelPtr_.reset(new CAPModelLVPS4X4("heart", gui_->GetCmissContext()));
	assert(heartModelPtr_);
	heartModelPtr_->SetFocalLengh(xmlFile.GetFocalLength());
	int numberOfModelFrames = exnodeFileNames.size();
	heartModelPtr_->SetNumberOfModelFrames(numberOfModelFrames);
	LoadHeartModel(modelFilePath, exnodeFileNames);
	std::string title = imageSet_->GetPatientID() + " - " + xmlFile.GetFilename();
	gui_->SetTitle(wxString(title.c_str(),wxConvUTF8));
	gtMatrix m;
	xmlFile.GetTransformationMatrix(m);
	heartModelPtr_->SetLocalToGlobalTransformation(m);
	modeller_->SetDataPoints(dataPoints);
	UpdateMII();
	
	gui_->UpdateModeSelectionUI(CAPModeller::GUIDEPOINT);
	Refresh3DCanvas();
}

void CAPClient::OpenAnnotation(std::string const& filename, std::string const& imageDirname)
{
	// work with the file
	std::cout << __func__ << " - File name: " << filename.c_str() << '\n';
	std::cout << __func__ << " - Dir name: " << imageDirname << '\n';
	
	CAPAnnotationFile annotationFile(filename.c_str());
	std::cout << "Start reading xml file\n";
	annotationFile.ReadFile();
	
	// Create DICOMTable (filename -> DICOMImage map)
	// Create TextureTable (filename -> Cmiss_texture* map)
	
	// check if a valid file 
	if (annotationFile.GetCardiacAnnotation().imageAnnotations.empty())
	{
		std::cout << "Invalid Annotation File\n";
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

void CAPClient::SaveModel(std::string const& dirname, std::string const& userComment)
{
	// Need to write the model files first 
	// FIXME : this is brittle code. shoule be less dependent on the order of execution
	if (mainWindowState_ == MODEL_LOADED_STATE)
	{
		std::cout << __func__ << " - Model name: " << dirname.c_str() << '\n';
		assert(heartModelPtr_);
		heartModelPtr_->WriteToFile(dirname.c_str());
	}
	
	if (!capXMLFilePtr_)
	{
		capXMLFilePtr_.reset(new CAPXMLFile(dirname.c_str()));
	}
	CAPXMLFile& xmlFile(*capXMLFilePtr_);
	
	SlicesWithImages const& slicesAndImages = imageSet_->GetSlicesWithImages();
	std::vector<DataPoint> const& dataPoints = modeller_->GetDataPoints();
	CAPXMLFileHandler xmlFileHandler(xmlFile);
	xmlFileHandler.ContructCAPXMLFile(slicesAndImages, dataPoints, *heartModelPtr_);
	xmlFileHandler.AddProvenanceDetail(userComment);
	
	std::string dirnameStl(dirname.c_str());
	size_t positionOfLastSlash = dirnameStl.find_last_of("/\\");
	std::string modelName = dirnameStl.substr(positionOfLastSlash + 1);
	xmlFile.SetName(modelName);
	std::string xmlFilename = std::string(dirname.c_str()) + "/" + modelName + ".xml";
	std::cout << "xmlFilename = " << xmlFilename << '\n';
	
	if (cardiacAnnotationPtr_)
	{
		xmlFile.SetCardiacAnnotation(*cardiacAnnotationPtr_);
	}
	
	xmlFile.WriteFile(xmlFilename);
}

void CAPClient::EnterImagesLoadedState()
{
	gui_->EnterImagesLoadedState();
	
	StopCine();
	
	// Initialize timer for animation
	size_t numberOfLogicalFrames = 8;//--imageSet_->GetNumberOfFrames(); // smallest number of frames of all slices
	if (timeNotifier_)
	{
		Cmiss_time_keeper_remove_time_notifier(timeKeeper_, timeNotifier_);
		Cmiss_time_notifier_destroy(&timeNotifier_);
	}
	timeNotifier_ = Cmiss_time_keeper_create_notifier_regular(timeKeeper_, numberOfLogicalFrames, 0);
	Cmiss_time_notifier_add_callback(timeNotifier_, time_callback, (void*)this);
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME, 0.0);
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME, 1.0);
	//		Time_keeper_set_minimum(timeKeeper_, 0); // FIXME time range is always 0~1
	//		Time_keeper_set_maximum(timeKeeper_, 1);
	
	gui_->SetAnimationSliderRange(0, numberOfLogicalFrames);
	
	//--gui_->SetTitle(wxString(imageSet_->GetPatientID().c_str(),wxConvUTF8));
	
	mainWindowState_ = IMAGES_LOADED_STATE;
}

void CAPClient::InitializeMII()
{
	// This method only makes sense when both the images and the model have been already loaded.
	const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
	std::vector<std::string>::const_iterator itr = sliceNames.begin();
	for (;itr != sliceNames.end();++itr)
	{
		std::string const& sliceName = *itr;
		char str[256];
		
		// Initialize the MII-related field and iso_scalar to some dummy values
		// This is done to set the graphical attributes that are needed for the MII rendering
		
		sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[1 1 1]\";",
				sliceName.c_str() );
		Cmiss_context_execute_command(gui_->GetCmissContext(), str);
		
		sprintf((char*)str, "gfx modify g_element heart iso_surfaces exterior iso_scalar slice_%s iso_values 100 use_faces select_on material gold selected_material default_selected render_shaded line_width 2;"
		,sliceName.c_str());
		Cmiss_context_execute_command(gui_->GetCmissContext(), str);
	}
}

void CAPClient::UpdateMII()
{
	const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
	std::vector<std::string>::const_iterator itr = sliceNames.begin();
	for (int index = 0;itr != sliceNames.end();++itr, ++index)
	{
		const std::string& sliceName = *itr;
		char str[256];
		
		const ImagePlane& plane = imageSet_->GetImagePlane(sliceName);
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

void CAPClient::InitializeModelTemplate(SlicesWithImages const& slices)
{
	SlicesWithImages::const_iterator 
	itrToMinNumberOfFrames = std::min_element(slices.begin(), slices.end(),
											  ComparatorForNumFrames());
	int minNumberOfFrames = itrToMinNumberOfFrames->GetDICOMImages().size();
	
	heartModelPtr_.reset(new CAPModelLVPS4X4("heart", gui_->GetCmissContext()));
	assert(heartModelPtr_);
	heartModelPtr_->SetNumberOfModelFrames(minNumberOfFrames);
	LoadTemplateHeartModel("heart", std::string(CAP_DATA_DIR) + "templates/" ); //HACK FIXME
	//		XRCCTRL(*this, "MII", wxCheckBox)->SetValue(false); FIXME
	//		XRCCTRL(*this, "Wireframe", wxCheckBox)->SetValue(false);
	gui_->EnterInitState(); // HACK to clear mii and wireframe check boxes
	heartModelPtr_->SetMIIVisibility(false);
	heartModelPtr_->SetModelVisibility(false);
}

} // namespace cap

