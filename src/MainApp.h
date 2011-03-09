/*
 * MainApp.h
 *
 *  Created on: Mar 9, 2011
 *      Author: jchu014
 */

#ifndef MAINAPP_H_
#define MAINAPP_H_

#include "ImageBrowseWindowClient.h"
#include "CAPMath.h"

#include "Config.h"
#include "SliceInfo.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
#include "ImageSetBuilder.h"
#include "ImageSet.h"
#include "CmguiExtensions.h"
#include "ImageBrowser.h"
#include "ImageBrowseWindow.h"
#include "CAPXMLFile.h"
#include "CAPAnnotationFile.h"
#include "CAPModeller.h"
#include "CAPXMLFileHandler.h"
#include "CAPModelLVPS4X4.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

//class Cmiss_node;
class wxPanel;
		
extern "C"
{
#include "api/cmiss_time_keeper.h"
#include "api/cmiss_time.h"
#include "command/cmiss.h"
#include "graphics/scene.h"	
#include "graphics/scene_viewer.h"
#include "three_d_drawing/graphics_buffer.h"
//#include "general/debug.h"
#include "finite_element/export_finite_element.h"
}

namespace cap
{

namespace
{
const char* ModeStrings[] = {
		"Apex",
		"Base",
		"RV Inserts",
		"Baseplane Points",
		"Guide Points"
};//REVISE
}

template <typename MainWindow, typename CmguiManager>
class MainApp : ImageBrowseWindowClient
{
	static int input_callback(struct Scene_viewer *scene_viewer, 
			struct Graphics_buffer_input *input, void *viewer_frame_void)
	{
	//	cout << "input_callback() : input_type = " << input->type << endl;
	//	if (input->type == GRAPHICS_BUFFER_KEY_PRESS)
	//	{
	//		int keyCode = input->key_code;
	//		cout << "Key pressed = " << keyCode << endl;
	//		return 0;
	//	}
		
		if (!(input->input_modifier & GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT))
		{
			return 1;
		}
		
		static Cmiss_node_id selectedNode = NULL; // Thread unsafe
		MainApp* app = static_cast<MainApp*>(viewer_frame_void);
		
		// We have to stop the animation when the user clicks on the 3D panel.
		// Since dragging a point while cine is playing can cause a problem
		// But Is this the best place put this code?
		app->StopCine();
		
		double x = (double)(input->position_x);
		double y = (double)(input->position_y);
		double time = app->GetCurrentTime(); // TODO REVISE
		if (input->type == GRAPHICS_BUFFER_BUTTON_PRESS)
		{
			// Select node or create one
			std::cout << "Mouse clicked, time = " << time << '\n';
			std::cout << "Mouse button number = " << input->button_number << '\n';
			
			Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
			Point3D coords;
			selectedNode = Cmiss_select_node_from_screen_coords(scene_viewer, x, y, time, coords);
						
			if (input->button_number == wxMOUSE_BTN_LEFT )
			{	
				if (!selectedNode) //REVISE
				{
					if (selectedNode = Cmiss_create_or_select_node_from_screen_coords(scene_viewer, x, y, time, coords)) 
					{
						app->AddDataPoint(selectedNode, coords);
					}
				}
			}
			else if (input->button_number == wxMOUSE_BTN_RIGHT)
			{
				if (selectedNode)
				{
					app->RemoveDataPoint(selectedNode);
					selectedNode = 0;
				}
			}
		}
		else if (input->type == GRAPHICS_BUFFER_MOTION_NOTIFY)
		{
			// Move node		
			if (!selectedNode)
			{
				std::cout << "GRAPHICS_BUFFER_MOTION_NOTIFY with NULL selectedNode" << '\n';
	//			frame->InitialiseModel();
				return 0;
			}
			Point3D coords;
	//		cout << "Mouse Drag node = " << Cmiss_node_get_identifier(selectedNode) << endl;
			Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
			Cmiss_move_node_to_screen_coords(scene_viewer, selectedNode, x, y, time, coords);
			
	//		cout << "Move coord = " << coords << endl;
			app->MoveDataPoint(selectedNode, coords);
		}
		else if (input->type == GRAPHICS_BUFFER_BUTTON_RELEASE)
		{
			std::cout << "Mouse released" << '\n';
			app->SmoothAlongTime();
			selectedNode = NULL;
		}
		
