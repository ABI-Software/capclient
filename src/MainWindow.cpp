#include "wx/xrc/xmlres.h"
#include <wx/dir.h>

//#include "wx/splitter.h"
#include <wx/aboutdlg.h>

#include "Config.h"
#include "CAPTypes.h"
#include "MainWindow.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
#include "ImageSet.h"
#include "CmguiExtensions.h"
#include "ImageBrowseWindow.h"
#include "CAPHtmlWindow.h"
#include "CAPXMLFile.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>

#include <boost/foreach.hpp>

extern "C"
{
#include "api/cmiss_time_keeper.h"
#include "api/cmiss_time.h"
#include "command/cmiss.h"
#include "graphics/scene.h"	
#include "graphics/scene_viewer.h"
#include "three_d_drawing/graphics_buffer.h"
//#include "general/debug.h"
}

using namespace std;

namespace cap 
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
	MainWindow* frame = static_cast<MainWindow*>(viewer_frame_void);
	
	double x = (double)(input->position_x);
	double y = (double)(input->position_y);
	double time = frame->GetCurrentTime(); // TODO REVISE
	if (input->type == GRAPHICS_BUFFER_BUTTON_PRESS)
	{
		// Select node or create one
		cout << "Mouse clicked, time = " << time << endl;
		cout << "Mouse button number = " << input->button_number << endl;
		
		Cmiss_scene_viewer_id scene_viewer = frame->GetCmissSceneViewer();
		Point3D coords;
		selectedNode = Cmiss_select_node_from_screen_coords(scene_viewer, x, y, time, coords);
					
		if (input->button_number == wxMOUSE_BTN_LEFT )
		{	
			if (!selectedNode) //REVISE
			{
				if (selectedNode = Cmiss_create_or_select_node_from_screen_coords(scene_viewer, x, y, time, coords)) 
				{
					frame->AddDataPoint(selectedNode, coords);
				}
			}
		}
		else if (input->button_number == wxMOUSE_BTN_RIGHT)
		{
			if (selectedNode)
			{
				frame->RemoveDataPoint(selectedNode);
				selectedNode = 0;
			}
		}
	}
	else if (input->type == GRAPHICS_BUFFER_MOTION_NOTIFY)
	{
		// Move node		
		if (!selectedNode)
		{
			cout << "GRAPHICS_BUFFER_MOTION_NOTIFY with NULL selectedNode" << endl;
//			frame->InitialiseModel();
			return 0;
		}
		Point3D coords;
//		cout << "Mouse Drag node = " << Cmiss_node_get_identifier(selectedNode) << endl;
		Cmiss_scene_viewer_id scene_viewer = frame->GetCmissSceneViewer();
		Cmiss_move_node_to_screen_coords(scene_viewer, selectedNode, x, y, time, coords);
		
//		cout << "Move coord = " << coords << endl;
		frame->MoveDataPoint(selectedNode, coords);
//		frame->RemoveDataPoint(selectedNode);
//		selectedNode = 0;
	}
	else if (input->type == GRAPHICS_BUFFER_BUTTON_RELEASE)
	{
		cout << "Mouse released" << endl;
		frame->SmoothAlongTime();
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
		cout << "Mouse button number = " << input->button_number << endl;
		
		MainWindow* frame = static_cast<MainWindow*>(viewer_frame_void);
		Cmiss_scene_viewer_id scene_viewer = frame->GetCmissSceneViewer();
		selectedRegion = Cmiss_get_slice_region(scene_viewer, x, y, (double*)coords, (Cmiss_region_id)0);
		if (selectedRegion)
		{
			string sliceName = Cmiss_region_get_path(selectedRegion);
			cout << "selected = " << sliceName << endl;
		}

	}
	else if (input->type == GRAPHICS_BUFFER_MOTION_NOTIFY)
	{
		double new_coords[3];
		//Cmiss_region_id selectedRegion = Cmiss_get_slice_region(x, y, (double*)new_coords, selectedRegion);
		if (selectedRegion)
		{
			MainWindow* frame = static_cast<MainWindow*>(viewer_frame_void);
			Cmiss_scene_viewer_id scene_viewer = frame->GetCmissSceneViewer();
			Cmiss_region_id tempRegion = Cmiss_get_slice_region(scene_viewer, x, y, (double*)new_coords, selectedRegion);
			if (!tempRegion)
			{
				cout << __func__ << "ERROR\n";
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
		cout << "Mouse released" << endl;
	}
	
	return 0; // returning false means don't call the other input handlers;
}

static int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
//	cout << "Time_call_back time = " << current_time << endl;
	
	MainWindow* frame = static_cast<MainWindow*>(user_data);
	frame->SetTime(current_time);
	
	frame->RefreshCmguiCanvas(); // this forces refresh even when UI is being manipulated by user

	return 0;
}

MainWindow::MainWindow(CmguiManager const& cmguiManager)
: 
	cmguiManager_(cmguiManager),
	context_(cmguiManager.GetCmissContext()),
	animationIsOn_(false),
	hideAll_(true),
	timeKeeper_(Cmiss_context_get_default_time_keeper(cmguiManager.GetCmissContext())),
	heartModel_("heart", cmguiManager.GetCmissContext()),
	modeller_(new CAPModeller(heartModel_)),
	imageSet_(0)
{
	// Load layout from .xrc file
	wxXmlResource::Get()->Load("MainWindow.xrc");
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("MainWindow"));
	
	// GUI initialization
	CreateStatusBar(0);

	// Initialize check box list of scene objects (image slices)
	objectList_ = XRCCTRL(*this, "SliceList", wxCheckListBox);
	objectList_->SetSelection(wxNOT_FOUND);
	objectList_->Clear();
	
	m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	sceneViewer_ = cmguiManager_.CreateSceneViewer(m_pPanel);
	Cmiss_scene_viewer_view_all(sceneViewer_);
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
	
	this->Fit();

	EnterInitState();
}

