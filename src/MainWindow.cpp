#include "wx/xrc/xmlres.h"
#include <wx/dir.h>

//#include "wx/splitter.h"
#include <wx/aboutdlg.h>

#include "Config.h"
#include "SliceInfo.h"
#include "MainWindow.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
#include "ImageSetBuilder.h"
#include "ImageSet.h"
#include "CmguiExtensions.h"
#include "ImageBrowser.h"
#include "ImageBrowseWindow.h"
#include "UserCommentDialog.h"
#include "CAPHtmlWindow.h"
#include "CAPXMLFile.h"
#include "CAPAnnotationFile.h"
#include "CAPModeller.h"
#include "CAPXMLFileHandler.h"

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
#include "finite_element/export_finite_element.h"
}

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
	
	// We have to stop the animation when the user clicks on the 3D panel.
	// Since dragging a point while cine is playing can cause a problem
	// But Is this the best place put this code?
	frame->StopCine();
	
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
				cout << __func__ << ": ERROR\n";
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
		cout << "Mouse released" << endl;
	}
	
	return 0; // returning false means don't call the other input handlers;
}

static int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
	cout << "Time_call_back time = " << current_time << endl;
	
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
	timeNotifier_(0),
	heartModelPtr_(0),
	modeller_(0),
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
	this->Centre();
	
	EnterInitState();
}

MainWindow::~MainWindow()
{
	cout << __func__ << endl;
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
	if (!modeller_)
	{
		return;//FIXME
	}
	modeller_->SmoothAlongTime();
	RefreshCmguiCanvas();
	
	assert(heartModelPtr_);
	cout << "ED Volume(EPI) = " << heartModelPtr_->ComputeVolume(EPICARDIUM, 0) << endl;
	cout << "ED Volume(ENDO) = " << heartModelPtr_->ComputeVolume(ENDOCARDIUM, 0) << endl;
}

template <typename Widget>
Widget* MainWindow::GetWidgetByName(std::string const& name)
{
	Widget* widget = XRCCTRL(*this, name.c_str(), Widget);
	return  widget;
}

void MainWindow::EnterInitState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
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
	capXMLFilePtr_.reset(0);
	cardiacAnnotationPtr_.reset(0);
	if(imageSet_)
	{
		delete imageSet_;
		imageSet_ = 0;
	}
	heartModelPtr_.reset(0);
	objectList_->Clear();

	mainWindowState_ = INIT_STATE;
}

void MainWindow::EnterImagesLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	GetWidgetByName<wxSlider>("AnimationSlider")->Enable(true);
	GetWidgetByName<wxSlider>("AnimationSlider")->SetValue(0);
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
//	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(false);
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(true);
	
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

void MainWindow::EnterModelLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
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
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(true);//FIXME

	StopCine();
	assert(heartModelPtr_);
	heartModelPtr_->SetModelVisibility(true);
	GetWidgetByName<wxCheckBox>("Wireframe")->SetValue(true);
	
	mainWindowState_ = MODEL_LOADED_STATE;
}

void MainWindow::PlayCine()
{
	wxButton* button = XRCCTRL(*this, "PlayButton", wxButton);
	Time_keeper_play(timeKeeper_,TIME_KEEPER_PLAY_FORWARD);
	Time_keeper_set_play_loop(timeKeeper_);
	//Time_keeper_set_play_every_frame(timeKeeper_);
	Time_keeper_set_play_skip_frames(timeKeeper_);
	this->animationIsOn_ = true;
	button->SetLabel("stop");
}

void MainWindow::StopCine()
{
	wxButton* button = XRCCTRL(*this, "PlayButton", wxButton);
	Time_keeper_stop(timeKeeper_);
	this->animationIsOn_ = false;
	button->SetLabel("play");
	wxCommandEvent event;
	OnAnimationSliderEvent(event); //HACK snap the slider to nearest frame time
}

void MainWindow::OnTogglePlay(wxCommandEvent& event)
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

