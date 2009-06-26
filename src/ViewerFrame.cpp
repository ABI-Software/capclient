#include "wx/xrc/xmlres.h"
#include "wx/splitter.h"
#include <wx/aboutdlg.h>

#include "Config.h"
#include "ViewerFrame.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
#include "ImageSet.h"
#include "CmguiExtensions.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

extern "C"
{
#include "api/cmiss_time_keeper.h"
#include "api/cmiss_time.h"
#include "command/cmiss.h"
#include "graphics/scene.h"	
#include "graphics/scene_viewer.h"
#include "three_d_drawing/graphics_buffer.h"
#include "general/debug.h"
}

using namespace std;
		
static int input_callback(struct Scene_viewer *scene_viewer, 
		struct Graphics_buffer_input *input, void *viewer_frame_void)
{
//	cout << "input_callback() : input_type = " << input->type << endl;
	
	if (!(input->input_modifier & GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT))
	{
		return 1;
	}
	
	static Cmiss_node_id selectedNode = NULL; // Thread unsafe
	ViewerFrame* frame = static_cast<ViewerFrame*>(viewer_frame_void);
	
	double x = (double)(input->position_x);
	double y = (double)(input->position_y);
	float time = frame->GetCurrentTime(); // TODO REVISE 
	if (input->type == GRAPHICS_BUFFER_BUTTON_PRESS)
	{
		// Select node or create one
		cout << "Mouse clicked, time = " << time << endl;
		cout << "Mouse button number = " << input->button_number << endl;
		
		Point3D coords;
		selectedNode = Cmiss_select_node_from_screen_coords(x, y, time, coords);
					
		if (input->button_number == wxMOUSE_BTN_LEFT )
		{	
			if (!selectedNode) //REVISE
			{
				if (selectedNode = Cmiss_create_or_select_node_from_screen_coords(x, y, time, coords)) 
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
		cout << "Mouse Drag node = " << Cmiss_node_get_identifier(selectedNode) << endl;
		Cmiss_move_node_to_screen_coords(selectedNode, x, y, time, coords);
		
		cout << "Move coord = " << coords << endl;
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

static int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
//	cout << "Time_call_back time = " << current_time << endl;
	
	ViewerFrame* frame = static_cast<ViewerFrame*>(user_data);
	frame->SetTime(current_time);
	
	frame->RefreshCmguiCanvas(); // this forces refresh even when UI is being manipulated by user

	return 0;
}

ViewerFrame::ViewerFrame(Cmiss_command_data* command_data_)
: 
	command_data(command_data_),
	animationIsOn_(false),
	hideAll_(true),
	timeKeeper_(Cmiss_command_data_get_default_time_keeper(command_data_)),
	heartModel_("heart"),
	modeller_(new CAPModeller(heartModel_))
{
	// Load layout from .xrc file
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ViewerFrame"));
	
	// HACK to make sure the layout is properly applied (for Mac)
	this->Show(true);
	wxSplitterWindow* win = XRCCTRL(*this, "window_1", wxSplitterWindow);
	assert(win);
	
	win->SetSashPosition(800, true);
//	this->SetSize(1023,767);
//	this->SetSize(1024,768);
	
	m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	m_pPanel->GetContainingSizer()->SetMinSize(800, 800);
	m_pPanel->GetContainingSizer()->SetDimension(-1, -1, 800, 800);
	this->GetSizer()->SetSizeHints(this);
	this->Fit();
	
	// GUI initialization

	// Initialize check box list of scene objects (image slices)
	objectList_ = XRCCTRL(*this, "SliceList", wxCheckListBox);
	objectList_->SetSelection(wxNOT_FOUND);
	objectList_->Clear();
	
	// Initialize animation speed control
	wxSlider* animantionSpeedControl = XRCCTRL(*this, "AnimationSpeedControl", wxSlider);
	int min = animantionSpeedControl->GetMin();
	int max = animantionSpeedControl->GetMax();
	animantionSpeedControl->SetValue((max - min)/2);
	
	this->Layout();
	this->Fit();
	
	this->Show(false);
	
//	heartModel_.ReadModelFromFiles("test");	
	
#define TEXTURE_ANIMATION
#ifdef TEXTURE_ANIMATION
	LoadImages();
	
	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().createSceneViewer(m_pPanel);
	Cmiss_scene_viewer_view_all(sceneViewer);
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1 );
	
#define TIME_OBJECT_CALLBACK_TEST
#ifdef TIME_OBJECT_CALLBACK_TEST
	Cmiss_time_notifier_id time_notifier = Cmiss_time_notifier_create_regular(28, 0); // FIX magic number
	Cmiss_time_notifier_add_callback(time_notifier, time_callback, (void*)this);
	Cmiss_time_keeper_add_time_notifier(timeKeeper_, time_notifier);
#endif		
#endif //TEXTURE_ANIMATION
	
	//Load model
	heartModel_.ReadModelFromFiles("MIDLIFE_01", CAP_DATA_DIR);	
	//heartModel_.SetRenderMode(CAPModelLVPS4X4::WIREFRAME);//this resets timer frequency for model!! if called after its been cleared!!??

	InitialiseMII();
	
	heartModel_.SetModelVisibility(false);
	heartModel_.SetMIIVisibility(false);
	
	Time_keeper_set_minimum(timeKeeper_, 0);
	Time_keeper_set_maximum(timeKeeper_, 1);
	
	this->Show(true);

	// initialize brightness and contrast sliders
	wxSlider* brightnessSlider = XRCCTRL(*this, "BrightnessSlider", wxSlider);
	min = brightnessSlider->GetMin();
	max = brightnessSlider->GetMax();
	brightnessSlider->SetValue((max - min)/2);
	imageSet_->SetBrightness(0.5);
	
	wxSlider* contrastSlider = XRCCTRL(*this, "ContrastSlider", wxSlider);
	min = contrastSlider->GetMin();
	max = contrastSlider->GetMax();
	contrastSlider->SetValue((max - min)/2);
	imageSet_->SetContrast(0.5);
	
	Time_keeper_request_new_time(timeKeeper_, 1);
	Time_keeper_request_new_time(timeKeeper_, 0); //HACK
#define NODE_CREATION
#ifdef NODE_CREATION
	Scene_viewer_add_input_callback(CmguiManager::getInstance().getSceneViewer(),
			input_callback, (void*)this, 1/*add_first*/);

#endif //NODE_CREATION
	
//	modeller_->InitialiseModel();//REVISE
	
	cout << "ED Volume(EPI) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::EPICARDIUM, 0) << endl;
	cout << "ED Volume(ENDO) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::ENDOCARDIUM, 0) << endl;
	
	cout << "ES Volume(EPI) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::EPICARDIUM, 0.3) << endl;
	cout << "ES Volume(ENDO) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::ENDOCARDIUM, 0.3) << endl;
	
//	InitialiseVolumeGraph();
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	slider->SetTickFreq(28,0);
}

ViewerFrame::~ViewerFrame()
{
	delete imageSet_;
	delete modeller_;
}

void ViewerFrame::LoadImages()
{
	vector<string> sliceNames;
	sliceNames.push_back("SA1");
	sliceNames.push_back("SA2");
	sliceNames.push_back("SA3");
	sliceNames.push_back("SA4");
	sliceNames.push_back("SA5");
	sliceNames.push_back("SA6");
	sliceNames.push_back("LA1");
	sliceNames.push_back("LA2");
	sliceNames.push_back("LA3");
	
	imageSet_ = new ImageSet(sliceNames); //REFACTOR
	
	this->PopulateObjectList(); // fill in slice check box list
}

float ViewerFrame::GetCurrentTime() const
{
	return static_cast<float>(Cmiss_time_keeper_get_time(timeKeeper_));
}

void ViewerFrame::AddDataPoint(Cmiss_node* dataPointID, const Point3D& position)
{
	modeller_->AddDataPoint(dataPointID, position, GetCurrentTime());
	RefreshCmguiCanvas(); // need to force refreshing
}

void ViewerFrame::MoveDataPoint(Cmiss_node* dataPointID, const Point3D& newPosition)
{
	modeller_->MoveDataPoint(dataPointID, newPosition, GetCurrentTime());
	RefreshCmguiCanvas(); // need to force refreshing
}

void ViewerFrame::RemoveDataPoint(Cmiss_node* dataPointID)
{
	cout << __func__ << endl;
	modeller_->RemoveDataPoint(dataPointID, GetCurrentTime());
	RefreshCmguiCanvas();
}

void ViewerFrame::InitialiseModel()
{
	modeller_->InitialiseModel();
	modeller_->UpdateTimeVaryingModel();
	RefreshCmguiCanvas();
	
	cout << "ED Volume(EPI) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::EPICARDIUM, 0) << endl;
	cout << "ED Volume(ENDO) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::ENDOCARDIUM, 0) << endl;
}

void ViewerFrame::SmoothAlongTime()
{
	modeller_->SmoothAlongTime();
	RefreshCmguiCanvas();
	
	cout << "ED Volume(EPI) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::EPICARDIUM, 0) << endl;
	cout << "ED Volume(ENDO) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::ENDOCARDIUM, 0) << endl;
}

wxPanel* ViewerFrame::getPanel()
{
	return m_pPanel;
}

void ViewerFrame::TogglePlay(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "PlayButton", wxButton);
	
	if (animationIsOn_)
	{
		Time_keeper_stop(timeKeeper_);
		this->animationIsOn_ = false;
		button->SetLabel("play");
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

void ViewerFrame::Terminate(wxCloseEvent& event)
{
//	int answer = wxMessageBox("Quit program?", "Confirm",
//	                            wxYES_NO | wxCANCEL, this);
//	if (answer == wxYES)
//	{
//		Close();
		exit(0); //without this, the funny temporary window appears
//	}
}

int ViewerFrame::add_scene_object_to_scene_check_box(struct Scene_object *scene_object, void* checklistbox)
/*******************************************************************************
LAST MODIFIED : 2 Match 2007

DESCRIPTION :
Add scene_object as checklistbox item into the box.
==============================================================================*/
{
	wxCheckListBox *checklist = static_cast<wxCheckListBox*>(checklistbox);
	char *name;
	int visible;

	ENTER(add_scene_object_to_scene_check_box);
	GET_NAME(Scene_object)(scene_object, &name);
	checklist->Append(name);
	visible =(g_VISIBLE == Scene_object_get_visibility(scene_object));
	/* default selection */
	if ( visible ==1)
	{
		 checklist->Check((checklist->GetCount()-1),1);
	}
	if (checklist->GetCount() == 1)
	{
		 checklist->SetSelection(0);
	}
	
	DEALLOCATE(name);
	LEAVE;
	return(1);
}

//test
void ViewerFrame::PopulateObjectList()
{
	//TODO move Cmgui specific code to ImageSet
	//Should just obtain the list of slice names from ImageSet and use that to populate the check list box
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	for_each_Scene_object_in_Scene(scene,
		 add_scene_object_to_scene_check_box, (void *)objectList_);
}

void ViewerFrame::ObjectCheckListChecked(wxCommandEvent& event)
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

void ViewerFrame::ObjectCheckListSelected(wxCommandEvent& event)
{
	wxString name = objectList_->GetStringSelection();
	const ImagePlane& plane = imageSet_->GetImagePlane(name.mb_str());
	
	// compute the center of the image plane, eye(camera) position and the up vector
	Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
	Point3D eye = planeCenter - (plane.normal * 500); // this seems to determine the near clip plane
	Vector3D up(plane.yside);
	up.Normalise();
	
	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();	
	if (!Cmiss_scene_viewer_set_lookat_parameters_non_skew(
			sceneViewer, eye.x, eye.y, eye.z,
			planeCenter.x, planeCenter.y, planeCenter.z,
			up.x, up.y, up.z
			))
	{
		//Error;
	}
	RefreshCmguiCanvas();
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1 );
//	Cmiss_scene_viewer_view_all(sceneViewer);
	return;
}

void ViewerFrame::OnAnimationSliderEvent(wxCommandEvent& event)
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
		time = prevFrameTime + (float)1/(heartModel_.GetNumberOfModelFrames());
	}
	slider->SetValue(time * (max - min));
//	cout << "time = " << time << endl;;	
//	imageSet_->SetTime(time);
	time = (time > 0.99) ? 0 : time;
	