MainWindow::~MainWindow()
{
	delete imageSet_;
	delete modeller_;
}

double MainWindow::GetCurrentTime() const
{
	return Cmiss_time_keeper_get_time(timeKeeper_);
}

void MainWindow::AddDataPoint(Cmiss_node* dataPointID, const Point3D& position)
{
	modeller_->AddDataPoint(dataPointID, position, GetCurrentTime());
	RefreshCmguiCanvas(); // need to force refreshing
}

void MainWindow::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& newPosition)
{
	modeller_->MoveDataPoint(dataPointID, newPosition, GetCurrentTime());
	RefreshCmguiCanvas(); // need to force refreshing
}

void MainWindow::RemoveDataPoint(Cmiss_node* dataPointID)
{
//	cout << __func__ << endl;
	modeller_->RemoveDataPoint(dataPointID, GetCurrentTime());
	RefreshCmguiCanvas();
}

void MainWindow::SmoothAlongTime()
{
	modeller_->SmoothAlongTime();
	RefreshCmguiCanvas();
	
	cout << "ED Volume(EPI) = " << heartModel_.ComputeVolume(EPICARDIUM, 0) << endl;
	cout << "ED Volume(ENDO) = " << heartModel_.ComputeVolume(ENDOCARDIUM, 0) << endl;
}

//void DisableWidgetByName(wxWindow& w, std::string const& name)
//{
//	wxWindow* widget = XRCCTRL(w, name.c_str(), wxWindow);
//	widget->Disable();
//}
//
//void EnableWidgetByName(wxWindow& w, std::string const& name)
//{
//	wxWindow* widget = XRCCTRL(w, name.c_str(), wxWindow);
//	widget->Enable();
//}

template <typename Widget>
Widget* MainWindow::GetWidgetByName(std::string const& name)
{
	Widget* widget = XRCCTRL(*this, name.c_str(), Widget);
	return  widget;
}