		return 0; // returning false means don't call the other input handlers;
	}

	static int input_callback_image_shifting(struct Scene_viewer *scene_viewer, 
			struct Graphics_buffer_input *input, void *viewer_frame_void)
	{
	//	cout << "input_callback_image_shifting() : input_type = " << input->type << endl;

		if (!(input->input_modifier & GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT))
		{
			return 1;
		}
		
	//	static Cmiss_node_id selectedNode = NULL; // Thread unsafe
	//	MainWindow* frame = static_cast<MainWindow*>(viewer_frame_void);
		
		double x = (double)(input->position_x);
		double y = (double)(input->position_y);
		
		static double coords[3];
		static Cmiss_region_id selectedRegion;
		if (input->type == GRAPHICS_BUFFER_BUTTON_PRESS)
		{
			// Select node or create one
			std::cout << "Mouse button number = " << input->button_number << '\n';
			
			MainApp* app = static_cast<MainApp*>(viewer_frame_void);
			Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
			selectedRegion = Cmiss_get_slice_region(scene_viewer, x, y, (double*)coords, (Cmiss_region_id)0);
			if (selectedRegion)
			{
				std::string sliceName = Cmiss_region_get_path(selectedRegion);
				std::cout << "selected = " << sliceName << '\n';
			}

		}
		else if (input->type == GRAPHICS_BUFFER_MOTION_NOTIFY)
		{
			double new_coords[3];
			//Cmiss_region_id selectedRegion = Cmiss_get_slice_region(x, y, (double*)new_coords, selectedRegion);
			if (selectedRegion)
			{
				MainApp* app = static_cast<MainApp*>(viewer_frame_void);
				Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
				Cmiss_region_id tempRegion = Cmiss_get_slice_region(scene_viewer, x, y, (double*)new_coords, selectedRegion);
				if (!tempRegion)
				{
					std::cout << __func__ << ": ERROR\n";
					return 0;
				}
	//			string sliceName = Cmiss_region_get_path(selectedRegion);
	//			cout << "dragged = " << sliceName << endl;
	//			cout << "coords = " << coords[0] << ", " << coords[1] << ", " << coords[2] << "\n";
	//			cout << "new_coords = " << new_coords[0] << ", " << new_coords[1] << ", " << new_coords[2] << "\n";
				for (int nodeNum = 1; nodeNum < 5; nodeNum++)
				{
					char nodeName[256];
					sprintf(nodeName,"%d", nodeNum);
					if (Cmiss_node* node = Cmiss_region_get_node(selectedRegion, nodeName))
					{
						FE_value x, y, z;
						FE_node_get_position_cartesian(node, 0, &x, &y, &z, 0);
	//					cout << "before = " << x << ", " << y << ", " << z << endl;
						x += (new_coords[0] - coords[0]);
						y += (new_coords[1] - coords[1]);
						z += (new_coords[2] - coords[2]);
	//					cout << "after = " << x << ", " << y << ", " << z << "\n" << endl ;
						FE_node_set_position_cartesian(node, 0, x, y, z);
					}
				}
				for (int i = 0; i<3; i++)
				{
					coords[i] = new_coords[i];
				}
			}
		}
		else if (input->type == GRAPHICS_BUFFER_BUTTON_RELEASE)
		{
			std::cout << "Mouse released" << '\n';
		}
		
		return 0; // returning false means don't call the other input handlers;
	}

	static int time_callback(struct Time_object *time, double current_time, void *user_data)
	{
		//DEBUG
		std::cout << "Time_call_back time = " << current_time << '\n';
		
		MainApp* app = static_cast<MainApp*>(user_data);
		app->SetTime(current_time);
		
		app->Refresh3DCanvas(); // this forces refresh even when UI is being manipulated by user

		return 0;
	}
	
public:
	static
	MainApp* CreateMainApp(CmguiManager const& manager)
	{
		MainApp<MainWindow, CmguiManager>* mainApp = new MainApp(manager);
		MainWindow *frame = new MainWindow(*mainApp);
		mainApp->SetMainWindow(frame);
		mainApp->Initialize();
		frame->Show(true);
		return mainApp;
	}
	
	~MainApp()
	{
		delete imageSet_;
		delete modeller_;
	}
	
	void SetMainWindow(MainWindow* win)
	{
		gui_ = win;
	}

