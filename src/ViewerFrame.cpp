// For compilers that don't support precompilation, include "wx/wx.h";

#include "wx/xrc/xmlres.h"
#include "wx/splitter.h"
//#include "wx/slider.h"
//#include "wx/button.h"

#include "Config.h"
#include "ViewerFrame.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
#include "CmguiExtensions.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

extern "C"
{
#include "api/cmiss_region.h"
#include "time/time_keeper.h"
#include "time/time.h"
#include "command/cmiss.h"
#include "graphics/scene.h"	
#include "three_d_drawing/graphics_buffer.h"
#include "graphics/graphics_library.h"
#include "general/debug.h"
	
#include "graphics/material.h"
#include "graphics/element_group_settings.h"
#include "graphics/glyph.h"
	
#include "general/geometry.h"
}

//#include <OpenGL/gl.h>

using namespace std;
struct Viewer_frame_element_constraint_function_data
{
	struct FE_element *element, *found_element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *coordinate_field;
}; 

static int Viewer_frame_element_constraint_function(FE_value *point,
	void *void_data)
/*******************************************************************************
LAST MODIFIED : 14 February 2008

DESCRIPTION : need to find the point of intersection between picking ray and obj
==============================================================================*/
{
	int return_code;
	struct Viewer_frame_element_constraint_function_data *data;

	ENTER(Viewer_frame_element_constraint_function_data);
	if (point && (data = (struct Viewer_frame_element_constraint_function_data *)void_data))
	{
		data->found_element = data->element;
		return_code = Computed_field_find_element_xi(data->coordinate_field,
			point, /*number_of_values*/3, &(data->found_element), 
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

Cmiss_node_id Cmiss_create_or_select_node_from_screen_coords(double x, double y, float time, Point3D& coords)
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
	struct FE_node *picked_node;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Scene_picked_object *scene_picked_object, *scene_picked_object2;
	struct GT_element_group *gt_element_group, *gt_element_group_element;
	struct GT_element_settings *gt_element_settings, *gt_element_settings_element;
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(
			CmguiManager::getInstance().getCmissCommandData());
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	struct Graphics_buffer* graphics_buffer = Scene_viewer_get_graphics_buffer(CmguiManager::getInstance().getSceneViewer());
	
	if (scene_picked_object_list=
		Scene_pick_objects(scene,interaction_volume,graphics_buffer))
	{
		nearest_element = (struct FE_element *)NULL;

		picked_node=Scene_picked_object_list_get_nearest_node(
											scene_picked_object_list,1 /* use_data */,
											(struct Cmiss_region *)NULL,&scene_picked_object,
											&gt_element_group,&gt_element_settings);
		
		nearest_element=Scene_picked_object_list_get_nearest_element(
			scene_picked_object_list,(struct Cmiss_region *)NULL,
			/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
			/*select_lines_enabled*/0, &scene_picked_object2,
			&gt_element_group_element,&gt_element_settings_element);
		
		/* Reject the previously picked node if the element is nearer */
		if (picked_node && nearest_element)
		{
			if (Scene_picked_object_get_nearest(scene_picked_object) >
				Scene_picked_object_get_nearest(scene_picked_object2))
			{
				picked_node = (struct FE_node *)NULL;
			}
		}	
		DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
	}
	
	/* Find the intersection of the element and the interaction volume */
	Computed_field* nearest_element_coordinate_field = (struct Computed_field *)NULL;
	if (picked_node)
	{
		return picked_node; // Cmiss_node = FE_node;
	}
	else if (nearest_element)
	{
		if (!(nearest_element_coordinate_field = 
				GT_element_settings_get_coordinate_field(gt_element_settings_element)))
		{
			nearest_element_coordinate_field = 
				GT_element_group_get_default_coordinate_field(
				gt_element_group_element);
		}


//		double node_coordinates[3];

		Viewer_frame_element_constraint_function_data constraint_data;
		constraint_data.element = nearest_element;
		constraint_data.found_element = nearest_element;
		constraint_data.coordinate_field = nearest_element_coordinate_field;
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			constraint_data.xi[i] = 0.5;
		}
		double node_coordinates[3];
		return_code = Interaction_volume_get_placement_point(interaction_volume,
			node_coordinates, Viewer_frame_element_constraint_function,
			&constraint_data);
		
		if (return_code)
		{
			Cmiss_region* region = Cmiss_element_get_region(nearest_element); // this doesn't work with groups
			cout << "region = " << Cmiss_region_get_path(region) << endl;
			
			coords.x = node_coordinates[0], coords.y = node_coordinates[1], coords.z = node_coordinates[2];
			cout << "debug: intersection point = " << coords <<  endl;
					
			return Cmiss_create_data_point_at_coord(region, nearest_element_coordinate_field, (float*)&coords, time);
		}
	}
	
	return 0;
}

int Cmiss_move_node_to_screen_coords(Cmiss_node_id node, double x, double y, float time, Point3D& coords)
{
	double node_coordinates[3];
	Cmiss_field_id field;
	FE_element* element = Cmiss_get_ray_intersection_point(x, y, node_coordinates, &field);
	
	coords.x = node_coordinates[0], coords.y = node_coordinates[1], coords.z = node_coordinates[2];
	if (Cmiss_field_set_values_at_node( field, node, 0 /* time*/ , 3 , (float*)& coords))
	{
		return 1;
	}
		
	return 0;
}
		
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
		Point3D coords;
		selectedNode = Cmiss_create_or_select_node_from_screen_coords(x, y, time, coords);
		if (!selectedNode)
		{
			// No node selected or created
			return 0;
		}
		frame->AddDataPoint(selectedNode, DataPoint(selectedNode, coords, time));
	}
	else if (input->type == GRAPHICS_BUFFER_MOTION_NOTIFY)
	{
		// Move node		
		if (!selectedNode)
		{
			cout << "GRAPHICS_BUFFER_MOTION_NOTIFY with NULL selectedNode" << endl;
			frame->InitialiseModel();
			return 0;
		}
		Point3D coords;
		cout << "Mouse Drag node = " << Cmiss_node_get_identifier(selectedNode) << endl;
		Cmiss_move_node_to_screen_coords(selectedNode, x, y, time, coords);
		
		cout << "Move coord = " << coords << endl;
		frame->MoveDataPoint(selectedNode, coords);
	}
	else if (input->type == GRAPHICS_BUFFER_BUTTON_RELEASE)
	{
		cout << "Mouse released" << endl;
		frame->SmoothAlongTime();
		selectedNode = NULL;
	}
	