void MainWindow::EnterInitState()
{
	// Put the gui in the init state
	GetWidgetByName<wxSlider>("AnimationSlider")->Enable(false);
	GetWidgetByName<wxSlider>("AnimationSpeedControl")->Enable(false);
	GetWidgetByName<wxButton>("PlayButton")->Enable(false);
	GetWidgetByName<wxButton>("HideShowAll")->Enable(false);
	GetWidgetByName<wxButton>("HideShowOthers")->Enable(false);
	GetWidgetByName<wxCheckBox>("MII")->Enable(false);
	GetWidgetByName<wxCheckBox>("Wireframe")->Enable(false);
	GetWidgetByName<wxSlider>("BrightnessSlider")->Enable(false);
	GetWidgetByName<wxSlider>("ContrastSlider")->Enable(false);
	GetWidgetByName<wxChoice>("ModeChoice")->Enable(false);
	GetWidgetByName<wxButton>("AcceptButton")->Enable(false);
	GetWidgetByName<wxButton>("PlaneShiftButton")->Enable(false);

	GetMenuBar()->FindItem(XRCID("OpenModelMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("SaveMenuItem"))->Enable(false);
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(false);

	// Initialize input callback
	Scene_viewer_add_input_callback(sceneViewer_, input_callback, (void*)this, 1/*add_first*/);

	// Also clean up cmgui objects such as scene, regions, materials ..etc

	mainWindowState_ = INIT_STATE;
}

void MainWindow::EnterImagesLoadedState()
{
	GetWidgetByName<wxSlider>("AnimationSlider")->Enable(true);
	GetWidgetByName<wxSlider>("AnimationSpeedControl")->Enable(true);
	GetWidgetByName<wxButton>("PlayButton")->Enable(true);
	GetWidgetByName<wxButton>("HideShowAll")->Enable(true);
	GetWidgetByName<wxButton>("HideShowOthers")->Enable(true);
	GetWidgetByName<wxCheckBox>("MII")->Enable(false);
	GetWidgetByName<wxCheckBox>("Wireframe")->Enable(false);
	GetWidgetByName<wxSlider>("BrightnessSlider")->Enable(true);
	GetWidgetByName<wxSlider>("ContrastSlider")->Enable(true);
	GetWidgetByName<wxChoice>("ModeChoice")->Enable(true);
	GetWidgetByName<wxButton>("AcceptButton")->Enable(true);
	GetWidgetByName<wxButton>("PlaneShiftButton")->Enable(true);

	GetMenuBar()->FindItem(XRCID("OpenModelMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("SaveMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(false);

	// Initialize timer for animation
	size_t numberOfLogicalFrames = imageSet_->GetNumberOfFrames(); // smallest number of frames of all slices
	Cmiss_time_notifier_id time_notifier = Cmiss_time_keeper_create_notifier_regular(timeKeeper_, numberOfLogicalFrames, 0);
	Cmiss_time_notifier_add_callback(time_notifier, time_callback, (void*)this);
	Time_keeper_set_minimum(timeKeeper_, 0); // FIXME time range is always 0~1
	Time_keeper_set_maximum(timeKeeper_, 1);

	mainWindowState_ = IMAGES_LOADED_STATE;
}

void MainWindow::EnterModelLoadedState()
{
	GetWidgetByName<wxSlider>("AnimationSlider")->Enable(true);
	GetWidgetByName<wxSlider>("AnimationSpeedControl")->Enable(true);
	GetWidgetByName<wxButton>("PlayButton")->Enable(true);
	GetWidgetByName<wxButton>("HideShowAll")->Enable(true);
	GetWidgetByName<wxButton>("HideShowOthers")->Enable(true);
	GetWidgetByName<wxCheckBox>("MII")->Enable(true);
	GetWidgetByName<wxCheckBox>("Wireframe")->Enable(true);
	GetWidgetByName<wxSlider>("BrightnessSlider")->Enable(true);
	GetWidgetByName<wxSlider>("ContrastSlider")->Enable(true);
	GetWidgetByName<wxChoice>("ModeChoice")->Enable(true);
	GetWidgetByName<wxButton>("AcceptButton")->Enable(true);
	GetWidgetByName<wxButton>("PlaneShiftButton")->Enable(true);

	GetMenuBar()->FindItem(XRCID("OpenModelMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("SaveMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(true);

	mainWindowState_ = MODEL_LOADED_STATE;
}

void MainWindow::OnTogglePlay(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "PlayButton", wxButton);
	
	if (animationIsOn_)
	{
		Time_keeper_stop(timeKeeper_);
		this->animationIsOn_ = false;
		button->SetLabel("play");
		OnAnimationSliderEvent(event); //HACK snap the slider to nearest frame time
	}
	else
	{
		Time_keeper_play(timeKeeper_,TIME_KEEPER_PLAY_FORWARD);
		Time_keeper_set_play_loop(timeKeeper_);
		//Time_keeper_set_play_every_frame(timeKeeper_);
		Time_keeper_set_play_skip_frames(timeKeeper_);
		this->animationIsOn_ = true;
		button->SetLabel("stop");
	}
	
	return;
}

void MainWindow::Terminate(wxCloseEvent& event)
{
	int answer = wxMessageBox("Quit program?", "Confirm",
	                            wxYES_NO, this);
	if (answer == wxYES)
	{
		exit(0); //without this, the funny temporary window appears
	}
}

void MainWindow::PopulateObjectList()
{
	objectList_->Clear();
	
	const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
	BOOST_FOREACH(std::string const& sliceName, sliceNames)
	{
		objectList_->Append(sliceName.c_str());
		bool visible = imageSet_->IsVisible(sliceName);
		/* default selection */
		if ( visible )
		{
			objectList_->Check((objectList_->GetCount()-1),1);
		}
		objectList_->SetSelection(wxNOT_FOUND);
	}
}

void MainWindow::OnObjectCheckListChecked(wxCommandEvent& event)
{
	int selection = event.GetInt();
//	objectList_->SetSelection(selection);
	wxString name = objectList_->GetString(selection);
	std::cout << "Check: " << name << std::endl;
	
	if(objectList_->IsChecked(selection))
	{
		SetImageVisibility(true, name.mb_str());
	}
	else
	{
		SetImageVisibility(false, name.mb_str());
	}
	
//	RefreshCmguiCanvas(); //Necessary?? - doesn't help with the problem where the canvas doesn't redraw
	m_pPanel->Refresh();
	this->Refresh();//test to see if this helps with the problem where 3d canvas doesnt update
}

void MainWindow::OnObjectCheckListSelected(wxCommandEvent& event)
{
	wxString name = objectList_->GetStringSelection();
	const ImagePlane& plane = imageSet_->GetImagePlane(name.mb_str());
	
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
	RefreshCmguiCanvas();
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
//	Cmiss_scene_viewer_view_all(sceneViewer_);
	return;
}

void MainWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	int value = slider->GetValue();
	
	int min = slider->GetMin();
	int max = slider->GetMax();
	double time =  (double)(value - min) / (double)(max - min);
	double prevFrameTime = heartModel_.MapToModelFrameTime(time);
	if ((time - prevFrameTime) < (0.5)/(heartModel_.GetNumberOfModelFrames()))
	{
		time = prevFrameTime;
	}
	else
	{
		time = prevFrameTime + (double)1/(heartModel_.GetNumberOfModelFrames());
	}
	slider->SetValue(static_cast<int>(time * (max - min)));
//	cout << __func__ << ": time = " << time << endl;;
//	imageSet_->SetTime(time);
	time = (time > 0.99) ? 0 : time;
	
	Time_keeper_request_new_time(timeKeeper_, time);
	
	RefreshCmguiCanvas(); // forces redraw while silder is manipulated
	return;
}

void MainWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSpeedControl", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	double speed = (double)(value - min) / (double)(max - min) * 2.0;
	Time_keeper_set_speed(timeKeeper_, speed);
	
	RefreshCmguiCanvas(); // forces redraw while silder is manipulated
	return;
}

void MainWindow::RefreshCmguiCanvas()
{
//	Scene_viewer_redraw(sceneViewer_);
	if (sceneViewer_) 
	{
		Scene_viewer_redraw_now(sceneViewer_);
	}
}

void MainWindow::SetTime(double time)
{
	//cout << "SetTime" <<endl;
	imageSet_->SetTime(time);
	
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	int min = slider->GetMin();
	int max = slider->GetMax();
	//cout << "min = " << min << " ,max = " << max <<endl; 
	slider->SetValue(static_cast<int>(static_cast<double>(max-min)*time) + min);
	
	int frameNumber = heartModel_.MapToModelFrameNumber(time);
	std::ostringstream frameNumberStringStream;
	frameNumberStringStream << "Frame Number: " << frameNumber;
	SetStatusText(wxT(frameNumberStringStream.str().c_str()), 0);
	return;
}

void MainWindow::OnToggleHideShowAll(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "HideShowAll", wxButton);
	if (hideAll_) //means the button says hide all rather than show all
	{
		hideAll_ = false;
		SetImageVisibility(false);
		button->SetLabel("Show All");
	}
	else
	{
		hideAll_ = true;	
		SetImageVisibility(true);
		button->SetLabel("Hide All");
	}
	
	for (int i=0;i<imageSet_->GetNumberOfSlices();i++)
	{
		objectList_->Check(i, hideAll_);
	}
	this->Refresh(); // work around for the refresh bug
}

void MainWindow::OnToggleHideShowOthers(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "HideShowOthers", wxButton);
	static bool showOthers = true;
	
	static std::vector<int> indicesOfOthers;
	if (showOthers) //means the button says hide all rather than show all
	{
		showOthers = false;
		// remember which ones were visible
		indicesOfOthers.clear();
		for (int i=0;i<imageSet_->GetNumberOfSlices();i++)
		{
			if (objectList_->IsChecked(i) && objectList_->GetSelection() != i)
			{
				indicesOfOthers.push_back(i);
				SetImageVisibility(false, i);
				objectList_->Check(i, false);
			}
		}
		button->SetLabel("Show Others");
	}
	else
	{
		showOthers = true;	

		std::vector<int>::iterator itr = indicesOfOthers.begin();
		std::vector<int>::const_iterator end = indicesOfOthers.end();
		for (; itr!=end ; ++itr)
		{
			SetImageVisibility(true, *itr);
			objectList_->Check(*itr, true);
		}
	
		button->SetLabel("Hide Others");
	}
	

	this->Refresh(); // work around for the refresh bug
}

void MainWindow::SetImageVisibility(bool visibility, int index)
{
	if (XRCCTRL(*this, "MII", wxCheckBox)->IsChecked())
		heartModel_.SetMIIVisibility(visibility, index);
	imageSet_->SetVisible(visibility, index);
}

void MainWindow::SetImageVisibility(bool visibility, const std::string& name)
{
	if (name.length()) //REVISE
	{
		const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();

		int i = find(sliceNames.begin(),sliceNames.end(), name) - sliceNames.begin();
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

void MainWindow::RenderIsoSurfaces()
{
	
	char str[256];
	
	const ImagePlane& plane_SA1 = imageSet_->GetImagePlane("SA1");
	const ImagePlane& plane_SA6 = imageSet_->GetImagePlane("SA6");
	
	const gtMatrix& m = heartModel_.GetLocalToGlobalTransformation();//CAPModelLVPS4X4::
//	cout << m << endl;

	gtMatrix mInv;
	inverseMatrix(m, mInv);
//	cout << mInv << endl;
	transposeMatrix(mInv); // gtMatrix is column Major and our matrix functions assume row major FIX!!
//	cout << mInv << endl;
	
	//Need to transform the image plane using the Local to global transformation matrix of the heart (ie to hearts local coord)
	Vector3D normalTransformed = m * plane_SA1.normal;
	
	Point3D pointTLCTransformed_SA1 = mInv * plane_SA1.tlc;
	double d_SA1 = DotProduct((pointTLCTransformed_SA1 - Point3D(0,0,0)), normalTransformed);
	
	Point3D pointTLCTransformed_SA6 = mInv * plane_SA6.tlc;
	double d_SA6 = DotProduct((pointTLCTransformed_SA6 - Point3D(0,0,0)), normalTransformed);
	
//	sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[%f %f %f]\";",
//				"ISO_SA1" ,
//				normalTransformed.x, normalTransformed.y, normalTransformed.z);
//	cout << str << endl;
//	Cmiss_context_execute_command(context_, str);
//	
////	sprintf((char*)str, "gfx modify g_element heart iso_surfaces iso_scalar slice_%s iso_values %f use_elements;"// select_on material white selected_material default_selected;"
////				,"ISO_SA1" ,d_SA1);
//	
//	sprintf((char*)str, "gfx modify g_element heart iso_surfaces iso_scalar slice_%s range_number_of_iso_values 25 first_iso_value %f last_iso_value %f use_elements scene print_temp;"// select_on invisible material default selected_material default_selected;"
//				,"ISO_SA1" ,d_SA1, d_SA6);
//	cout << str << endl;
//	Cmiss_context_execute_command(context_, str);
	
	
	sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[%f %f %f]\";",
				"ISO_SA6" ,
				normalTransformed.x, normalTransformed.y, normalTransformed.z);
//	cout << str << endl;
	Cmiss_context_execute_command(context_, str);
	
//	sprintf((char*)str, "gfx modify g_element heart iso_surfaces as XC iso_scalar slice_%s iso_values %f use_elements select_on material white selected_material default_selected render_shaded scene print_temp;;"
//				,"ISO_SA6" ,d_SA6);
	
	sprintf((char*)str, "gfx modify g_element heart lines scene print_temp");
//	cout << str << endl;
	Cmiss_context_execute_command(context_, str);
}

void MainWindow::InitialiseMII()
{
	// This method only makes sense when both the images and the model have been already loaded.
	const vector<string>& sliceNames = imageSet_->GetSliceNames();
	vector<string>::const_iterator itr = sliceNames.begin();
	for (;itr != sliceNames.end();++itr)
	{
		RenderMII(*itr);
	}
}

void MainWindow::UpdateMII() //FIX
{	
	const vector<string>& sliceNames = imageSet_->GetSliceNames();
	vector<string>::const_iterator itr = sliceNames.begin();
	for (int index = 0;itr != sliceNames.end();++itr, ++index)
	{
		const std::string& sliceName = *itr;
		char str[256];
		
		const ImagePlane& plane = imageSet_->GetImagePlane(sliceName);
		const gtMatrix& m = heartModel_.GetLocalToGlobalTransformation();
	
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
		heartModel_.UpdateMII(index, d);
	}
}

void MainWindow::RenderMII(const std::string& sliceName) //MOVE to CAPModelLVPS4X4
{	
	char str[256];
	
	const ImagePlane& plane = imageSet_->GetImagePlane(sliceName);
	const gtMatrix& m = heartModel_.GetLocalToGlobalTransformation();//CAPModelLVPS4X4::
//	cout << m << endl;

	gtMatrix mInv;
	inverseMatrix(m, mInv);
//	cout << mInv << endl;
	transposeMatrix(mInv); // gtMatrix is column Major and our matrix functions assume row major FIX!!
//	cout << mInv << endl;
	
	//Need to transform the image plane using the Local to global transformation matrix of the heart (ie to hearts local coord)
	Vector3D normalTransformed = m * plane.normal;
	sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[%f %f %f]\";",
				sliceName.c_str() ,
				normalTransformed.x, normalTransformed.y, normalTransformed.z);
//	cout << str << endl;
	Cmiss_context_execute_command(context_, str);
	
	Point3D pointTLCTransformed = mInv * plane.tlc;
	double d = DotProduct((pointTLCTransformed - Point3D(0,0,0)), normalTransformed);

	sprintf((char*)str, "gfx modify g_element heart iso_surfaces exterior iso_scalar slice_%s iso_values %f use_faces select_on material gold selected_material default_selected render_shaded line_width 2;"
				,sliceName.c_str() ,d);
//	cout << str << endl;
	Cmiss_context_execute_command(context_, str);
}

void MainWindow::OnMIICheckBox(wxCommandEvent& event)
{
	//heartModel_.SetMIIVisibility(event.IsChecked()); TEST
	for (int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
	{
		if (objectList_->IsChecked(i))
		{
			heartModel_.SetMIIVisibility(event.IsChecked(),i);
		}
	}
}

void MainWindow::OnWireframeCheckBox(wxCommandEvent& event)
{
	heartModel_.SetModelVisibility(event.IsChecked());
}

void MainWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
//	cout << "MainWindow::OnBrightnessSliderEvent" << endl;
	wxSlider* slider = XRCCTRL(*this, "BrightnessSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	float brightness = (float)(value - min) / (float)(max - min);
	imageSet_->SetBrightness(brightness);
	
	RefreshCmguiCanvas();
}

void MainWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
//	cout << "MainWindow::OnContrastSliderEvent" << endl;
	wxSlider* slider = XRCCTRL(*this, "ContrastSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	imageSet_->SetContrast(contrast);
	
	RefreshCmguiCanvas();
}

void MainWindow::OnAcceptButtonPressed(wxCommandEvent& event)
{
	std::cout << "Accept" << std::endl;

	const char* ModeStrings[] = {
			"Apex",
			"Base",
			"RV Inserts",
			"Baseline Points",
			"Guide Points"
	};//REVISE
	
	if (modeller_->OnAccept())
	{
		wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
		int selectionIndex = choice->GetSelection();
		int newIndex = std::min(selectionIndex + 1, static_cast<int>(choice->GetCount()));
		choice->Append(ModeStrings[newIndex]);
		choice->SetSelection(newIndex);
		//REVISE
		if (newIndex == 4) // guide point
		{
//			InitialiseVolumeGraph();
			UpdateMII();
			EnterModelLoadedState();
		}
	}
	
	RefreshCmguiCanvas();
}

void MainWindow::OnModellingModeChanged(wxCommandEvent& event)
{
	wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
	std::cout << "MODE = " << choice->GetStringSelection() << endl;

	int selectionIndex = choice->GetSelection();
	modeller_->ChangeMode((CAPModeller::ModellingMode) selectionIndex);//FIX type unsafe

	int numberOfItems = choice->GetCount();
	
	cout << __func__ << ": numberOfItems = " << numberOfItems << ", selectionIndex = " << selectionIndex << endl;
	
	for (int i = numberOfItems-1; i > selectionIndex; i--)
	{
		choice->Delete(i);
	}
	RefreshCmguiCanvas();
}

void MainWindow::OnAbout(wxCommandEvent& event)
{
	wxBoxSizer *topsizer;
	wxHtmlWindow *html;
	wxDialog dlg(this, wxID_ANY, wxString(_("About CAP Client")));
	
	topsizer = new wxBoxSizer(wxVERTICAL);
	
	html = new CAPHtmlWindow(&dlg, wxID_ANY, wxDefaultPosition, wxSize(600, 400));
	html -> SetBorders(0);
	html -> LoadPage(wxT("Data/HTML/AboutCAPClient.html"));
	//html -> SetSize(html -> GetInternalRepresentation() -> GetWidth(),
	//				html -> GetInternalRepresentation() -> GetHeight());
	
	topsizer -> Add(html, 1, wxALL, 10);
	
	wxButton *bu1 = new wxButton(&dlg, wxID_OK, _("OK"));
	bu1 -> SetDefault();
	
	topsizer -> Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);
	
	dlg.SetSizer(topsizer);
	topsizer -> Fit(&dlg);
	
	dlg.Center();
	dlg.ShowModal();
}

void MainWindow::OnOpenImages(wxCommandEvent& event)
{
	wxString currentWorkingDir = wxGetCwd();
//	wxString defaultPath = currentWorkingDir.Append("/Data");
	wxString defaultPath = currentWorkingDir.Append("/temp");
	
	const wxString& dirname = wxDirSelector("Choose the folder that contains the images", defaultPath);
	if ( !dirname.empty() )
	{
		cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
	}
	else
	{
		return;
	}
	
	cap::ImageBrowseWindow *frame = new ImageBrowseWindow(std::string(dirname.c_str()),
			cmguiManager_, *this);
	frame->Show(true);
}

void MainWindow::LoadImages(SlicesWithImages const& slices)
{
	std::cout << __func__ << " : slices.size() = " << slices.size() <<  '\n';
	if (slices.empty())
	{
		std::cout << "Empty image set.\n";
		return;
	}

	if(imageSet_)
	{
		delete imageSet_;
	}
	imageSet_ = new ImageSet(slices, cmguiManager_);
	Cmiss_scene_viewer_view_all(sceneViewer_);
	
	LoadHeartModel("MIDLIFE_01", CAP_DATA_DIR); //HACK FIXME
	
	this->PopulateObjectList(); // fill in slice check box list

	EnterImagesLoadedState();
}

void MainWindow::LoadHeartModel(std::string const& dirOnly, std::string const& prefix)
{
	heartModel_.ReadModelFromFiles(dirOnly, prefix);
	
	if (modeller_)
	{
		delete modeller_;
	}
	modeller_ = new CAPModeller(heartModel_); // initialise modeller and all the data points

	wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
	int numberOfItems = choice->GetCount();
	for (int i = numberOfItems-1; i > 0; i--)
	{
		// Remove all items except Apex
		choice->Delete(i);
	}
	choice->SetSelection(0);
	
	wxCheckBox* modelVisibilityCheckBox = XRCCTRL(*this, "Wireframe", wxCheckBox);
	heartModel_.SetModelVisibility(modelVisibilityCheckBox->IsChecked());
	
	InitialiseMII(); // This turns on all MII's
	
	// Update the visibility of each mii according to the ui status
	// ( = mii checkbox and the slice list)
	wxCheckBox* miiCheckBox = XRCCTRL(*this, "MII", wxCheckBox);
	if (miiCheckBox->IsChecked())
	{
		const int numberOfSlices = imageSet_->GetNumberOfSlices();
		for (int i = 0; i < numberOfSlices; i++)
		{
			cout << "slice num = " << i << ", isChecked = " << objectList_->IsChecked(i) << endl;
			if (!objectList_->IsChecked(i))
			{
				heartModel_.SetMIIVisibility(false,i);
			}
		}
	}
	else
	{
		heartModel_.SetMIIVisibility(false);
	}

	EnterModelLoadedState();
}

void MainWindow::OnOpenModel(wxCommandEvent& event)
{
	wxString currentWorkingDir = wxGetCwd();
	wxString defaultPath = currentWorkingDir.Append("/Data");
	wxString defaultFilename = "";
	wxString defaultExtension = "xml";
	wxString wildcard = "";
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector("Choose a file to open",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		CAPXMLFile xmlFile(filename.c_str());
		std::cout << "Start reading xml file\n";
		xmlFile.ReadFile();
		SlicesWithImages const& slicesWithImages = xmlFile.GetSlicesWithImages(cmguiManager_);
		// TODO clean up first
		LoadImages(slicesWithImages);

		std::vector<DataPoint> dataPoints = xmlFile.GetDataPoints(cmguiManager_);

		std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
		std::cout << "number of exnodeFilenames = " << exnodeFileNames.size();
		std::string const& exelemFileName = xmlFile.GetExelemFileName();


		//HACK FIXME
		std::string xmlFilename = filename.c_str();
		size_t positionOfLastSlash = xmlFilename.find_last_of("/\\");
		std::string modelFilePath = xmlFilename.substr(0, positionOfLastSlash);
		std::cout << "modelFilePath = " << modelFilePath << '\n';
		positionOfLastSlash = modelFilePath.find_last_of("/\\");
		string dirOnly = modelFilePath.substr(positionOfLastSlash+1); //FIX use wxFileName::SplitPath?
		string prefix = modelFilePath.substr(0, positionOfLastSlash+1);

		std::cout << __func__ << ", dir = " << dirOnly << ", prefix = " << prefix << '\n';
		// FIXME heartModel needs to be properly initialised and cleaned up
		heartModel_.SetFocalLengh(xmlFile.GetFocalLength());
		LoadHeartModel(dirOnly, prefix);
		modeller_->SetDataPoints(dataPoints);

		//HACK
		wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);

		//FIXME
		const char* ModeStrings[] = {
				"Apex",
				"Base",
				"RV Inserts",
				"Baseline Points",
				"Guide Points"
		};
		for (size_t i = 1; i < 5; i++)
		{
			choice->Append(ModeStrings[i]);
		}
		choice->SetSelection(CAPModeller::GUIDEPOINT);
		RefreshCmguiCanvas();
	}

//	const wxString& dirname = wxDirSelector("Choose the folder that contains the model", defaultPath);
//	if ( !dirname.empty() )
//	{
//		cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
//		string filename(dirname.c_str());
//		size_t positionOfLastSlash = filename.find_last_of("/\\");
//		std::cout << "positionOfLastSlash = " << positionOfLastSlash << std::endl;
//		string dirOnly = filename.substr(positionOfLastSlash+1); //FIX use wxFileName::SplitPath?
//		string prefix = filename.substr(0, positionOfLastSlash+1);
//		std::cout << __func__ << " - dirOnly = " << dirOnly << std::endl;
//
//		LoadHeartModel(dirOnly, prefix);
//	}
}