	Cmiss_scene_viewer_id GetCmissSceneViewer() const
	{
		return sceneViewer_;
	}
	
	void OnTogglePlay()
	{
		if (animationIsOn_)
		{
			StopCine();
		}
		else
		{
			PlayCine();
		}
		return;
	}
	
	void SetImageVisibility(bool visibility, std::string const& sliceName = "")
	{
		if (sliceName.length()) //REVISE
		{
			const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();

			int i = find(sliceNames.begin(),sliceNames.end(), sliceName) - sliceNames.begin();
			assert(i < sliceNames.size());
			SetImageVisibility(visibility, i);
		}
		else
		{
			for (int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
			{
				SetImageVisibility(visibility, i);
			}
		}
	}
	
	void SetImageVisibility(bool visibility, int index)
	{
		if (miiIsOn_)	
			heartModelPtr_->SetMIIVisibility(visibility, index);
		imageSet_->SetVisible(visibility, index);
	}
	
	void OnSliceSelected(std::string const& sliceName)
	{
		//called from OnObjectCheckListSelected
		
		const ImagePlane& plane = imageSet_->GetImagePlane(sliceName);
			
		// compute the center of the image plane, eye(camera) position and the up vector
		Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
		Point3D eye = planeCenter - (plane.normal * 500); // this seems to determine the near clip plane
		Vector3D up(plane.yside);
		up.Normalise();
		
		//Hack :: perturb direction vector a little
		eye.x *= 1.01; //HACK 1.001 makes the iso lines partially visible
		
		if (!Cmiss_scene_viewer_set_lookat_parameters_non_skew(
				sceneViewer_, eye.x, eye.y, eye.z,
				planeCenter.x, planeCenter.y, planeCenter.z,
				up.x, up.y, up.z
				))
		{
			//Error;
		}
		Refresh3DCanvas();
		Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
	}
	
	void OnAnimationSliderEvent(int value, int min, int max)
	{
		if (!heartModelPtr_) //FIXME
		{
			return;
		}
		
		double time =  (double)(value - min) / (double)(max - min);
		double prevFrameTime = heartModelPtr_->MapToModelFrameTime(time);
		if ((time - prevFrameTime) < (0.5)/(heartModelPtr_->GetNumberOfModelFrames()))
		{
			time = prevFrameTime;
		}
		else
		{
			time = prevFrameTime + (double)1/(heartModelPtr_->GetNumberOfModelFrames());
		}

		// fix for the bug where the client crashes on linux
		int newFrame = static_cast<int>(time * (max - min));
		if (newFrame == value)
		{
			return;
		}
		
	//	cout << __func__ << ": time = " << time << endl;;
	//	imageSet_->SetTime(time);
		time = (time > 0.99) ? 0 : time;
		
		Time_keeper_request_new_time(timeKeeper_, time);
		
		Refresh3DCanvas(); // forces redraw while silder is manipulated
	}
	
	void OnAnimationSpeedControlEvent(double speed)
	{
		Time_keeper_set_speed(timeKeeper_, speed);	
		Refresh3DCanvas(); // forces redraw while silder is manipulated
	}
	
	void Refresh3DCanvas()
	{
		if (sceneViewer_) 
		{
			Scene_viewer_redraw_now(sceneViewer_);
		}
	}
	
	double GetCurrentTime() const
	{
		Cmiss_time_keeper_get_time(timeKeeper_);
	}
	
	void SetTime(double time)
	{
		//cout << "SetTime" <<endl;
		imageSet_->SetTime(time);
		
		assert(heartModelPtr_);
		int frameNumber = heartModelPtr_->MapToModelFrameNumber(time);

		gui_->SetTime(time, frameNumber);
	}
	
	void OnToggleHideShowAll() {}//????
	
	void OnToggleHideShowOthers() {}//?????
	
	void OnMIICheckBox(bool checked)
	{
		miiIsOn_ = checked;
		assert(heartModelPtr_);
		for (int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
		{
			if (gui_->IsSliceChecked(i))
			{
				heartModelPtr_->SetMIIVisibility(checked,i);
			}
		}
	}
	
	void OnWireframeCheckBox(bool checked)
	{
		assert(heartModelPtr_);
		heartModelPtr_->SetModelVisibility(checked);
	}
	
	void SetImageBrightness(double brightness)
	{
		imageSet_->SetBrightness(brightness);
		Refresh3DCanvas();
	}
	
	void SetImageContrast(double contrast)
	{
		imageSet_->SetContrast(contrast);
		Refresh3DCanvas();
	}
	
	void ProcessDataPointsEnteredForCurrentMode()
	{
		if (modeller_->OnAccept())
		{
			CAPModeller::ModellingMode mode = modeller_->GetCurrentMode();
			gui_->UpdateModeSelectionUI(mode);
			if (mode == CAPModeller::GUIDEPOINT)
			{
				UpdateMII();
				EnterModelLoadedState();
			}
		}
		
		Refresh3DCanvas();
	}
	
	void ChangeModellingMode(int mode)
	{
		modeller_->ChangeMode((CAPModeller::ModellingMode) mode);//FIX type unsafe
		gui_->UpdateModeSelectionUI(mode);
		Refresh3DCanvas();
	}
	
	void LoadImages(SlicesWithImages const& slices) // name misleading?
	{
		assert(!slices.empty());

		if(imageSet_)
		{
			imageSet_->SetVisible(false); // HACK should really destroy region
			delete imageSet_;
		}

		ImageSetBuilder builder(slices, cmguiManager_);
		imageSet_ = builder.build();
		imageSet_->SetVisible(true);//FIXME
		Cmiss_scene_viewer_view_all(sceneViewer_);
		
		this->PopulateSliceList(); // fill in slice check box list
	}
	
	void LoadImagesFromXMLFile(SlicesWithImages const& slices)
	{
		LoadImages(slices);
		EnterImagesLoadedState();
	}
	
	virtual void LoadImagesFromImageBrowseWindow(SlicesWithImages const& slices, CardiacAnnotation const& anno)
	{
		// Reset capXMLFilePtr_
		if (capXMLFilePtr_)
		{
			capXMLFilePtr_.reset(0);
		}
	//	// Reset the state of the MainWindow
	//	EnterInitState(); // this re-registers the input call back -REVISE
		
		LoadImages(slices);
		InitializeModelTemplate(slices);
		
		// Create DataPoints if corresponding annotations exist in the CardiacAnnotation
		assert(!anno.imageAnnotations.empty());
		
		cardiacAnnotationPtr_.reset(new CardiacAnnotation(anno));
		
		// The following code should only execute when reading a pre-defined annotation file
		Cmiss_context_id cmiss_context = cmguiManager_.GetCmissContext();
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
								Cmiss_field_id field = Cmiss_region_find_field_by_name(region, "coordinates_rect");
								Cmiss_node_id cmissNode = Cmiss_create_data_point_at_coord(region,
												field, (double*) coords, time);

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
								Cmiss_field_id field = Cmiss_region_find_field_by_name(region, "coordinates_rect");
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
	
	void SetModelVisibility(bool visibility) {}
	
	void SetMIIVisibility(bool visibility, int sliceNum) {}

	void OpenImages(std::string const& imageDirname)
	{
		ImageBrowser<ImageBrowseWindow, CmguiManager>* ib = 
					ImageBrowser<ImageBrowseWindow, CmguiManager>::CreateImageBrowser(std::string(imageDirname), cmguiManager_, *this);
	}
	
	void OpenModel(std::string const& filename)
	{
		cardiacAnnotationPtr_.reset(0);
	//		CAPXMLFile xmlFile(filename.c_str());
		capXMLFilePtr_.reset(new CAPXMLFile(filename.c_str()));
		CAPXMLFile& xmlFile(*capXMLFilePtr_);
		std::cout << "Start reading xml file\n";
		xmlFile.ReadFile();
		
		CAPXMLFileHandler xmlFileHandler(xmlFile);
		SlicesWithImages const& slicesWithImages = xmlFileHandler.GetSlicesWithImages(cmguiManager_);
		if (slicesWithImages.empty())
		{
			std::cout << "Can't locate image files\n";
			return;
		}

		// TODO clean up first
		LoadImagesFromXMLFile(slicesWithImages);

		std::vector<DataPoint> dataPoints = xmlFileHandler.GetDataPoints(cmguiManager_);

		std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
		std::cout << "number of exnodeFilenames = " << exnodeFileNames.size() << '\n';
		if (exnodeFileNames.empty())
		{
			// This means no output element is defined
			InitializeModelTemplate(slicesWithImages);
			modeller_->SetDataPoints(dataPoints);
			// FIXME memory is prematurely released when ok button is pressed from the following window
			// Suppress this feature for now
	//			ImageBrowseWindow *frame = new ImageBrowseWindow(slicesWithImages, cmguiManager_, *this);
	//			frame->Show(true);
			
			//HACK : uncommenting the following will enable models to be constructed from model files with
			// only the input element defined.
			EnterModelLoadedState();
			
			gui_->UpdateModeSelectionUI(CAPModeller::GUIDEPOINT);
			Refresh3DCanvas();
			
			return;
		}
		
		std::string const& exelemFileName = xmlFile.GetExelemFileName();

		//HACK FIXME
		std::string xmlFilename = filename.c_str();
		size_t positionOfLastSlash = xmlFilename.find_last_of("/\\");
		std::string modelFilePath = xmlFilename.substr(0, positionOfLastSlash);
		std::cout << "modelFilePath = " << modelFilePath << '\n';

		heartModelPtr_.reset(new CAPModelLVPS4X4("heart", cmguiManager_.GetCmissContext()));
		assert(heartModelPtr_);
		heartModelPtr_->SetFocalLengh(xmlFile.GetFocalLength());
		int numberOfModelFrames = exnodeFileNames.size();
		heartModelPtr_->SetNumberOfModelFrames(numberOfModelFrames);
		LoadHeartModel(modelFilePath, exnodeFileNames);
		gtMatrix m;
		xmlFile.GetTransformationMatrix(m);
		heartModelPtr_->SetLocalToGlobalTransformation(m);
		modeller_->SetDataPoints(dataPoints);
		UpdateMII();

		gui_->UpdateModeSelectionUI(CAPModeller::GUIDEPOINT);
		Refresh3DCanvas();
	}
	
	void OpenAnnotation(std::string const& filename, std::string const& imageDirname)
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
		
		ImageBrowser<ImageBrowseWindow, CmguiManager>* ib = 
				ImageBrowser<ImageBrowseWindow, CmguiManager>::CreateImageBrowser(imageDirname, cmguiManager_, *this);
		
		// Set annotations to the images in the ImageBrowseWindow.
		ib->SetAnnotation(annotationFile.GetCardiacAnnotation());
	}
	
	void SaveModel(std::string const& dirname, std::string const& userComment)
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
	
	void StartPlaneShift()
	{
		Cmiss_scene_viewer_remove_input_callback(sceneViewer_,
						input_callback, (void*)this);
		Cmiss_scene_viewer_add_input_callback(sceneViewer_,
						input_callback_image_shifting, (void*)this, 1/*add_first*/);
	}
	
	void FinishPlaneShift()
	{
		Cmiss_scene_viewer_remove_input_callback(sceneViewer_,
						input_callback_image_shifting, (void*)this);
		Cmiss_scene_viewer_add_input_callback(sceneViewer_,
						input_callback, (void*)this, 1/*add_first*/);
		
		imageSet_->SetShiftedImagePosition();
	}
	
private:
	
	void Initialize()
	{
		assert(gui_);
		wxPanel* panel = gui_->Get3DPanel();
		sceneViewer_ = cmguiManager_.CreateSceneViewer(panel);
		Cmiss_scene_viewer_view_all(sceneViewer_);
		Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
		
		EnterInitState();
	}
	
	void EnterInitState() 
	{
		gui_->EnterInitState();
		
		// Initialize input callback
		Scene_viewer_add_input_callback(sceneViewer_, input_callback, (void*)this, 1/*add_first*/);

		// Also clean up cmgui objects such as scene, regions, materials ..etc
		capXMLFilePtr_.reset(0);
		cardiacAnnotationPtr_.reset(0);
		if(imageSet_)
		{
			delete imageSet_;
			imageSet_ = 0;
		}
		heartModelPtr_.reset(0);

		mainWindowState_ = INIT_STATE;
	}
	
	void EnterImagesLoadedState()
	{
		gui_->EnterImagesLoadedState();
		
		StopCine();
		
		// Initialize timer for animation
		size_t numberOfLogicalFrames = imageSet_->GetNumberOfFrames(); // smallest number of frames of all slices
		if (timeNotifier_)
		{
			Cmiss_time_keeper_remove_time_notifier(timeKeeper_, timeNotifier_);
			Cmiss_time_notifier_destroy(&timeNotifier_);
		}
		timeNotifier_ = Cmiss_time_keeper_create_notifier_regular(timeKeeper_, numberOfLogicalFrames, 0);
		Cmiss_time_notifier_add_callback(timeNotifier_, time_callback, (void*)this);
		Time_keeper_set_minimum(timeKeeper_, 0); // FIXME time range is always 0~1
		Time_keeper_set_maximum(timeKeeper_, 1);

		mainWindowState_ = IMAGES_LOADED_STATE;
	}
	
	void EnterModelLoadedState()
	{
		gui_->EnterModelLoadedState();
		
		StopCine();
		assert(heartModelPtr_);
		heartModelPtr_->SetModelVisibility(true);
		
		mainWindowState_ = MODEL_LOADED_STATE;
	}
	
	void PlayCine()
	{
		Time_keeper_play(timeKeeper_,TIME_KEEPER_PLAY_FORWARD);
		Time_keeper_set_play_loop(timeKeeper_);
		//Time_keeper_set_play_every_frame(timeKeeper_);
		Time_keeper_set_play_skip_frames(timeKeeper_);
		this->animationIsOn_ = true;
		
		gui_->PlayCine();
	}
	
	void StopCine()
	{
		Time_keeper_stop(timeKeeper_);
		this->animationIsOn_ = false;
		
		gui_->StopCine();
	}
	
	void Terminate()
	{
		// ???????
		delete this;
	}
	
	void PopulateSliceList()
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
	
	void AddDataPoint(Cmiss_node* dataPointID, Point3D const& position)
	{
		modeller_->AddDataPoint(dataPointID, position, GetCurrentTime());
		Refresh3DCanvas(); // need to force refreshing
	}
	
	void MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition)
	{
		modeller_->MoveDataPoint(dataPointID, newPosition, GetCurrentTime());
		Refresh3DCanvas(); // need to force refreshing
	}
	
	void RemoveDataPoint(Cmiss_node* dataPointID) 
	{
		modeller_->RemoveDataPoint(dataPointID, GetCurrentTime());
		Refresh3DCanvas();
	}
	
	void SmoothAlongTime()
	{
		if (!modeller_)
		{
			return;//FIXME
		}
		modeller_->SmoothAlongTime();
		Refresh3DCanvas();
		
		assert(heartModelPtr_);
		std::cout << "ED Volume(EPI) = " << heartModelPtr_->ComputeVolume(EPICARDIUM, 0) << '\n';
		std::cout << "ED Volume(ENDO) = " << heartModelPtr_->ComputeVolume(ENDOCARDIUM, 0) << '\n';
	}
	
	void InitializeMII()
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
			Cmiss_context_execute_command(context_, str);

			sprintf((char*)str, "gfx modify g_element heart iso_surfaces exterior iso_scalar slice_%s iso_values 100 use_faces select_on material gold selected_material default_selected render_shaded line_width 2;"
						,sliceName.c_str());
			Cmiss_context_execute_command(context_, str);
		}
	}
	