void MainWindow::Terminate(wxCloseEvent& event)
{
	int answer = wxMessageBox("Quit program?", "Confirm",
	                            wxYES_NO, this);
	if (answer == wxYES)
	{
//		Destroy();
//		exit(0); //without this, the funny temporary window appears
//		Cmiss_context_execute_command(context_, "QUIT");
		wxExit();
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
	if (!heartModelPtr_) //FIXME
	{
		return;
	}
	
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	int value = slider->GetValue();
	
	int min = slider->GetMin();
	int max = slider->GetMax();
	double time =  (double)(value - min) / (double)(max - min);
	assert(heartModelPtr_);
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
	
	assert(heartModelPtr_);
	int frameNumber = heartModelPtr_->MapToModelFrameNumber(time);
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
	assert(heartModelPtr_);
	if (XRCCTRL(*this, "MII", wxCheckBox)->IsChecked())
		heartModelPtr_->SetMIIVisibility(visibility, index);
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

void MainWindow::InitialiseMII()
{
	// This method only makes sense when both the images and the model have been already loaded.
	const vector<string>& sliceNames = imageSet_->GetSliceNames();
	vector<string>::const_iterator itr = sliceNames.begin();
	for (;itr != sliceNames.end();++itr)
	{
		string const& sliceName = *itr;
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

void MainWindow::UpdateMII() //FIX
{	
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

void MainWindow::OnMIICheckBox(wxCommandEvent& event)
{
	assert(heartModelPtr_);
	for (int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
	{
		if (objectList_->IsChecked(i))
		{
			heartModelPtr_->SetMIIVisibility(event.IsChecked(),i);
		}
	}
}

void MainWindow::OnWireframeCheckBox(wxCommandEvent& event)
{
	assert(heartModelPtr_);
	heartModelPtr_->SetModelVisibility(event.IsChecked());
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

	choice->SetSelection(selectionIndex);
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
	wxString defaultPath = wxGetCwd();;
	
	const wxString& dirname = wxDirSelector("Choose the folder that contains the images", defaultPath);
	if ( !dirname.empty() )
	{
		cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
	}
	else
	{
		return;
	}
	
//	ImageBrowseWindow *frame = new ImageBrowseWindow(std::string(dirname.c_str()), cmguiManager_, *this);
//	frame->Show(true);
	ImageBrowser<ImageBrowseWindow, CmguiManager>* ib = 
			ImageBrowser<ImageBrowseWindow, CmguiManager>::CreateImageBrowser(std::string(dirname.c_str()), cmguiManager_, *this);
		
}

void MainWindow::LoadImages(SlicesWithImages const& slices)
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
	
	this->PopulateObjectList(); // fill in slice check box list
}

void MainWindow::LoadImagesFromXMLFile(SlicesWithImages const& slices)
{
	LoadImages(slices);
	EnterImagesLoadedState();
}

void MainWindow::LoadImagesFromImageBrowseWindow(SlicesWithImages const& slices, CardiacAnnotation const& anno)
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
							wxCommandEvent event;
							OnAcceptButtonPressed(event);
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
							wxCommandEvent event;
							OnAcceptButtonPressed(event);
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

namespace
{

	struct ComparatorForNumFrames
	{
		bool operator() (SliceInfo const& a, SliceInfo const& b)
		{
			return (a.GetDICOMImages().size() < b.GetDICOMImages().size());
		}
	};

} // unnamed namespace

void MainWindow::InitializeModelTemplate(SlicesWithImages const& slices)
{	
	SlicesWithImages::const_iterator 
		itrToMinNumberOfFrames = std::min_element(slices.begin(), slices.end(),
													ComparatorForNumFrames());
	int minNumberOfFrames = itrToMinNumberOfFrames->GetDICOMImages().size();
	
	heartModelPtr_.reset(new CAPModelLVPS4X4("heart", cmguiManager_.GetCmissContext()));
	assert(heartModelPtr_);
	heartModelPtr_->SetNumberOfModelFrames(minNumberOfFrames);
	LoadTemplateHeartModel("heart", std::string(CAP_DATA_DIR) + "templates/" ); //HACK FIXME
	XRCCTRL(*this, "MII", wxCheckBox)->SetValue(false);
	XRCCTRL(*this, "Wireframe", wxCheckBox)->SetValue(false);
	heartModelPtr_->SetMIIVisibility(false);
	heartModelPtr_->SetModelVisibility(false);
}

void MainWindow::CreateModeller()
{
	if (modeller_)
	{
		delete modeller_;
	}
	assert(heartModelPtr_);
	modeller_ = new CAPModeller(*heartModelPtr_); // initialise modeller and all the data points
}

void MainWindow::ResetModeChoice()
{
	// Resets the mode choice UI widget to Apex mode
	wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
	int numberOfItems = choice->GetCount();
	for (int i = numberOfItems-1; i > 0; i--)
	{
		// Remove all items except Apex
		choice->Delete(i);
	}
	choice->SetSelection(0);
}

void MainWindow::UpdateModelVisibilityAccordingToUI()
{
	wxCheckBox* modelVisibilityCheckBox = XRCCTRL(*this, "Wireframe", wxCheckBox);
	assert(heartModelPtr_);
	heartModelPtr_->SetModelVisibility(modelVisibilityCheckBox->IsChecked());
}

void MainWindow::UpdateMIIVisibilityAccordingToUI()
{
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
				heartModelPtr_->SetMIIVisibility(false,i);
			}
		}
	}
	else
	{
		heartModelPtr_->SetMIIVisibility(false);
	}
}

void MainWindow::UpdateStatesAfterLoadingModel()
{
	CreateModeller();
	ResetModeChoice();
	
	UpdateModelVisibilityAccordingToUI();
	
	InitialiseMII(); // This turns on all MII's
	UpdateMIIVisibilityAccordingToUI();
}

void MainWindow::LoadTemplateHeartModel(std::string const& dirOnly, std::string const& prefix)
{
	// This function is used to load the unfitted generic heart model.
	// Currently this is necessary. It might be possible to eliminate the use of generic model
	// in the future by creating the related cmgui nodes, fields and element completely thru cmgui api
	assert(heartModelPtr_);
	heartModelPtr_->ReadModelFromFiles(dirOnly, prefix);
	UpdateStatesAfterLoadingModel();
}

void MainWindow::LoadHeartModel(std::string const& path, std::vector<std::string> const& modelFilenames)
{
	assert(heartModelPtr_);
	heartModelPtr_->ReadModelFromFiles(path, modelFilenames);
	UpdateStatesAfterLoadingModel();
	EnterModelLoadedState();
}

void MainWindow::OpenModel(std::string const& filename)
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
		
		wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);

		for (size_t i = 1; i < 5; i++)
		{
			choice->Append(ModeStrings[i]);
		}
		choice->SetSelection(CAPModeller::GUIDEPOINT);
		RefreshCmguiCanvas();
		
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

	//HACK
	wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);

	for (size_t i = 1; i < 5; i++)
	{
		choice->Append(ModeStrings[i]);
	}
	choice->SetSelection(CAPModeller::GUIDEPOINT);
	RefreshCmguiCanvas();
}