void MainWindow::OnSave(wxCommandEvent& event)
{
	wxString defaultPath = "./Data";
	wxString defaultFilename = "";
	wxString defaultExtension = "";
	wxString wildcard = "";
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector("Save file",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !dirname.empty() && mainWindowState_ == MODEL_LOADED_STATE)
	{
	    // work with the file
	    cout << __func__ << " - Model name: " << dirname.c_str() << endl;
	    heartModel_.WriteToFile(dirname.c_str());
	}

	CAPXMLFile xmlFile(dirname.c_str());
	SlicesWithImages const& slicesAndImages = imageSet_->GetSlicesWithImages();
	std::vector<DataPoint> const& dataPoints = modeller_->GetDataPoints();
	xmlFile.ContructCAPXMLFile(slicesAndImages, dataPoints, heartModel_);
	xmlFile.WriteFile(std::string(dirname.c_str()) + "/" + xmlFile.GetName() + ".xml");
}

void MainWindow::OnQuit(wxCommandEvent& event)
{
	Close();
}

void MainWindow::OnPlaneShiftButtonPressed(wxCommandEvent& event)
{
	static bool isPlaneShiftModeOn = false;
	
	wxButton* button = XRCCTRL(*this, "PlaneShiftButton", wxButton);
	assert(button);
	
	if (!isPlaneShiftModeOn)
	{
		isPlaneShiftModeOn = true;
		button->SetLabel("End Shifting");
		
		Cmiss_scene_viewer_remove_input_callback(sceneViewer_,
						input_callback, (void*)this);
		Cmiss_scene_viewer_add_input_callback(sceneViewer_,
						input_callback_image_shifting, (void*)this, 1/*add_first*/);
	}
	else
	{
		isPlaneShiftModeOn = false;
		button->SetLabel("Start Shifting");
		
		Cmiss_scene_viewer_remove_input_callback(sceneViewer_,
						input_callback_image_shifting, (void*)this);
		Cmiss_scene_viewer_add_input_callback(sceneViewer_,
						input_callback, (void*)this, 1/*add_first*/);
		
		imageSet_->WritePlaneInfoToFiles();
	}
	
	return;
}