	void UpdateMII()
	{
		using namespace std;
		const vector<string>& sliceNames = imageSet_->GetSliceNames();
		vector<string>::const_iterator itr = sliceNames.begin();
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
			Cmiss_context_execute_command(context_, str);
			
			Point3D pointTLCTransformed = mInv * plane.tlc;
			double d = DotProduct((pointTLCTransformed - Point3D(0,0,0)), normalTransformed);
			heartModelPtr_->UpdateMII(index, d);
		}
	}
	
	struct ComparatorForNumFrames
	{
		bool operator() (SliceInfo const& a, SliceInfo const& b)
		{
			return (a.GetDICOMImages().size() < b.GetDICOMImages().size());
		}
	};

	void InitializeModelTemplate(SlicesWithImages const& slices)
	{
		SlicesWithImages::const_iterator 
			itrToMinNumberOfFrames = std::min_element(slices.begin(), slices.end(),
														ComparatorForNumFrames());
		int minNumberOfFrames = itrToMinNumberOfFrames->GetDICOMImages().size();
		
		heartModelPtr_.reset(new CAPModelLVPS4X4("heart", cmguiManager_.GetCmissContext()));
		assert(heartModelPtr_);
		heartModelPtr_->SetNumberOfModelFrames(minNumberOfFrames);
		LoadTemplateHeartModel("heart", std::string(CAP_DATA_DIR) + "templates/" ); //HACK FIXME
//		XRCCTRL(*this, "MII", wxCheckBox)->SetValue(false); FIXME
//		XRCCTRL(*this, "Wireframe", wxCheckBox)->SetValue(false);
		gui_->EnterInitState();
		heartModelPtr_->SetMIIVisibility(false);
		heartModelPtr_->SetModelVisibility(false);
	}
	
	void CreateModeller()
	{
		if (modeller_)
		{
			delete modeller_;
		}
		assert(heartModelPtr_);
		modeller_ = new CAPModeller(*heartModelPtr_); // initialise modeller and all the data points
	}
	
	void UpdateStatesAfterLoadingModel()
	{
		CreateModeller();
		gui_->UpdateModeSelectionUI(CAPModeller::APEX);
		
		UpdateModelVisibilityAccordingToUI();
		
		InitializeMII(); // This turns on all MII's
		UpdateMIIVisibilityAccordingToUI();
	}
	
	void LoadTemplateHeartModel(std::string const dirname, std::string const& path)
	{
		// This function is used to load the unfitted generic heart model.
		// Currently this is necessary. It might be possible to eliminate the use of generic model
		// in the future by creating the related cmgui nodes, fields and element completely thru cmgui api
		assert(heartModelPtr_);
		heartModelPtr_->ReadModelFromFiles(dirname, path);
		UpdateStatesAfterLoadingModel();
	}

	void UpdateModelVisibilityAccordingToUI()
	{
		assert(heartModelPtr_);
		heartModelPtr_->SetModelVisibility(wireFrameIsOn_);
	}
	
	void UpdateMIIVisibilityAccordingToUI()
	{
		// Update the visibility of each mii according to the ui status
		// ( = mii checkbox and the slice list)
		if (miiIsOn_)
		{
			const int numberOfSlices = imageSet_->GetNumberOfSlices();
			for (int i = 0; i < numberOfSlices; i++)
			{
				std::cout << "slice num = " << i << ", isChecked = " << gui_->IsSliceChecked(i) << '\n';
				if (!gui_->IsSliceChecked(i))
				{
					heartModelPtr_->SetMIIVisibility(false,i);
				}
			}
		}
		else
		{
			heartModelPtr_->SetMIIVisibility(false);
		}
	}
	
	void LoadHeartModel(std::string const& path, std::vector<std::string> const& modelFilenames)
	{
		assert(heartModelPtr_);
		heartModelPtr_->ReadModelFromFiles(path, modelFilenames);
		UpdateStatesAfterLoadingModel();
		EnterModelLoadedState();
	}
	
	// Private Constructor - This class should be instantiated from the static factory method
	MainApp(CmguiManager const& manager)
	:
		cmguiManager_(manager),
		imageSet_(0),
		animationIsOn_(false),
		hideAll_(false),
		miiIsOn_(false),
		wireFrameIsOn_(false),
		heartModelPtr_(0),
		modeller_(0),
		capXMLFilePtr_(0),
		cardiacAnnotationPtr_(0),
		gui_(0),
		context_(manager.GetCmissContext()),
		timeKeeper_(Cmiss_context_get_default_time_keeper(manager.GetCmissContext())),
		timeNotifier_(0)
	{}
	
	MainWindow* gui_;
	CmguiManager const& cmguiManager_;
	
	Cmiss_context_id context_;
	Cmiss_time_keeper* timeKeeper_;
	Cmiss_time_notifier* timeNotifier_;
	ImageSet* imageSet_;
	
	bool animationIsOn_;
	bool hideAll_;
	bool miiIsOn_;
	bool wireFrameIsOn_;
	
	boost::scoped_ptr<CAPModelLVPS4X4> heartModelPtr_;
	
	CAPModeller* modeller_;
	
	Cmiss_scene_viewer_id sceneViewer_;

	enum MainWindowState
	{
		INIT_STATE,
		IMAGES_LOADED_STATE,
		MODEL_LOADED_STATE
	};

	MainWindowState mainWindowState_;
	boost::scoped_ptr<CAPXMLFile> capXMLFilePtr_;
	boost::scoped_ptr<CardiacAnnotation> cardiacAnnotationPtr_;
};

} // namespace cap

#endif /* MAINAPP_H_ */