	Time_keeper_request_new_time(timeKeeper_, time);
	
	RefreshCmguiCanvas(); // forces redraw while silder is manipulated
	return;
}

void ViewerFrame::OnAnimationSpeedControlEvent(wxCommandEvent& event)
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

void ViewerFrame::RefreshCmguiCanvas()
{
	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();
//	Scene_viewer_redraw(sceneViewer);
	if (sceneViewer) 
	{
		Scene_viewer_redraw_now(sceneViewer);
	}
}

void ViewerFrame::SetTime(double time)
{
	//cout << "SetTime" <<endl;
	imageSet_->SetTime(time);
	
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	int min = slider->GetMin();
	int max = slider->GetMax();
	//cout << "min = " << min << " ,max = " << max <<endl; 
	slider->SetValue(static_cast<int>(static_cast<double>(max-min)*time) + min);
	
	return;
}

void ViewerFrame::ToggleHideShowAll(wxCommandEvent& event)
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

void ViewerFrame::ToggleHideShowOthers(wxCommandEvent& event)
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

void ViewerFrame::SetImageVisibility(bool visibility, int index)
{
	if (XRCCTRL(*this, "MII", wxCheckBox)->IsChecked())
		heartModel_.SetMIIVisibility(visibility, index);
	imageSet_->SetVisible(visibility, index);
}

