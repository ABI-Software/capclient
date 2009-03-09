// For compilers that don't support precompilation, include "wx/wx.h";

#include "wx/xrc/xmlres.h"
#include "wx/splitter.h"
//#include "wx/slider.h"
//#include "wx/button.h"

#include "Config.h"
#include "ViewerFrame.h"
#include "CmguiManager.h"
#include "DICOMImage.h"
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


static int input_callback(struct Scene_viewer *scene_viewer, 
		struct Graphics_buffer_input *input, void *viewer_frame_void)
{
//	cout << "input_callback()" << endl;
	int return_code = 0;
	
	if (input->type!=GRAPHICS_BUFFER_BUTTON_RELEASE
			|| input->input_modifier!=GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT)
	{
		return 0;
	}
//	if (input->type ==GRAPHICS_BUFFER_BUTTON_RELEASE
//			&& input->input_modifier==GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT)
//	{
//		cout << "input->input_modifier==" << input->input_modifier << endl;
//	}
	
	ViewerFrame* frame = static_cast<ViewerFrame*>(viewer_frame_void);
	
	GLint viewport[4];
	
	glGetIntegerv(GL_VIEWPORT,viewport);
	double viewport_left   = (double)(viewport[0]);
	double viewport_bottom = (double)(viewport[1]);
	double viewport_width  = (double)(viewport[2]);
	double viewport_height = (double)(viewport[3]);
	
	double centre_x=(double)(input->position_x);
	/* flip y as x event has y=0 at top of window, increasing down */
	double centre_y=viewport_height-(double)(input->position_y)-1.0;
	
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
			scene_picked_object_list,(struct Cmiss_region *)NULL,
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

//		if (picked_node=Node_tool_create_node_at_interaction_volume(
//			node_tool,scene,interaction_volume,nearest_element,
//			nearest_element_coordinate_field))
//		{
//			node_tool->picked_node_was_unselected=1;
//		}
//		else
//		{
//			node_tool->picked_node_was_unselected=0;
//		}
		double node_coordinates[3];
//		Interaction_volume_get_placement_point(interaction_volume,
//							node_coordinates, (Interation_volume_constraint_function)NULL,
//							(void *)NULL);
		Viewer_frame_element_constraint_function_data constraint_data;
		constraint_data.element = nearest_element;
		constraint_data.found_element = nearest_element;
		constraint_data.coordinate_field = nearest_element_coordinate_field;
		for (int i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
		{
			constraint_data.xi[i] = 0.5;
		}
		Interaction_volume_get_placement_point(interaction_volume,
			node_coordinates, Viewer_frame_element_constraint_function,
			&constraint_data);
		Point3D coords(node_coordinates[0], node_coordinates[1], node_coordinates[2]);
		cout << "debug: " << coords <<  endl;
		
		Cmiss_region* root_region = Cmiss_command_data_get_root_region(
				CmguiManager::getInstance().getCmissCommandData());
		Cmiss_region* cmiss_region;
		if(!Cmiss_region_get_region_from_path(root_region, "DataPoints", &cmiss_region))
		{
			//error
			std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< cmiss_region <<endl;
		}
		
		FE_region* fe_region = Cmiss_region_get_FE_region(cmiss_region);
		if (!fe_region)
		{
			cout << "fe_region is null" << endl;
		}
		
		int node_identifier = FE_region_get_next_FE_node_identifier(fe_region, /*start*/1);
		cout << "node id = " << node_identifier << endl;
		
		Cmiss_node_id node = create_Cmiss_node(node_identifier, cmiss_region);
		Cmiss_region_merge_Cmiss_node(cmiss_region, node);
		Cmiss_field_id field = nearest_element_coordinate_field;
		Cmiss_field_finite_element_define_at_node(
			field,  node,
			0 /* time_sequence*/, 0/* node_field_creator*/);
		
		Cmiss_field_set_values_at_node( field, node, 0 /* time*/ , 3 , (float *)&coords);
	}
	else
	{
//		node_tool->picked_node_was_unselected=0;
	}

	return return_code;
}

static int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
	cout << "Time_call_back time = " << current_time << endl;
	
//	ImageSet* imageSet = reinterpret_cast<ImageSet*>(user_data);
//	imageSet->SetTime(current_time);
	
	ViewerFrame* frame = static_cast<ViewerFrame*>(user_data);
	frame->SetTime(current_time);
	
//	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();
//	Scene_viewer_redraw(sceneViewer);
	frame->RefreshCmguiCanvas(); // this forces refresh even when UI is being manipulated by user
	
	return 0;
}