//	if (input->type ==GRAPHICS_BUFFER_BUTTON_RELEASE
//			&& input->input_modifier==GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT)
//	{
//		cout << "input->input_modifier==" << input->input_modifier << endl;
//	}

//	//Get intersection point
//	double node_coordinates[3];
//	double x = (double)(input->position_x);
//	double y = (double)(input->position_y);
//	
//	Cmiss_field_id nearest_element_coordinate_field;
//	FE_element* element = Cmiss_get_ray_intersection_point(x, y, node_coordinates, &nearest_element_coordinate_field);
//	
//	//Create a node 
//	if (element)
//	{		
//		Cmiss_region* region = Cmiss_element_get_region(element); // this doenst work with groups
//		if (!region)
//		{
//			cout << "input_callback: Can't get region from element" << endl;
//		}
//		cout << "region = " << Cmiss_region_get_path(region) << endl;
//		
//		Point3D coords(node_coordinates[0], node_coordinates[1], node_coordinates[2]);
//		cout << "debug: intersection point = " << coords <<  endl;
//				
//		if (Cmiss_node_id node = Cmiss_create_data_point_at_coord(region, nearest_element_coordinate_field, (float*)&coords))
//		{
//			frame->AddDataPoint(new DataPoint(node,coords));
//		}
//	}	
	
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
	modeller_(heartModel_)
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
	
	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().createSceneViewer(m_pPanel);
	Cmiss_scene_viewer_view_all(sceneViewer);
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1 );
	
#define TIME_OBJECT_CALLBACK_TEST
#ifdef TIME_OBJECT_CALLBACK_TEST
	Cmiss_time_notifier_id time_notifier = Cmiss_time_notifier_create_regular(10, 0);
	Cmiss_time_notifier_add_callback(time_notifier, time_callback, (void*)this);
	Cmiss_time_keeper_add_time_notifier(timeKeeper_, time_notifier);
#endif		
#endif //TEXTURE_ANIMATION
	
	this->PopulateObjectList(); // fill in slice check box list
	
	//Load model
	heartModel_.ReadModelFromFiles("test");	
	//heartModel_.SetRenderMode(CAPModelLVPS4X4::WIREFRAME);//this resets timer frequency for model!! if called after its been cleared!!??
	vector<string>::iterator itr = sliceNames.begin();
	for (;itr != sliceNames.end();++itr)
	{
		RenderMII(*itr);
	}
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
	
	modeller_.InitialiseModel();//REVISE
}

ViewerFrame::~ViewerFrame()
{
	delete imageSet_;
}