void MainWindow::OnOpenModel(wxCommandEvent& event)
{
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = "";
	wxString defaultExtension = "xml";
	wxString wildcard = "";
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector("Choose a model file to open",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		OpenModel(filename.c_str());
//		cardiacAnnotationPtr_.reset(0);
////		CAPXMLFile xmlFile(filename.c_str());
//		capXMLFilePtr_.reset(new CAPXMLFile(filename.c_str()));
//		CAPXMLFile& xmlFile(*capXMLFilePtr_);
//		std::cout << "Start reading xml file\n";
//		xmlFile.ReadFile();
//		
//		CAPXMLFileHandler xmlFileHandler(xmlFile);
//		SlicesWithImages const& slicesWithImages = xmlFileHandler.GetSlicesWithImages(cmguiManager_);
//		if (slicesWithImages.empty())
//		{
//			std::cout << "Can't locate image files\n";
//			return;
//		}
//
//		// TODO clean up first
//		LoadImagesFromXMLFile(slicesWithImages);
//
//		std::vector<DataPoint> dataPoints = xmlFileHandler.GetDataPoints(cmguiManager_);
//
//		std::vector<std::string> exnodeFileNames = xmlFile.GetExnodeFileNames();
//		std::cout << "number of exnodeFilenames = " << exnodeFileNames.size() << '\n';
//		if (exnodeFileNames.empty())
//		{
//			// This means no output element is defined
//			InitializeModelTemplate(slicesWithImages);
//			modeller_->SetDataPoints(dataPoints);
//			// FIXME memory is prematurely released when ok button is pressed from the following window
//			// Suppress this feature for now
////			ImageBrowseWindow *frame = new ImageBrowseWindow(slicesWithImages, cmguiManager_, *this);
////			frame->Show(true);
//			
//			//HACK : uncommenting the following will enable models to be constructed from model files with
//			// only the input element defined.
////			EnterModelLoadedState();
////			
////			wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
////
////			for (size_t i = 1; i < 5; i++)
////			{
////				choice->Append(ModeStrings[i]);
////			}
////			choice->SetSelection(CAPModeller::GUIDEPOINT);
////			RefreshCmguiCanvas();
//			
//			return;
//		}
//		
//		std::string const& exelemFileName = xmlFile.GetExelemFileName();
//
//		//HACK FIXME
//		std::string xmlFilename = filename.c_str();
//		size_t positionOfLastSlash = xmlFilename.find_last_of("/\\");
//		std::string modelFilePath = xmlFilename.substr(0, positionOfLastSlash);
//		std::cout << "modelFilePath = " << modelFilePath << '\n';
//
//		heartModelPtr_.reset(new CAPModelLVPS4X4("heart", cmguiManager_.GetCmissContext()));
//		assert(heartModelPtr_);
//		heartModelPtr_->SetFocalLengh(xmlFile.GetFocalLength());
//		int numberOfModelFrames = exnodeFileNames.size();
//		heartModelPtr_->SetNumberOfModelFrames(numberOfModelFrames);
//		LoadHeartModel(modelFilePath, exnodeFileNames);
//		gtMatrix m;
//		xmlFile.GetTransformationMatrix(m);
//		heartModelPtr_->SetLocalToGlobalTransformation(m);
//		modeller_->SetDataPoints(dataPoints);
//		UpdateMII();
//
//		//HACK
//		wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
//
//		for (size_t i = 1; i < 5; i++)
//		{
//			choice->Append(ModeStrings[i]);
//		}
//		choice->SetSelection(CAPModeller::GUIDEPOINT);
//		RefreshCmguiCanvas();
	}
}

