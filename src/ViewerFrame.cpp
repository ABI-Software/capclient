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


struct Viewer_frame_element_constraint_function_data
{
	struct FE_element *element, *found_element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *coordinate_field;
}; 

static int Viewer_frame_element_constraint_function(FE_value *point,
	void *void_data)
{
	int return_code;
	struct Viewer_frame_element_constraint_function_data *data;

	ENTER(Viewer_frame_element_constraint_function_data);
	if (point && (data = (struct Viewer_frame_element_constraint_function_data *)void_data))
	{
		data->found_element = data->element;
		return_code = Computed_field_find_element_xi(data->coordinate_field,
			point, /*number_of_values*/3, /*time ok to be 0 since slices don't move wrt time*/ 0.0, &(data->found_element), 
			data->xi, /*element_dimension*/2, 
			(struct Cmiss_region *)NULL, /*propagate_field*/0, /*find_nearest_location*/1);
		Computed_field_evaluate_in_element(data->coordinate_field,
			data->found_element, data->xi, /*time*/0.0, (struct FE_element *)NULL,
			point, (FE_value *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_element_constraint_function.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_element_constraint_function */

Cmiss_region_id Cmiss_get_slice_region(double x, double y, double* node_coordinates, Cmiss_region_id region)
{
	int return_code = 0;

	GLint viewport[4];
	
	glGetIntegerv(GL_VIEWPORT,viewport);
	double viewport_left   = (double)(viewport[0]);
	double viewport_bottom = (double)(viewport[1]);
	double viewport_width  = (double)(viewport[2]);
	double viewport_height = (double)(viewport[3]);
	
	double centre_x = x;
	/* flip y as x event has y=0 at top of window, increasing down */
	double centre_y = viewport_height-y-1.0;
	
//	std::cout << viewport_height <<"," <<centre_y<< std::endl;
	
	GLdouble modelview_matrix[16], window_projection_matrix[16];

	Scene_viewer_get_modelview_matrix(CmguiManager::getInstance().getSceneViewer(), modelview_matrix);
	Scene_viewer_get_window_projection_matrix(CmguiManager::getInstance().getSceneViewer(), window_projection_matrix);
	
	
	double size_x = 7.0;//FIX
	double size_y = 7.0;
	
	struct Interaction_volume *interaction_volume = create_Interaction_volume_ray_frustum(
					modelview_matrix,window_projection_matrix,
					viewport_left,viewport_bottom,viewport_width,viewport_height,
					centre_x,centre_y,size_x,size_y);
	
	FE_element* nearest_element;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene_picked_object *scene_picked_object2;
	struct GT_element_group *gt_element_group_element;
	struct GT_element_settings *gt_element_settings_element;
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(
			CmguiManager::getInstance().getCmissCommandData());
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	struct Graphics_buffer* graphics_buffer = Scene_viewer_get_graphics_buffer(CmguiManager::getInstance().getSceneViewer());
	
	if (scene_picked_object_list=
		Scene_pick_objects(scene,interaction_volume,graphics_buffer))
	{
		nearest_element = (struct FE_element *)NULL;

		nearest_element=Scene_picked_object_list_get_nearest_element(
			scene_picked_object_list,region,
			/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
			/*select_lines_enabled*/0, &scene_picked_object2,
			&gt_element_group_element,&gt_element_settings_element);
		
		DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
	}
	
	/* Find the intersection of the element and the interaction volume */
	Computed_field* nearest_element_coordinate_field = (struct Computed_field *)NULL;
	if (nearest_element)
	{
		if (!(nearest_element_coordinate_field = 
				GT_element_settings_get_coordinate_field(gt_element_settings_element)))
		{
			nearest_element_coordinate_field = 
				GT_element_group_get_default_coordinate_field(
				gt_element_group_element);
		}

		//Test
		//*field = nearest_element_coordinate_field;

//		double node_coordinates[3];

		Viewer_frame_element_constraint_function_data constraint_data;
		constraint_data.element = nearest_element;
		constraint_data.found_element = nearest_element;
		constraint_data.coordinate_field = nearest_element_coordinate_field;
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			constraint_data.xi[i] = 0.5;
		}
		return_code = Interaction_volume_get_placement_point(interaction_volume,
			node_coordinates, Viewer_frame_element_constraint_function,
			&constraint_data);
	}

	if (return_code)
	{
		Cmiss_region_id region = GT_element_group_get_Cmiss_region(gt_element_group_element);
		return region;
	}
	else
	{
		return (Cmiss_region_id)0;
	}
//	return return_code;
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
//	ViewerFrame* frame = static_cast<ViewerFrame*>(viewer_frame_void);
	
	double x = (double)(input->position_x);
	double y = (double)(input->position_y);
	
	static double coords[3];
	static Cmiss_region_id selectedRegion;
	if (input->type == GRAPHICS_BUFFER_BUTTON_PRESS)
	{
		// Select node or create one
		cout << "Mouse button number = " << input->button_number << endl;
		selectedRegion = Cmiss_get_slice_region(x, y, (double*)coords, (Cmiss_region_id)0);
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
			Cmiss_region_id tempRegion = Cmiss_get_slice_region(x, y, (double*)new_coords, selectedRegion);
			if (!tempRegion)
			{
				cout << __func__ << "ERROR\n";
			}
			string sliceName = Cmiss_region_get_path(selectedRegion);
//			cout << "dragged = " << sliceName << endl;
//			cout << "coords = " << coords[0] << ", " << coords[1] << ", " << coords[2] << "\n";
//			cout << "new_coords = " << new_coords[0] << ", " << new_coords[1] << ", " << new_coords[2] << "\n";
			for (int nodeNum = 1; nodeNum < 5; nodeNum++)
			{
				char nodeName[256];
				sprintf(nodeName,"%d", nodeNum);
				if (Cmiss_node* node = Cmiss_region_get_node(selectedRegion, nodeName))
				{
					float x, y, z;
					FE_node_get_position_cartesian(node, 0, &x, &y, &z, 0);
					cout << "before = " << x << ", " << y << ", " << z << endl;
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
	
	//TEST - Image Shifting
//	Cmiss_scene_viewer_add_input_callback(CmguiManager::getInstance().getSceneViewer(),
//			input_callback_image_shifting, (void*)this, 1/*add_first*/);
	
//	modeller_->InitialiseModel();//REVISE
	
	cout << "ED Volume(EPI) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::EPICARDIUM, 0) << endl;
	cout << "ED Volume(ENDO) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::ENDOCARDIUM, 0) << endl;
	
	cout << "ES Volume(EPI) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::EPICARDIUM, 0.3) << endl;
	cout << "ES Volume(ENDO) = " << heartModel_.ComputeVolume(CAPModelLVPS4X4::ENDOCARDIUM, 0.3) << endl;
	
//	InitialiseVolumeGraph();
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	slider->SetTickFreq(28,0);
	
	// we use PNG & JPEG images in our HTML page
	//wxImage::AddHandler(new wxJPEGHandler);
	wxImage::AddHandler(new wxPNGHandler);
	
	CreateStatusBar(0);
	
	SetTime(0.0);
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
//	sliceNames.push_back("SA7");
//	sliceNames.push_back("SA8");
//	sliceNames.push_back("SA9");
//	sliceNames.push_back("SA10");
	//sliceNames.push_back("SA11");
	//sliceNames.push_back("SA12");
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

void ViewerFrame::OnTogglePlay(wxCommandEvent& event)
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
	
	//DEALLOCATE(name);
	free(name);
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

void ViewerFrame::OnObjectCheckListChecked(wxCommandEvent& event)
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

void ViewerFrame::OnObjectCheckListSelected(wxCommandEvent& event)
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
	
	int frameNumber = heartModel_.MapToModelFrameNumber(time);
	std::ostringstream frameNumberStringStream;
	frameNumberStringStream << "Frame Number: " << frameNumber;
	SetStatusText(wxT(frameNumberStringStream.str().c_str()), 0);
	return;
}

void ViewerFrame::OnToggleHideShowAll(wxCommandEvent& event)
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

void ViewerFrame::OnToggleHideShowOthers(wxCommandEvent& event)
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

	sprintf((char*)str, "gfx modify g_element heart iso_surfaces exterior iso_scalar slice_%s iso_values %f use_faces select_on material gold selected_material default_selected render_shaded line_width 2;"
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

#include "CAPHtmlWindow.h"

void ViewerFrame::OnAbout(wxCommandEvent& event)
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

#include <wx/dir.h>

void EnumerateAllFiles(const wxString& dirname)
{
	wxDir dir(dirname);
	
	if ( !dir.IsOpened() )
	{
		// deal with the error here - wxDir would already log an error message
		// explaining the exact reason of the failure
		cout << "Error";
		return;
	}
	
	puts("Enumerating object files in current directory:");
	
//	wxString filename;
	
//	bool cont = dir.GetFirst(&filename);
//	while ( cont )
//	{
//		printf("%s\n", filename.c_str());
//	
//		cont = dir.GetNext(&filename);
//	}
	
	wxArrayString files;
	dir.GetAllFiles(dirname, &files);
	
	for (int i = 0; i < files.GetCount(); i++)
	{
		string filename(files[i].c_str());
		size_t positionOfLastSlash = filename.find_last_of("/\\");
		//std::cout << "positionOfLastSlash = " << positionOfLastSlash << std::endl;
		string fileNameOnly = filename.substr(positionOfLastSlash+1); //FIX use wxFileName::SplitPath?
		string prefix = filename.substr(0, positionOfLastSlash+1); 

		if (fileNameOnly[0] != '.')
			cout << files[i] << "\n";
	}
}

void ViewerFrame::OnOpenImages(wxCommandEvent& event)
{
	wxString currentWorkingDir = wxGetCwd();
		wxString defaultPath = currentWorkingDir.Append("/Data");
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
		
		const wxString& dirname = wxDirSelector("Choose the folder that contains the images", defaultPath);
		if ( !dirname.empty() )
		{
			cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
//			string filename(dirname.c_str());
//			size_t positionOfLastSlash = filename.find_last_of("/\\");
//			std::cout << "positionOfLastSlash = " << positionOfLastSlash << std::endl;
//			string dirOnly = filename.substr(positionOfLastSlash+1); //FIX use wxFileName::SplitPath?
//			string prefix = filename.substr(0, positionOfLastSlash+1); 
//			std::cout << __func__ << " - dirOnly = " << dirOnly << std::endl;
			
			EnumerateAllFiles(dirname);
		}
}

void ViewerFrame::OnOpen(wxCommandEvent& event)
{
	wxString currentWorkingDir = wxGetCwd();
	wxString defaultPath = currentWorkingDir.Append("/Data");
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
		
		delete modeller_;
		modeller_ = new CAPModeller(heartModel_); // initialise modeller and all the data points

		wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
		int numberOfItems = choice->GetCount();
		for (int i = numberOfItems-1; i > 0; i--)
		{
			// Remove all items except Apex
			choice->Delete(i);
		}
		choice->SetSelection(0);
		
		InitialiseMII(); // This turns on all MII's
		
		wxCheckBox* modelVisibilityCheckBox = XRCCTRL(*this, "Wireframe", wxCheckBox);
		heartModel_.SetModelVisibility(modelVisibilityCheckBox->IsChecked());
		
		wxCheckBox* miiCheckBox = XRCCTRL(*this, "MII", wxCheckBox);
		//heartModel_.SetMIIVisibility(miiCheckBox->IsChecked());
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

void ViewerFrame::OnPlaneShiftButtonPressed(wxCommandEvent& event)
{
	static bool isPlaneShiftModeOn = false;
	
	wxButton* button = XRCCTRL(*this, "PlaneShiftButton", wxButton);
	assert(button);
	
	if (!isPlaneShiftModeOn)
	{
		isPlaneShiftModeOn = true;
		button->SetLabel("End Shifting");
		
		Cmiss_scene_viewer_remove_input_callback(CmguiManager::getInstance().getSceneViewer(),
						input_callback, (void*)this);
		Cmiss_scene_viewer_add_input_callback(CmguiManager::getInstance().getSceneViewer(),
						input_callback_image_shifting, (void*)this, 1/*add_first*/);
	}
	else
	{
		isPlaneShiftModeOn = false;
		button->SetLabel("Start Shifting");
		
		Cmiss_scene_viewer_remove_input_callback(CmguiManager::getInstance().getSceneViewer(),
						input_callback_image_shifting, (void*)this);
		Cmiss_scene_viewer_add_input_callback(CmguiManager::getInstance().getSceneViewer(),
						input_callback, (void*)this, 1/*add_first*/);
		
		imageSet_->WritePlaneInfoToFiles();
	}
	
	return;
}

BEGIN_EVENT_TABLE(ViewerFrame, wxFrame)
	EVT_BUTTON(XRCID("PlayButton"),ViewerFrame::OnTogglePlay) // play button
	EVT_SLIDER(XRCID("AnimationSlider"),ViewerFrame::OnAnimationSliderEvent) // animation slider
	EVT_SLIDER(XRCID("AnimationSpeedControl"),ViewerFrame::OnAnimationSpeedControlEvent)
	EVT_CHECKLISTBOX(XRCID("SliceList"), ViewerFrame::OnObjectCheckListChecked)
	EVT_BUTTON(XRCID("HideShowAll"),ViewerFrame::OnToggleHideShowAll) // hide all button
	EVT_BUTTON(XRCID("HideShowOthers"),ViewerFrame::OnToggleHideShowOthers) // hide others button
	EVT_CHECKBOX(XRCID("MII"),ViewerFrame::OnMIICheckBox)
	EVT_CHECKBOX(XRCID("Wireframe"),ViewerFrame::OnWireframeCheckBox)
	EVT_LISTBOX(XRCID("SliceList"), ViewerFrame::OnObjectCheckListSelected)
	EVT_SLIDER(XRCID("BrightnessSlider"),ViewerFrame::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),ViewerFrame::OnContrastSliderEvent)
	EVT_BUTTON(XRCID("AcceptButton"),ViewerFrame::OnAcceptButtonPressed)
	EVT_CHOICE(XRCID("ModeChoice"),ViewerFrame::OnModellingModeChanged)
	EVT_CLOSE(ViewerFrame::Terminate)
	EVT_MENU(XRCID("QuitMenuItem"),  ViewerFrame::OnQuit)
	EVT_MENU(XRCID("AboutMenuItem"), ViewerFrame::OnAbout)
	EVT_MENU(XRCID("OpenImagesMenuItem"), ViewerFrame::OnOpenImages)
	EVT_MENU(XRCID("OpenMenuItem"), ViewerFrame::OnOpen)
	EVT_MENU(XRCID("SaveMenuItem"), ViewerFrame::OnSave)
	EVT_BUTTON(XRCID("PlaneShiftButton"), ViewerFrame::OnPlaneShiftButtonPressed)
END_EVENT_TABLE()