//void ViewerFrame::AddDataPoint(DataPoint* dataPoint)
//{
//	modeller_.AddDataPoint(dataPoint);
//}

float ViewerFrame::GetCurrentTime() const
{
	return static_cast<float>(Cmiss_time_keeper_get_time(timeKeeper_));
}

void ViewerFrame::AddDataPoint(Cmiss_node_id dataPointID, const DataPoint& dataPoint)
{
	modeller_.AddDataPoint(dataPointID, dataPoint);
	RefreshCmguiCanvas(); // need to force refreshing
}

void ViewerFrame::MoveDataPoint(Cmiss_node_id dataPointID, const Point3D& newPosition)
{
	modeller_.MoveDataPoint(dataPointID, newPosition, GetCurrentTime());
	RefreshCmguiCanvas(); // need to force refreshing
}

void ViewerFrame::InitialiseModel()
{
	modeller_.InitialiseModel();
	modeller_.UpdateTimeVaryingModel();
	RefreshCmguiCanvas();
}

void ViewerFrame::SmoothAlongTime()
{
	modeller_.SmoothAlongTime();
	
#ifdef TEST
	static bool smooth = false;
	if (smooth)
	{
		modeller_.SmoothAlongTime();
		smooth = false;
	}
	else
	{
		modeller_.InitialiseModel();
		modeller_.UpdateTimeVaryingModel();
		smooth = true;
	}
#endif
	RefreshCmguiCanvas();
}

wxPanel* ViewerFrame::getPanel()
{
	return m_pPanel;
}

void ViewerFrame::TogglePlay(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "button_1", wxButton);
	
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
	exit(0); //without this, the funny temporary window appears
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
	
//	//hack to test the callback works when time is manually set to 0
//	if ("heart"==name)
//	{
//		Time_keeper_request_new_time(timeKeeper_, 0);
//	}
	
	if(objectList_->IsChecked(selection))
	{
		SetImageVisibility(true, name.mb_str());
	}
	else
	{
		SetImageVisibility(false, name.mb_str());
	}
	
//	RefreshCmguiCanvas(); //Necessary?? - doesn't help with the problem where the canvas doesn't redraw
	this->Refresh();//test to see if this helps with the problem where 3d canvas doesnt update
}

void ViewerFrame::ObjectCheckListSelected(wxCommandEvent& event)
{
	wxString name = objectList_->GetStringSelection();
	const ImagePlane& plane = imageSet_->GetImagePlane(name.mb_str());
	
	// compute the center of the image plane, eye(camera) position and the up vector
	Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
	Point3D eye = planeCenter + (plane.normal * 500); // this seems to determine the near clip plane
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
	wxSlider* slider = XRCCTRL(*this, "slider_1", wxSlider);
	int value = slider->GetValue();
	
	int min = slider->GetMin();
	int max = slider->GetMax();
	double time =  (double)(value - min) / (double)(max - min);
	
//	cout << "time = " << time << endl;;	
//	imageSet_->SetTime(time);
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
	
	wxSlider* slider = XRCCTRL(*this, "slider_1", wxSlider);
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

void ViewerFrame::RenderMII(const std::string& sliceName)
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
	sprintf((char*)str, "gfx define field slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[%f %f %f]\";",
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

BEGIN_EVENT_TABLE(ViewerFrame, wxFrame)
	EVT_BUTTON(XRCID("button_1"),ViewerFrame::TogglePlay) // play button
	EVT_SLIDER(XRCID("slider_1"),ViewerFrame::OnAnimationSliderEvent) // animation slider
	EVT_SLIDER(XRCID("AnimationSpeedControl"),ViewerFrame::OnAnimationSpeedControlEvent)
	EVT_CHECKLISTBOX(XRCID("SliceList"), ViewerFrame::ObjectCheckListChecked)
	EVT_BUTTON(XRCID("HideShowAll"),ViewerFrame::ToggleHideShowAll) // hide all button
	EVT_BUTTON(XRCID("HideShowOthers"),ViewerFrame::ToggleHideShowOthers) // hide others button
	EVT_CHECKBOX(XRCID("MII"),ViewerFrame::OnMIICheckBox)
	EVT_CHECKBOX(XRCID("Wireframe"),ViewerFrame::OnWireframeCheckBox)
	EVT_LISTBOX(XRCID("SliceList"), ViewerFrame::ObjectCheckListSelected)
	EVT_SLIDER(XRCID("BrightnessSlider"),ViewerFrame::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),ViewerFrame::OnContrastSliderEvent)
	EVT_CLOSE(ViewerFrame::Terminate)
END_EVENT_TABLE()
