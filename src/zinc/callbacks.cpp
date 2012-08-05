

extern "C"
{
	#include <zn/cmiss_region.h>
	#include <zn/cmiss_time_keeper.h>
	#include <zn/cmiss_time.h>
	#include <zn/cmiss_field_module.h>
	#include <zn/cmiss_field_image.h>
	#include <zn/cmiss_interactive_tool.h>
	#include <zn/cmiss_field_group.h>
	#include <zn/cmiss_field.h>
}

#include "dicomimage.h"
#include "labelledslice.h"

#include "capclientwindow.h"
#include "zinc/extensions.h"
#include "utils/debug.h"
#include "utils/misc.h"

#include "zinc/callbacks.h"

namespace cap
{

const int KEYCODE_A = 65;
const int KEYCODE_D = 68;
const int KEYCODE_E = 69;
const int KEYCODE_Z = 90;

int input_callback_modelling_setup(Cmiss_scene_viewer_id /*scene_viewer*/,
	struct Cmiss_scene_viewer_input *input, void *capclientwindow_void)
{
	Cmiss_scene_viewer_input_event_type event_type;
	Cmiss_scene_viewer_input_get_event_type(input, &event_type);

	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);

	if (event_type == CMISS_SCENE_VIEWER_INPUT_KEY_PRESS)
	{
		int keyCode = Cmiss_scene_viewer_input_get_key_code(input);
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
		if (keyCode == KEYCODE_D)
		{
			gui->DeleteCurrentlySelectedNode();
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

	if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS)
	{
		gui->StartModellingAction();
	}

	return 1;
}

int input_callback_modelling(Cmiss_scene_viewer_id /*scene_viewer*/,
	struct Cmiss_scene_viewer_input *input, void *capclientwindow_void)
{
	Cmiss_scene_viewer_input_event_type event_type;
	Cmiss_scene_viewer_input_get_event_type(input, &event_type);

	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);

	Cmiss_scene_viewer_input_modifier_flags modifier_flags;
	Cmiss_scene_viewer_input_get_modifier_flags(input, &modifier_flags);
	bool modellingActive = gui->IsModellingActive();

	int modifier_flags_int = static_cast<int>(modifier_flags);
	if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS
			&& !(modifier_flags_int & CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL))
	{
		gui->AddCurrentlySelectedNode();
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY
			 && modellingActive)
	{
		gui->MoveCurrentlySelectedNode();
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE
			 && modellingActive)
	{
		gui->AttachCurrentlySelectedNode();
		gui->EndModellingAction();
	}

	return 1; // returning false means don't call the other input handlers;
}

int input_callback_image_shifting(Cmiss_scene_viewer_id /* scene_viewer */,
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
		if (key_code == KEYCODE_Z)
		{
			gui->SetZKeyDown(true);
		}
	}
	if (event_type == CMISS_SCENE_VIEWER_INPUT_KEY_RELEASE)
	{
		int key_code = Cmiss_scene_viewer_input_get_key_code(input);
		if (key_code == KEYCODE_Z)
		{
			gui->SetZKeyDown(false);
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
	Point3D pos(x, y, 0.0);
	if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS)
	{
		gui->SetInitialPosition(pos);
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY)
	{
		gui->UpdatePosition(pos);
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE)
	{
		gui->SetEndPosition(pos);
	}

	return 1; // returning false means don't call the other input handlers;
}

int time_callback(Cmiss_time_notifier_id /*time*/, double current_time, void *capclientwindow_void)
{
	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);
	gui->SetTime(current_time);

	return 1;
}

int input_callback_ctrl_modifier_switch(Cmiss_scene_viewer_id /*scene_viewer*/,
	struct Cmiss_scene_viewer_input *input, void * /*null_void*/)
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
