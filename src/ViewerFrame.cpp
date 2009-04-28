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

static int input_callback(struct Scene_viewer *scene_viewer, 
		struct Graphics_buffer_input *input, void *viewer_frame_void)
{
//	cout << "input_callback()" << endl;

	if (input->type!=GRAPHICS_BUFFER_BUTTON_RELEASE)
	{
		if (input->input_modifier==GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	else if (input->input_modifier!=GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT)
	{
		return 1;
	}
	
//	if (input->type ==GRAPHICS_BUFFER_BUTTON_RELEASE
//			&& input->input_modifier==GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT)
//	{
//		cout << "input->input_modifier==" << input->input_modifier << endl;
//	}
	
	ViewerFrame* frame = static_cast<ViewerFrame*>(viewer_frame_void);
	
	//Get intersection point
	double node_coordinates[3];
	double x = (double)(input->position_x);
	double y = (double)(input->position_y);
	
//	return_code = Cmiss_get_ray_intersection_point(x, y, node_coordinates);
	Cmiss_field_id nearest_element_coordinate_field;
	FE_element* element = Cmiss_get_ray_intersection_point(x, y, node_coordinates, &nearest_element_coordinate_field);
	
	//Create a node 
	if (element)
	{
		Cmiss_region* root_region = Cmiss_command_data_get_root_region(
								CmguiManager::getInstance().getCmissCommandData());
		//Cmiss_region* region;
		//Cmiss_region_get_region_from_path(root_region, "DataPoints", &region);
		
//		Cmiss_region* region =  Computed_field_get_region(nearest_element_coordinate_field);
//		cout << "Element ID =" << Cmiss_element_get_identifier(element) << endl;
		
		Cmiss_region* region = Cmiss_element_get_region(element);
		if (!region)
		{
			cout << "input_callback: Can't get region from element" << endl;
		}
//		if (region == root_region)
//		{
//			cout << "Error:region is root" << endl;
//		}
		cout << "region = " << Cmiss_region_get_path(region) << endl;
		
		Point3D coords(node_coordinates[0], node_coordinates[1], node_coordinates[2]);
		cout << "debug: intersection point = " << coords <<  endl;
				
		if (Cmiss_node_id node = Cmiss_create_node_at_coord(region, nearest_element_coordinate_field, (float*)&coords))
		{
//			return_code = 1;
			frame->AddDataPoint(new DataPoint(node,coords));
		}
	}	
		
//	return return_code;
	return 0; // returning false means don't call the other input handlers;
}

static int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
//	cout << "Time_call_back time = " << current_time << endl;
	
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
//	Cmiss_time_object_id time_object = Cmiss_time_object_create("Texture_animation_time_object");
//	Cmiss_time_object_add_callback(time_object, time_callback, (void*)this);
//	Cmiss_time_keeper_add_time_object(timeKeeper_, time_object);
//	Cmiss_time_object_set_update_frequency(time_object, 10);
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
	//Data point Placing
//	int Scene_viewer_add_input_callback(struct Scene_viewer *scene_viewer,
//		CMISS_CALLBACK_FUNCTION(Scene_viewer_input_callback) *function,
//		void *user_data, int add_first)

#define NODE_CREATION
#ifdef NODE_CREATION
	Scene_viewer_add_input_callback(CmguiManager::getInstance().getSceneViewer(),
			input_callback, (void*)this, 1/*add_first*/);
	
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
	
	Triple new_glyph_size;
	new_glyph_size[0] = 2, new_glyph_size[1] = 2, new_glyph_size[1] = 2;
	
	if (!(GT_element_settings_get_glyph_parameters(settings,
		 &old_glyph, &glyph_scaling_mode ,glyph_centre, glyph_size,
		 &orientation_scale_field, glyph_scale_factors,
		 &variable_scale_field) &&
		GT_element_settings_set_glyph_parameters(settings,glyph,
		 glyph_scaling_mode, glyph_centre, new_glyph_size,
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
#endif //NODE_CREATION
	
}

ViewerFrame::~ViewerFrame()
{
	delete imageSet_;
}

void ViewerFrame::AddDataPoint(DataPoint* dataPoint)
{
//	dataPoints_.push_back(dataPoint);
	modeller_.AddDataPoint(dataPoint);
	
//	//1. Transform to model coordinate
//	const gtMatrix& m = heartModel_.GetLocalToGlobalTransformation();//CAPModelLVPS4X4::
//
//	gtMatrix mInv;
//	inverseMatrix(m, mInv);
////	cout << mInv << endl;
//	transposeMatrix(mInv);// gtMatrix is column Major and our matrix functions assume row major FIX
//	
//	const Point3D& coord = dataPoint->GetCoordinate();
//	Point3D coordLocal = mInv * coord;
//	
//	cout << "Local coord = " << coordLocal << endl;
//	
//	//2. Transform to Prolate Spheroidal
//	float lambda, mu, theta;
//	cartesian_to_prolate_spheroidal(coordLocal.x,coordLocal.y,coordLocal.z, 38.6449, 
//			&lambda,&mu, &theta,0);
//	cout << "lambda: " << lambda << ", mu: " << mu << ", theta: " << theta << endl;
//	
//	//3. Project on to model surface and obtain the material coordinates
//	Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
//	Cmiss_region* cmiss_region;
//	Cmiss_region_get_region_from_path(root_region, "heart", &cmiss_region);
//	
//	Cmiss_field_id field = Cmiss_region_find_field_by_name(cmiss_region, "coordinates");//FIX
//	
//	FE_value point[3], xi[3];
//	point[0] = lambda, point[1] = mu, point[2] = theta;
//	FE_element* element = 0;
//	int return_code = Computed_field_find_element_xi(field,
//		point, /*number_of_values*/3, &element /*FE_element** */, 
//		xi, /*element_dimension*/3, cmiss_region
//		, /*propagate_field*/0, /*find_nearest_location*/1);
//	if (return_code)
//	{
//		cout << "PS xi : " << xi[0] << ", " << xi[1] << ", " << xi[2] << endl;
//		cout << "elem : " << Cmiss_element_get_identifier(element)<< endl;
//	}
//	else
//	{
//		cout << "Can't find xi" << endl;
//	}
//	
//	//Rectangular Cartesian
//	field = Cmiss_region_find_field_by_name(cmiss_region, "heart_rc_coord");//FIX
//	point[0] = coordLocal.x, point[1] = coordLocal.y, point[2] = coordLocal.z;
//	return_code = Computed_field_find_element_xi(field,
//		point, /*number_of_values*/3, &element /*FE_element** */, 
//		xi, /*element_dimension*/3, cmiss_region
//		, /*propagate_field*/0, /*find_nearest_location*/1);
//	if (return_code)
//	{
//		cout << "RC xi : " << xi[0] << ", " << xi[1] << ", " << xi[2] << endl;
//		cout << "elem : " << Cmiss_element_get_identifier(element)<< endl;
//	}
//	else
//	{
//		cout << "Can't find xi" << endl;
//	}
//	
//	//4. Evaluate basis functions at the element coordinate
//	
//	//5. Construct P matrix (add/insert a row)
//	
//	//6. Compute rhs
//	//    p = dataLambda - prior
//	//    rhs = GtPt p
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
		imageSet_->SetVisible(true, name.mb_str());
	}
	else
	{
		imageSet_->SetVisible(false, name.mb_str());
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
		imageSet_->SetVisible(false);
		button->SetLabel("Show All");
	}
	else
	{
		hideAll_ = true;	
		imageSet_->SetVisible(true);
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
				imageSet_->SetVisible(false, i);
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
			imageSet_->SetVisible(true, *itr);
			objectList_->Check(*itr, true);
		}
	
		button->SetLabel("Hide Others");
	}
	

	this->Refresh(); // work around for the refresh bug
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
	heartModel_.SetMIIVisibility(event.IsChecked());
}

void ViewerFrame::OnWireframeCheckBox(wxCommandEvent& event)
{
	heartModel_.SetModelVisibility(event.IsChecked());
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
	EVT_CLOSE(ViewerFrame::Terminate)
END_EVENT_TABLE()
