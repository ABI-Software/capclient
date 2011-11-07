

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

int input_callback_modelling(Cmiss_scene_viewer_id scene_viewer, 
						  struct Cmiss_scene_viewer_input *input, void *capclientwindow_void)
{
	Cmiss_scene_viewer_input_event_type event_type;
	Cmiss_scene_viewer_input_get_event_type(input, &event_type);
	//dbg("input_callback() : input_type = " + toString(event_type));

	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);
	std::string modelling_mode = CAPModeller::ModellingModeStrings.find(gui->GetModellingMode())->second;

	if (event_type == CMISS_SCENE_VIEWER_INPUT_KEY_PRESS)
	{
		int keyCode = Cmiss_scene_viewer_input_get_key_code(input);
		dbg("Key pressed = " + toString(keyCode));
		return 0;
	}

	static Cmiss_node_id selectedNode = 0; // Thread unsafe
	//CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);
	
	// We have to stop the animation when the user clicks on the 3D panel.
	// Since dragging a point while cine is playing can cause a problem
	// But Is this the best place put this code?
	gui->StopCine();
	
	double time = gui->GetCurrentTime(); // TODO REVISE
	if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS)
	{
		int button_number = Cmiss_scene_viewer_input_get_button_number(input);
		// Select node or create one
		dbg("Mouse clicked, time = " + toString(time));
		dbg("Mouse button number = " + toString(button_number));
		
		Cmiss_context_create_region_with_nodes(gui->GetCmissContext(), modelling_mode);
		
		Cmiss_interactive_tool_id i_tool = Cmiss_scene_viewer_get_current_interactive_tool(gui->GetCmissSceneViewer());
		std::string command = "group " + modelling_mode + " coordinate_field coordinates edit create define constrain_to_surfaces";
		Cmiss_interactive_tool_execute_command(i_tool, command.c_str());

		Cmiss_interactive_tool_destroy(&i_tool);
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
		Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(gui->GetCmissContext(), modelling_mode.c_str());
		Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_selection.cmiss_nodes");

		Cmiss_node_iterator_id it = Cmiss_nodeset_create_node_iterator(nodeset);
		Cmiss_node_id selected_node = Cmiss_node_iterator_next(it);
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
		Cmiss_field_cache_set_node(field_cache, selected_node);
		Cmiss_field_id coordinate_field = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
		double values[3];
		Cmiss_field_evaluate_real(coordinate_field, field_cache, 3, values);
	
		Cmiss_field_cache_destroy(&field_cache);
		Cmiss_field_destroy(&coordinate_field);
		Cmiss_node_iterator_destroy(&it);
		Cmiss_nodeset_destroy(&nodeset);
		Cmiss_field_module_destroy(&field_module);

		Point3D coords;
		coords.x = values[0]; coords.y = values[1]; coords.z = values[2];
		dbg("node location : " + toString(coords));
		gui->AddDataPoint(selected_node, coords);
		gui->SmoothAlongTime();
		selectedNode = 0;
	}
	
	return 1; // returning false means don't call the other input handlers;
}

int input_callback_image_shifting(Cmiss_scene_viewer_id scene_viewer, 
										 struct Cmiss_scene_viewer_input *input, void *capclientwindow_void)
{
	Cmiss_scene_viewer_input_event_type event_type;
	Cmiss_scene_viewer_input_get_event_type(input, &event_type);
	//dbg("input_callback_image_shifting() : input_type = " + toString(event_type));
	//	cout << "input_callback_image_shifting() : input_type = " << input->type << endl;
	
	Cmiss_scene_viewer_input_modifier_flags modifier_flags;
	Cmiss_scene_viewer_input_get_modifier_flags(input, &modifier_flags);
	if (!(modifier_flags & CMISS_SCENE_VIEWER_INPUT_MODIFIER_SHIFT))
	{
		return 1;
	}
	
	static Cmiss_node_id selectedNode = 0; // Thread unsafe
	CAPClientWindow* gui = static_cast<CAPClientWindow*>(capclientwindow_void);
	
	double x = static_cast<double>(Cmiss_scene_viewer_input_get_x_position(input));
	double y = static_cast<double>(Cmiss_scene_viewer_input_get_y_position(input));
	
	static double coords[3];
	static Cmiss_region_id selectedRegion;
	if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS)
	{
		// Select node or create one
		int button_number = Cmiss_scene_viewer_input_get_button_number(input);
		dbg("Mouse button number = " + button_number);
		
		Cmiss_scene_viewer_id scene_viewer = gui->GetCmissSceneViewer();
		selectedRegion = Cmiss_get_slice_region(scene_viewer, x, y, (double*)coords, (Cmiss_region_id)0);
		if (selectedRegion)
		{
			std::string sliceName = ""; // Cmiss_region_get_path(selectedRegion);
			std::cout << "selected = " << sliceName << '\n';
		}
		
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY)
	{
		double new_coords[3];
		//Cmiss_region_id selectedRegion = Cmiss_get_slice_region(x, y, (double*)new_coords, selectedRegion);
		if (selectedRegion)
		{
			Cmiss_scene_viewer_id scene_viewer = gui->GetCmissSceneViewer();
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
				//if (Cmiss_node* node = Cmiss_region_get_node(selectedRegion, nodeName))
				{
					double x, y, z;
					//						FE_node_get_position_cartesian(node, 0, &x, &y, &z, 0);
					//					cout << "before = " << x << ", " << y << ", " << z << endl;
					x += (new_coords[0] - coords[0]);
					y += (new_coords[1] - coords[1]);
					z += (new_coords[2] - coords[2]);
					//					cout << "after = " << x << ", " << y << ", " << z << "\n" << endl ;
					//						FE_node_set_position_cartesian(node, 0, x, y, z);
				}
			}
			for (int i = 0; i<3; i++)
			{
				coords[i] = new_coords[i];
			}
		}
	}
	else if (event_type == CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE)
	{
		std::cout << "Mouse released" << '\n';
	}
	
	return 0; // returning false means don't call the other input handlers;
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

}