void temp_fn(Cmiss_context_id context_)
{	
//	char str[256];
//	sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[%f %f %f]\";",
//				"ISO_SA6" ,
//				normalTransformed.x, normalTransformed.y, normalTransformed.z);
//	Cmiss_context_execute_command(context_, str);
//	
//	sprintf((char*)str, "gfx modify g_element heart iso_surfaces iso_scalar slice_%s iso_values %f use_elements select_on material white selected_material default_selected render_shaded scene print_temp;;"
//				,"ISO_SA6" ,d_SA6);
//	Cmiss_context_execute_command(context_, str);
}

std::pair<double,double> get_range(const ImageSet* imageSet_, const CAPModelLVPS4X4& heartModel_ )
{
	char str[256];
	
	const ImagePlane& plane_SA1 = imageSet_->GetImagePlane("SA1");
	const ImagePlane& plane_SA6 = imageSet_->GetImagePlane("SA6");
	
	const gtMatrix& m = heartModel_.GetLocalToGlobalTransformation();//CAPModelLVPS4X4::
//	cout << m << endl;

	gtMatrix mInv;
	inverseMatrix(m, mInv);
//	cout << mInv << endl;
	transposeMatrix(mInv); // gtMatrix is column Major and our matrix functions assume row major FIX!!
//	cout << mInv << endl;
	
	//Need to transform the image plane using the Local to global transformation matrix of the heart (ie to hearts local coord)
	Vector3D normalTransformed = m * plane_SA1.normal;
	
	Point3D pointTLCTransformed_SA1 = mInv * plane_SA1.tlc;
	double d_SA1 = DotProduct((pointTLCTransformed_SA1 - Point3D(0,0,0)), normalTransformed);
	
	Point3D pointTLCTransformed_SA6 = mInv * plane_SA6.tlc;
	double d_SA6 = DotProduct((pointTLCTransformed_SA6 - Point3D(0,0,0)), normalTransformed);
	
	return std::make_pair(d_SA1,d_SA6);
}
	