void ViewerFrame::SetImageVisibility(bool visibility, const std::string& name)
{
	if (XRCCTRL(*this, "MII", wxCheckBox)->IsChecked())
	{
		if (name.length()) //REVISE
		{
			const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
	
			int i = find(sliceNames.begin(),sliceNames.end(), name) - sliceNames.begin();
			assert(i < sliceNames.size());
			heartModel_.SetMIIVisibility(visibility, i);
		}
		else
		{
			for (int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
			{
				heartModel_.SetMIIVisibility(visibility,i);
			}
		}
	}
	imageSet_->SetVisible(visibility, name);
}

void ViewerFrame::InitialiseMII()
{
	const vector<string>& sliceNames = imageSet_->GetSliceNames();
	vector<string>::const_iterator itr = sliceNames.begin();
	for (;itr != sliceNames.end();++itr)
	{
		RenderMII(*itr);
	}
}

void ViewerFrame::UpdateMII() //FIX
{
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	
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
		Cmiss_command_data_execute_command(command_data, str);
		
		Point3D pointTLCTransformed = mInv * plane.tlc;
		float d = DotProduct((pointTLCTransformed - Point3D(0,0,0)), normalTransformed);
		heartModel_.UpdateMII(index, d);
	}
}

void ViewerFrame::RenderMII(const std::string& sliceName) //MOVE to CAPModelLVPS4X4
{
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	
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
	Cmiss_command_data_execute_command(command_data, str);
	
	Point3D pointTLCTransformed = mInv * plane.tlc;
	float d = DotProduct((pointTLCTransformed - Point3D(0,0,0)), normalTransformed);

	sprintf((char*)str, "gfx modify g_element heart iso_surfaces exterior iso_scalar slice_%s iso_values %f use_faces select_on material gold selected_material default_selected render_shaded;"
				,sliceName.c_str() ,d);
//	cout << str << endl;
	Cmiss_command_data_execute_command(command_data, str);
}