ViewerFrame::ViewerFrame(Cmiss_command_data* command_data_)
: 
	command_data(command_data_),
	animationIsOn(false),
	timeKeeper_(Cmiss_command_data_get_default_time_keeper(command_data_))
{
	// Load layout from .xrc file
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ViewerFrame"));
	
	// HACK to make sure the layout is properly applied (for Mac)
	this->Show(true);
	wxSplitterWindow* win = XRCCTRL(*this, "window_1", wxSplitterWindow);
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
	Time_object* time_object = create_Time_object("Texture_animation_timer");
	
	Time_object_add_callback(time_object,time_callback,(void*)this);
	Time_object_set_time_keeper(time_object, timeKeeper_);
//		Time_object_set_update_frequency(time_object,28);//BUG?? doesnt actually update 28 times -> only 27 
	
	Time_keeper_set_minimum(timeKeeper_, -0.01); //workaround for the timer bug
	Time_keeper_set_maximum(timeKeeper_, 1);
	
#endif		
#endif //TEXTURE_ANIMATION
	
	//test
//	int Scene_viewer_add_input_callback(struct Scene_viewer *scene_viewer,
//		CMISS_CALLBACK_FUNCTION(Scene_viewer_input_callback) *function,
//		void *user_data, int add_first)
	
	Scene_viewer_add_input_callback(CmguiManager::getInstance().getSceneViewer(),
			input_callback, (void*)this, 0/*add_first*/);
	
	//FIX move to a separate function
	Cmiss_region* region = Cmiss_command_data_get_root_region(command_data);
	
	stringstream pathStream;
	pathStream << prefix << "templates/DataPoints.exnode";
	string filename = pathStream.str();
	if (!Cmiss_region_read_file(region,(char*)filename.c_str()))
	{
		std::cout << "Error reading ex file - DataPoints.exnode" << std::endl;
	}
	
	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_NODE_POINTS);
//	//hack
	Graphical_material* material = create_Graphical_material("DataPoints");
	GT_element_settings_set_selected_material(settings, material);

	//Glyphs
	/* default to point glyph for fastest possible display */
	GT_object *glyph, *old_glyph;
	Glyph_scaling_mode glyph_scaling_mode;
	Triple glyph_centre,glyph_scale_factors,glyph_size;
	Computed_field *orientation_scale_field, *variable_scale_field; ;
	glyph=make_glyph_sphere("sphere",12,6);
	if (!(GT_element_settings_get_glyph_parameters(settings,
		 &old_glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
		 &orientation_scale_field, glyph_scale_factors,
		 &variable_scale_field) &&
		GT_element_settings_set_glyph_parameters(settings,glyph,
		 glyph_scaling_mode, glyph_centre, glyph_size,
		 orientation_scale_field, glyph_scale_factors,
		 variable_scale_field)))
	{
		cout << "No glyphs defined" << endl;
	}

	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(
				CmguiManager::getInstance().getCmissCommandData());
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	Scene_object* scene_object = Scene_get_Scene_object_by_name(scene, "DataPoints");
	GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(scene_object);
	GT_element_group_add_settings(gt_element_group, settings, 0);
}

ViewerFrame::~ViewerFrame()
{
	delete imageSet_;
}

wxPanel* ViewerFrame::getPanel()
{
	return m_pPanel;
}

void ViewerFrame::TogglePlay(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "button_1", wxButton);
	
	if (animationIsOn)
	{
		Time_keeper_stop(timeKeeper_);
		this->animationIsOn = false;
		button->SetLabel("play");
	}
	else
	{
		Time_keeper_play(timeKeeper_,TIME_KEEPER_PLAY_FORWARD);
		Time_keeper_set_play_loop(timeKeeper_);
		Time_keeper_set_play_every_frame(timeKeeper_);
		this->animationIsOn = true;
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
	//Should just objtain the list of slice names from ImageSet and use that to populate the check list box
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
		imageSet_->SetVisible(name.mb_str(), true);
	}
	else
	{
		imageSet_->SetVisible(name.mb_str(), false);
	}
	
//	RefreshCmguiCanvas(); //Necessary?? - doesn't help with the problem where the canvas doesn't redraw
	this->Refresh();//test to see if this helps with the problem where 3d canvas doesnt update
}

void ViewerFrame::ObjectCheckListSelected(wxCommandEvent& event)
{
	wxString name = objectList_->GetStringSelection();
	const ImagePlane& plane = imageSet_->GetImagePlane(name.mb_str());
	
	// compute the center of the image plane, eye(camera) position and the up vector
	Point3D planeCenter =  0.5 * (plane.trc + plane.blc);
	Point3D eye = planeCenter + (500 * plane.normal); // this seems to determine the near clip plane
	Point3D up(plane.yside);
	NORMALISE(up);
	
	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();	
	if (!Cmiss_scene_viewer_set_lookat_parameters_non_skew(
			sceneViewer, eye.x, eye.y, eye.z,
			planeCenter.x, planeCenter.y, planeCenter.z,
			up.x, up.y, up.z
			))
	{
		//Error;
	}
	
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
	Scene_viewer_redraw_now(sceneViewer);
}

void ViewerFrame::SetTime(double time)
{
	imageSet_->SetTime(time);
	
	wxSlider* slider = XRCCTRL(*this, "slider_1", wxSlider);
	int min = slider->GetMin();
	int max = slider->GetMax();
//	cout << "min = " << min << " ,max = " << max <<endl; 
	slider->SetValue(static_cast<int>(static_cast<double>(max-min)*time) + min);
	
	return;
}

BEGIN_EVENT_TABLE(ViewerFrame, wxFrame)
	EVT_BUTTON(XRCID("button_1"),ViewerFrame::TogglePlay)
	EVT_SLIDER(XRCID("slider_1"),ViewerFrame::OnAnimationSliderEvent) // animation slider
	EVT_SLIDER(XRCID("AnimationSpeedControl"),ViewerFrame::OnAnimationSpeedControlEvent)
	EVT_CHECKLISTBOX(XRCID("SliceList"), ViewerFrame::ObjectCheckListChecked)
	EVT_LISTBOX(XRCID("SliceList"), ViewerFrame::ObjectCheckListSelected)
	EVT_CLOSE(ViewerFrame::Terminate)
END_EVENT_TABLE()