void MainWindow::OnExportModel(wxCommandEvent& event)
{
	cout << __func__ << "\n";
	
	char* file_name = "screen_dump.png";
	int force_onscreen_flag = 0;
	int width = 256;
	int height = 256;
	int antialias = 0;
	int transparency_layers = 0;
	
	Cmiss_scene_viewer_id scene_viewer = Cmiss_scene_viewer_create_wx(Cmiss_context_get_default_scene_viewer_package(context_),
			//panel,
			m_pPanel,
			CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
			CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
			/*minimum_colour_buffer_depth*/8,
			/*minimum_depth_buffer_depth*/8,
			/*minimum_accumulation_buffer_depth*/8);
	
	Cmiss_context_execute_command(context_, "gfx create scene print_temp manual_g_element");
	Cmiss_scene_viewer_set_scene_by_name(scene_viewer, "print_temp");
	struct Scene *scene = Scene_viewer_get_scene(scene_viewer);
	
	Cmiss_context_execute_command(context_, "gfx draw as heart group heart scene print_temp");
	// The above doesn't copy the transformation so it has to be done manually
	char* scene_object_name = "heart";
	
	RenderIsoSurfaces();
		
	double centre_x, centre_y, centre_z, size_x, size_y, size_z;
	if (!Scene_get_graphics_range(scene, &centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z))
	{
		cout << "Error: Scene_get_graphics_range before transformation\n";
	}
	cout << "range before: " << centre_x  << ", " << centre_y << ", " 
			<< centre_z << ", " << size_x << ", " << size_y <<", "<<  size_z << endl;
	
	struct Scene_object * modelSceneObject=Scene_get_Scene_object_by_name(scene, scene_object_name);
	if (modelSceneObject)
	{
		const gtMatrix& patientToGlobalTransform = heartModel_.GetLocalToGlobalTransformation();
		Scene_object_set_transformation(modelSceneObject, const_cast<gtMatrix*>(&patientToGlobalTransform));
	}
	else
	{
		display_message(ERROR_MESSAGE,"No object named '%s' in scene",scene_object_name);
	}
	
//	RenderIsoSurfaces();
	
	const ImagePlane& plane = imageSet_->GetImagePlane("SA1");
	
	// compute the center of the image plane, eye(camera) position and the up vector
	Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
	Point3D eye = planeCenter - (plane.normal * 500); // this seems to determine the near clip plane
	Vector3D up(plane.yside);
	up.Normalise();
	
	//Hack :: perturb direction vector a little
//	eye.x *= 1.01; //HACK 1.001 makes the iso lines partially visible
	
	
	if (!Cmiss_scene_viewer_set_lookat_parameters_non_skew(
			scene_viewer, eye.x, eye.y, eye.z,
			planeCenter.x, planeCenter.y, planeCenter.z,
			up.x, up.y, up.z
			))
	{
		//Error;
	}
	
//	Cmiss_scene_viewer_set_perturb_lines(scene_viewer, 1 );
	
	Cmiss_scene_viewer_redraw_now(scene_viewer);
	
	if (!Scene_get_graphics_range(scene, &centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z))
	{
		cout << "Error: Scene_get_graphics_range after transformation\n";
	}
	cout << "range after : " << centre_x  << ", " << centre_y << ", "
			<< centre_z << ", " << size_x << ", " << size_y <<", "<<  size_z << endl;
	
	std::pair<double, double> range = get_range(imageSet_, heartModel_);
	double min = std::min(range.first, range.second);
	double max = std::max(range.first, range.second);
	int i = 1;
//	for (double d = min; d<max ; d=d+1.0, i++)
//	{	
////		GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(modelSceneObject);
////		if (!gt_element_group)
////		{
////			cout << "Can't find gt_element_group" << endl;
////			assert(gt_element_group);
////		}
////		
//////		int num_settings = GT_element_group_get_number_of_settings(gt_element_group);
//////		cout << "num_settings = " << num_settings << "\n"; 
////		GT_element_settings* settings = get_settings_at_position_in_GT_element_group(gt_element_group,1);
////		if (!settings)
////		{
////			cout << "Can't find settings by position" << endl;
////			assert(settings);
////		}
////		
////		Cmiss_region_id region = GT_element_group_get_Cmiss_region(gt_element_group);
////		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
////		Computed_field* iso_scalar_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("slice_ISO_SA6",cfm);
////		if (!iso_scalar_field)
////		{
////			cout << "Can't find iso_scalar_field\n";
////		}
////		if (!GT_element_settings_get_iso_surface_parameters())
////		if (!GT_element_settings_set_iso_surface_parameters(settings, iso_scalar_field, 1, &d, 0, 0, 0))
////		{
////			cout << "Error setting iso surface params\n";
////		}
////
////		GT_element_group_modify(gt_element_group, gt_element_group);
////		Cmiss_scene_viewer_redraw_now(scene_viewer);
//		
//		char str[256];
//		sprintf((char*)str, "gfx modify g_element heart iso_surfaces as XC iso_scalar slice_%s iso_values %f use_elements select_on material white selected_material default_selected render_shaded scene print_temp;;"
//					,"ISO_SA6" ,d);
//	//	cout << str << endl;
//		Cmiss_context_execute_command(context_, str);
//		
//		std::stringstream filenameStream;
//		filenameStream << "binary_" << i << ".png" ;
//		Cmiss_scene_viewer_write_image_to_file(scene_viewer, filenameStream.str().c_str(), force_onscreen_flag , width,
//			height, antialias, transparency_layers);
//	}
	
//	Cmiss_scene_viewer_destroy(&scene_viewer);
//	Cmiss_context_execute_command(context_, "gfx destroy scene print_temp");
}

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_BUTTON(XRCID("PlayButton"),MainWindow::OnTogglePlay) // play button
	EVT_SLIDER(XRCID("AnimationSlider"),MainWindow::OnAnimationSliderEvent) // animation slider
	EVT_SLIDER(XRCID("AnimationSpeedControl"),MainWindow::OnAnimationSpeedControlEvent)
	EVT_CHECKLISTBOX(XRCID("SliceList"), MainWindow::OnObjectCheckListChecked)
	EVT_BUTTON(XRCID("HideShowAll"),MainWindow::OnToggleHideShowAll) // hide all button
	EVT_BUTTON(XRCID("HideShowOthers"),MainWindow::OnToggleHideShowOthers) // hide others button
	EVT_CHECKBOX(XRCID("MII"),MainWindow::OnMIICheckBox)
	EVT_CHECKBOX(XRCID("Wireframe"),MainWindow::OnWireframeCheckBox)
	EVT_LISTBOX(XRCID("SliceList"), MainWindow::OnObjectCheckListSelected)
	EVT_SLIDER(XRCID("BrightnessSlider"),MainWindow::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),MainWindow::OnContrastSliderEvent)
	EVT_BUTTON(XRCID("AcceptButton"),MainWindow::OnAcceptButtonPressed)
	EVT_CHOICE(XRCID("ModeChoice"),MainWindow::OnModellingModeChanged)
	EVT_CLOSE(MainWindow::Terminate)
	EVT_MENU(XRCID("QuitMenuItem"),  MainWindow::OnQuit)
	EVT_MENU(XRCID("AboutMenuItem"), MainWindow::OnAbout)
	EVT_MENU(XRCID("OpenImagesMenuItem"), MainWindow::OnOpenImages)
	EVT_MENU(XRCID("OpenModelMenuItem"), MainWindow::OnOpenModel)
	EVT_MENU(XRCID("SaveMenuItem"), MainWindow::OnSave)
	EVT_MENU(XRCID("ExportMenuItem"), MainWindow::OnExportModel)
	EVT_BUTTON(XRCID("PlaneShiftButton"), MainWindow::OnPlaneShiftButtonPressed)
END_EVENT_TABLE()

} // end namespace cap