#ifdef GRAPH
#include "VolumeGraph.h"

void ViewerFrame::InitialiseVolumeGraph()
{
	wxPanel* graphPanel = XRCCTRL(*this, "GraphPanel", wxPanel);
//	VolumeGraph* v = new VolumeGraph(graphPanel);
//	wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
//	topsizer->Add(v,
//		 wxSizerFlags(1).Align(wxALIGN_CENTER).Expand());
//	graphPanel->SetSizer(topsizer);
	
	std::vector<float> volumes;
	int numFrames = heartModel_.GetNumberOfModelFrames();
	for (int i = 0; i < numFrames; i++)
	{
		volumes.push_back(heartModel_.ComputeVolume(CAPModelLVPS4X4::EPICARDIUM, (float)i/numFrames));
	}
	MyFrame* v = new MyFrame(heartModel_, volumes);
	v->Show(true);
}
#endif

void ViewerFrame::OnMIICheckBox(wxCommandEvent& event)
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

void ViewerFrame::OnWireframeCheckBox(wxCommandEvent& event)
{
	heartModel_.SetModelVisibility(event.IsChecked());
}

void ViewerFrame::OnBrightnessSliderEvent(wxCommandEvent& event)
{
//	cout << "ViewerFrame::OnBrightnessSliderEvent" << endl;
	wxSlider* slider = XRCCTRL(*this, "BrightnessSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	float brightness = (float)(value - min) / (float)(max - min);
	imageSet_->SetBrightness(brightness);
	
	imageSet_->SetTime(0);
	imageSet_->SetTime(1);
	imageSet_->SetTime(GetCurrentTime());//FIX hack to force texture change
	
	RefreshCmguiCanvas();
}

void ViewerFrame::OnContrastSliderEvent(wxCommandEvent& event)
{
//	cout << "ViewerFrame::OnContrastSliderEvent" << endl;
	wxSlider* slider = XRCCTRL(*this, "ContrastSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	imageSet_->SetContrast(contrast);
	
	imageSet_->SetTime(0);
	imageSet_->SetTime(1);
	imageSet_->SetTime(GetCurrentTime());//FIX hack to force texture change
	
	RefreshCmguiCanvas();
}

void ViewerFrame::OnAcceptButtonPressed(wxCommandEvent& event)
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
		}
	}
	
	RefreshCmguiCanvas();
}

