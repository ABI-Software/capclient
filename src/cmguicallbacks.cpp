

extern "C"
{
	#include <api/cmiss_region.h>
	#include <api/cmiss_time_keeper.h>
	#include <api/cmiss_time.h>
	#include <api/cmiss_field_module.h>
	#include <api/cmiss_field_image.h>
	#include <api/cmiss_interactive_tool.h>
	#include <api/cmiss_field_group.h>
	#include <api/cmiss_field.h>
}

#include "DICOMImage.h"
#include "abstractlabelled.h"
#include "labelledslice.h"
#include "labelledtexture.h"

#include "capclient.h"
#include "cmgui/utilities.h"
#include "utils/debug.h"

#include "cmguicallbacks.h"

namespace cap
{

const int KEYCODE_A = 65;
const int KEYCODE_E = 69;

int input_callback_modelling(Cmiss_scene_viewer_id scene_viewer, 
						  struct Cmiss_scene_viewer_input *input, void *capclientwindow_void)
{
	Cmiss_scene_viewer_input_event_type event_type;
	Cmiss_scene_viewer_input_get_event_type(input, &event_type);
	//dbg("input_callback() : input_type = " + toString(event_type));

	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);

	if (event_type == CMISS_SCENE_VIEWER_INPUT_KEY_PRESS)
	{
		int keyCode = Cmiss_scene_viewer_input_get_key_code(input);
		// dbg("key code : " + toString(keyCode));
		if (keyCode == KEYCODE_E)
		{
			gui->EndCurrentModellingMode();
			return 0;
		}
		if (keyCode == KEYCODE_A)
		{
			gui->OnAccept();
			return 0;
		}
	}

	Cmiss_scene_viewer_input_modifier_flags modifier_flags;
	Cmiss_scene_viewer_input_get_modifier_flags(input, &modifier_flags);

	int modifier_flags_int = static_cast<int>(modifier_flags);
	if (modifier_flags_int & CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL)
	{
		return 1;
	}
	
	// We have to stop the animation when the user clicks on the 3D panel.
	// Since dragging a point while cine is playing can cause a problem
	// But Is this the best place put this code?
	gui->StopCine();
	
	//double time = gui->GetCurrentTime(); // TODO REVISE
	if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS)
	{
		int button_number = Cmiss_scene_viewer_input_get_button_number(input);
		// Select node or create one
		//dbg("Mouse clicked, time = " + toString(time));
		//dbg("Mouse button number = " + toString(button_number));
		
		gui->EnterModellingMode();
		
	}
	//else if (event_type == CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY)
	//{
	//	// Move node		
	//	if (!selectedNode)
	//	{
	//		dbg("GRAPHICS_BUFFER_MOTION_NOTIFY with NULL selectedNode");
	//		//			frame->InitialiseModel();
	//		return 0;
	//	}
	//	Point3D coords;
	//	//		cout << "Mouse Drag node = " << Cmiss_node_get_identifier(selectedNode) << endl;
	//	Cmiss_scene_viewer_id scene_viewer = gui->GetCmissSceneViewer();
	//	Cmiss_move_node_to_screen_coords(scene_viewer, selectedNode, x, y, time, coords);
	//	
	//	//		cout << "Move coord = " << coords << endl;
	//	gui->MoveDataPoint(selectedNode, coords);
	//}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE)
	{
		//dbg("Mouse released");
		//dbg("node location : " + toString(coords));
		Cmiss_node_id selected_node = gui->GetCurrentlySelectedNode();
		if (selected_node)
		{
			Point3D coords = gui->GetNodeRCCoordinates(selected_node);
			gui->AddDataPoint(selected_node, coords);
		}
		//--gui->SmoothAlongTime();
	}
	
	return 1; // returning false means don't call the other input handlers;
}

int input_callback_image_shifting(Cmiss_scene_viewer_id scene_viewer, 
	struct Cmiss_scene_viewer_input *input, void *capclientwindow_void)
{
	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);

	Cmiss_scene_viewer_input_event_type event_type;
	Cmiss_scene_viewer_input_get_event_type(input, &event_type);
	if (event_type == CMISS_SCENE_VIEWER_INPUT_KEY_PRESS)
	{
		int key_code = Cmiss_scene_viewer_input_get_key_code(input);
		if (key_code == KEYCODE_E)
		{
			gui->EndCurrentModellingMode();
			return 0;
		}
	}
	

	Cmiss_scene_viewer_input_modifier_flags modifier_flags;
	Cmiss_scene_viewer_input_get_modifier_flags(input, &modifier_flags);

	int modifier_flags_int = static_cast<int>(modifier_flags);
	if (modifier_flags_int & CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL)
	{
		return 1;
	}
	
	int x = Cmiss_scene_viewer_input_get_x_position(input);
	int y = Cmiss_scene_viewer_input_get_y_position(input);
	
	if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS)
	{
		gui->SetInitialPosition(x, y);
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY)
	{
		gui->UpdatePosition(x, y);
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE)
	{
		gui->SetEndPosition(x, y);
	}
	
	return 1; // returning false means don't call the other input handlers;
}

int time_callback(Cmiss_time_notifier_id time, double current_time, void *capclientwindow_void)
{
	//DEBUG
	//dbg("Time_call_back time = " + toString(current_time));
	
	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);
	gui->SetTime(current_time);
	
	//gui->RedrawNow(); // this forces refresh even when UI is being manipulated by user
	
	return 1;
}

int input_callback_ctrl_modifier_switch(Cmiss_scene_viewer_id scene_viewer, 
	struct Cmiss_scene_viewer_input *input, void *null_void)
{
	Cmiss_scene_viewer_input_modifier_flags modifier_flags;
	Cmiss_scene_viewer_input_get_modifier_flags(input, &modifier_flags);

	int modifier_flags_int = static_cast<int>(modifier_flags);
	if (modifier_flags & CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL)
	{
		modifier_flags_int &= ~CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL;
	}
	else
	{
		modifier_flags_int |= CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL;
	}

	modifier_flags = static_cast<Cmiss_scene_viewer_input_modifier_flags>(modifier_flags_int);

	Cmiss_scene_viewer_input_set_modifier_flags(input, modifier_flags);
	return 1;
}

}