void MainWindow::OnOpenAnnotation(wxCommandEvent& event)
{
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = "";
	wxString defaultExtension = "xml";
	wxString wildcard = "";
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector("Choose an annotation file to open",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		CAPAnnotationFile annotationFile(filename.c_str());
		std::cout << "Start reading xml file\n";
		annotationFile.ReadFile();
		
		// Create DICOMTable (filename -> DICOMImage map)
		// Create TextureTable (filename -> Cmiss_texture* map)
		
		// check if a valid file 
		if (annotationFile.GetCardiacAnnotation().imageAnnotations.empty())
		{
			cout << "Invalid Annotation File\n";
			return;
		}
		
		const wxString& dirname = wxDirSelector("Choose the folder that contains the images", defaultPath);
		if ( !dirname.empty() )
		{
			std::cout << __func__ << " - Dir name: " << dirname.c_str() << '\n';
		}
		else
		{
			// User cancelled the operation
			cout << "Invalid directory\n";
			return;
		}
		
//		EnterInitState();
//		cardiacAnnotationPtr_.reset(new CardiacAnnotation(annotationFile.GetCardiacAnnotation()));
		
//		ImageBrowseWindow *frame = new ImageBrowseWindow(std::string(dirname.c_str()), cmguiManager_, *this);
//		frame->Show(true);
		ImageBrowser<ImageBrowseWindow, CmguiManager>* ib = 
				ImageBrowser<ImageBrowseWindow, CmguiManager>::CreateImageBrowser(std::string(dirname.c_str()), cmguiManager_, *this);
		
		// Set annotations to the images in the ImageBrowseWindow.
		ib->SetAnnotation(annotationFile.GetCardiacAnnotation());
	}
}