void ViewerFrame::OnModellingModeChanged(wxCommandEvent& event)
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

void ViewerFrame::OnAbout(wxCommandEvent& event)
{
    wxAboutDialogInfo info;
    info.SetName(_("CAP Client"));
    info.SetVersion(_("0.1 Alpha"));
    info.SetDescription(_("See http://www.cardiacatlas.org"));
    info.SetCopyright(_T("(C) 2009 ABI"));

    wxAboutBox(info);
}

void ViewerFrame::OnOpen(wxCommandEvent& event)
{
	wxString defaultPath = "./Data";
//	wxString defaultFilename = "";
//	wxString defaultExtension = "";
//	wxString wildcard = "";
//	int flags = wxOPEN;
	
//	wxString filename = wxFileSelector("Choose a file to open",
//			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
//	if ( !filename.empty() )
//	{
//	    // work with the file
//	    cout << __func__ << " - File name: " << filename.c_str() << endl;
//	}
	
	const wxString& dirname = wxDirSelector("Choose the folder that contains the model", defaultPath);
	if ( !dirname.empty() )
	{
		cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
		string filename(dirname.c_str());
		size_t positionOfLastSlash = filename.find_last_of("/\\");
		std::cout << "positionOfLastSlash = " << positionOfLastSlash << std::endl;
		string dirOnly = filename.substr(positionOfLastSlash+1); //FIX use wxFileName::SplitPath?
		string prefix = filename.substr(0, positionOfLastSlash+1); 
		std::cout << __func__ << " - dirOnly = " << dirOnly << std::endl;
		heartModel_.ReadModelFromFiles(dirOnly, prefix);
		
//		delete modeller_;
//		modeller_ = new CAPModeller(heartModel_); // initialise modeller and all the data points

//		wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
//		int numberOfItems = choice->GetCount();
//		for (int i = numberOfItems-1; i > 0; i--)
//		{
//			// Remove all items except Apex
//			choice->Delete(i);
//		}
		
		InitialiseMII();
		
		wxCheckBox* modelVisibilityCheckBox = XRCCTRL(*this, "Wireframe", wxCheckBox);
		heartModel_.SetModelVisibility(modelVisibilityCheckBox->IsChecked());
		
		wxCheckBox* miiCheckBox = XRCCTRL(*this, "MII", wxCheckBox);
		heartModel_.SetMIIVisibility(miiCheckBox->IsChecked());
	}
}

void ViewerFrame::OnSave(wxCommandEvent& event)
{
	wxString defaultPath = "./Data";
	wxString defaultFilename = "";
	wxString defaultExtension = "";
	wxString wildcard = "";
	int flags = wxSAVE;
	
	wxString filename = wxFileSelector("Save file",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
	    cout << __func__ << " - File name: " << filename.c_str() << endl;
	    heartModel_.WriteToFile(filename.c_str());
	}
}

void ViewerFrame::OnQuit(wxCommandEvent& event)
{
	int answer = wxMessageBox("Quit program?", "Confirm",
	                            wxYES_NO, this);
	if (answer == wxYES)
	{
		Close();
	}
}

BEGIN_EVENT_TABLE(ViewerFrame, wxFrame)
	EVT_BUTTON(XRCID("PlayButton"),ViewerFrame::TogglePlay) // play button
	EVT_SLIDER(XRCID("AnimationSlider"),ViewerFrame::OnAnimationSliderEvent) // animation slider
	EVT_SLIDER(XRCID("AnimationSpeedControl"),ViewerFrame::OnAnimationSpeedControlEvent)
	EVT_CHECKLISTBOX(XRCID("SliceList"), ViewerFrame::ObjectCheckListChecked)
	EVT_BUTTON(XRCID("HideShowAll"),ViewerFrame::ToggleHideShowAll) // hide all button
	EVT_BUTTON(XRCID("HideShowOthers"),ViewerFrame::ToggleHideShowOthers) // hide others button
	EVT_CHECKBOX(XRCID("MII"),ViewerFrame::OnMIICheckBox)
	EVT_CHECKBOX(XRCID("Wireframe"),ViewerFrame::OnWireframeCheckBox)
	EVT_LISTBOX(XRCID("SliceList"), ViewerFrame::ObjectCheckListSelected)
	EVT_SLIDER(XRCID("BrightnessSlider"),ViewerFrame::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),ViewerFrame::OnContrastSliderEvent)
	EVT_BUTTON(XRCID("AcceptButton"),ViewerFrame::OnAcceptButtonPressed)
	EVT_CHOICE(XRCID("ModeChoice"),ViewerFrame::OnModellingModeChanged)
	EVT_CLOSE(ViewerFrame::Terminate)
	EVT_MENU(XRCID("QuitMenuItem"),  ViewerFrame::OnQuit)
	EVT_MENU(XRCID("AboutMenuItem"), ViewerFrame::OnAbout)
	EVT_MENU(XRCID("OpenMenuItem"), ViewerFrame::OnOpen)
	EVT_MENU(XRCID("SaveMenuItem"), ViewerFrame::OnSave)
END_EVENT_TABLE()