void MainWindow::SaveModel(std::string const& dirname, std::string const userComment)
{
	if (!wxMkdir(dirname.c_str()))
	{
		std::cout << __func__ << " - Error: can't create directory: " << dirname << std::endl;
		return;
	}
	
	// Need to write the model files first 
	// FIXME : this is brittle code. shoule be less dependent on the order of execution
	if (mainWindowState_ == MODEL_LOADED_STATE)
	{
	    cout << __func__ << " - Model name: " << dirname.c_str() << endl;
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

void MainWindow::OnSave(wxCommandEvent& event)
{
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = "";
	wxString defaultExtension = "";
	wxString wildcard = "";
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector("Save file",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if (dirname.empty())
	{
		return;
	}
	
	std::string const& userComment = PromptForUserComment();
	std::cout << "User comment = " << userComment << "\n";
	if (userComment.empty())
	{
		// save has been canceled 
		return;
	}

	SaveModel(dirname.c_str(), userComment);
	
//	if (!wxMkdir(dirname.c_str()))
//	{
//		std::cout << __func__ << " - Error: can't create directory: " << dirname << std::endl;
//		return;
//	}
//	
//	// Need to write the model files first 
//	// FIXME : this is brittle code. shoule be less dependent on the order of execution
//	if (mainWindowState_ == MODEL_LOADED_STATE)
//	{
//	    cout << __func__ << " - Model name: " << dirname.c_str() << endl;
//	    assert(heartModelPtr_);
//	    heartModelPtr_->WriteToFile(dirname.c_str());
//	}
//	
//	if (!capXMLFilePtr_)
//	{
//		capXMLFilePtr_.reset(new CAPXMLFile(dirname.c_str()));
//	}
//	CAPXMLFile& xmlFile(*capXMLFilePtr_);
//	
//	SlicesWithImages const& slicesAndImages = imageSet_->GetSlicesWithImages();
//	std::vector<DataPoint> const& dataPoints = modeller_->GetDataPoints();
//	CAPXMLFileHandler xmlFileHandler(xmlFile);
//	xmlFileHandler.ContructCAPXMLFile(slicesAndImages, dataPoints, *heartModelPtr_);
//	xmlFileHandler.AddProvenanceDetail(userComment);
//	
//	std::string dirnameStl(dirname.c_str());
//	size_t positionOfLastSlash = dirnameStl.find_last_of("/\\");
//	std::string modelName = dirnameStl.substr(positionOfLastSlash + 1);
//	xmlFile.SetName(modelName);
//	std::string xmlFilename = std::string(dirname.c_str()) + "/" + modelName + ".xml";
//	std::cout << "xmlFilename = " << xmlFilename << '\n';
//	
//	if (cardiacAnnotationPtr_)
//	{
//		xmlFile.SetCardiacAnnotation(*cardiacAnnotationPtr_);
//	}
//	
//	xmlFile.WriteFile(xmlFilename);
}

std::string MainWindow::PromptForUserComment()
{
	UserCommentDialog dialog(this);
	dialog.Center();
	std::string comment;
	
	while (true)
	{
		if (dialog.ShowModal() == wxID_OK)
		{
			comment = dialog.GetComment();
			if (!comment.empty())
			{
				break;
			}
			
			int answer = wxMessageBox("Please enter user comment", "Empty Comment",
											wxOK, this);
		}
		else
		{
			int answer = wxMessageBox("Cancel Save?", "Confirm",
											wxYES_NO, this);
			if (answer == wxYES)
			{
				break;
			}
		}
	}
	
	return comment;
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
		
		imageSet_->SetShiftedImagePosition();
	}
	
	return;
}

void MainWindow::OnExportModel(wxCommandEvent& event)
{
	cout << __func__ << "\n";
	
//	SlicesWithImages const& slicesWithImages = imageSet_->GetSlicesWithImages();
//	ImageBrowseWindow *frame = new ImageBrowseWindow(slicesWithImages, cmguiManager_, *this);
//	frame->Show(true);
	
	return;
	//// test

	heartModelPtr_.reset(0);
//	Cmiss_region_id root = Cmiss_context_get_default_region(context_);
//	Cmiss_region_id region = Cmiss_region_find_subregion_at_path(root, "/heart/");
//	Cmiss_region_id copy = region;
//	Cmiss_region_remove_child(root, region);
////	std::cout << "Use_count = " <<
//	Cmiss_region_destroy(&region);
//	Cmiss_region_destroy(&copy);

	///////

	return;
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
	EVT_MENU(XRCID("OpenAnnotationMenuItem"), MainWindow::OnOpenAnnotation)
	EVT_MENU(XRCID("SaveMenuItem"), MainWindow::OnSave)
	EVT_MENU(XRCID("ExportMenuItem"), MainWindow::OnExportModel)
	EVT_BUTTON(XRCID("PlaneShiftButton"), MainWindow::OnPlaneShiftButtonPressed)
END_EVENT_TABLE()

} // end namespace cap
