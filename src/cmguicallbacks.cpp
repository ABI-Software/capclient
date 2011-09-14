

#include "cmguicallbacks.h"

extern "C"
{
	#include <api/cmiss_region.h>
	#include <api/cmiss_time_keeper.h>
	#include <api/cmiss_time.h>
	#include <api/cmiss_field_module.h>
}

#include "capclient.h"

namespace cap
{

int input_callback(struct Scene_viewer *scene_viewer, 
						  struct Graphics_buffer_input *input, void *viewer_frame_void)
{
	//	cout << "input_callback() : input_type = " << input->type << endl;
	//	if (input->type == GRAPHICS_BUFFER_KEY_PRESS)
	//	{
		//		int keyCode = input->key_code;
		//		cout << "Key pressed = " << keyCode << endl;
		//		return 0;
		//	}
		
		//if (!(input->input_modifier & GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT))
		{
			return 1;
		}
		
		static Cmiss_node_id selectedNode = 0; // Thread unsafe
		CAPClientWindow* app = static_cast<CAPClientWindow*>(viewer_frame_void);
		
		// We have to stop the animation when the user clicks on the 3D panel.
		// Since dragging a point while cine is playing can cause a problem
		// But Is this the best place put this code?
		app->StopCine();
		
		double x = (double)(0.0); // input->position_x);
		double y = (double)(0.0); // input->position_y);
		double time = app->GetCurrentTime(); // TODO REVISE
		//if (input->type == GRAPHICS_BUFFER_BUTTON_PRESS)
		{
			// Select node or create one
			std::cout << "Mouse clicked, time = " << time << '\n';
			std::cout << "Mouse button number = " << 0 << "\n"; // input->button_number << '\n';
			
			Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
			Point3D coords;
			selectedNode = Cmiss_select_node_from_screen_coords(scene_viewer, x, y, time, coords);
			
			//if (input->button_number == wxMOUSE_BTN_LEFT )
			{	
				if (!selectedNode) //REVISE
				{
					selectedNode = Cmiss_create_or_select_node_from_screen_coords(scene_viewer, x, y, time, coords);
					if (selectedNode != 0) 
					{
						app->AddDataPoint(selectedNode, coords);
					}
				}
			}
			//else if (input->button_number == wxMOUSE_BTN_RIGHT)
			{
				if (selectedNode)
				{
					app->RemoveDataPoint(selectedNode);
					selectedNode = 0;
				}
			}
		}
		//else if (input->type == GRAPHICS_BUFFER_MOTION_NOTIFY)
		{
			// Move node		
			if (!selectedNode)
			{
				std::cout << "GRAPHICS_BUFFER_MOTION_NOTIFY with NULL selectedNode" << '\n';
				//			frame->InitialiseModel();
				return 0;
			}
			Point3D coords;
			//		cout << "Mouse Drag node = " << Cmiss_node_get_identifier(selectedNode) << endl;
			Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
			Cmiss_move_node_to_screen_coords(scene_viewer, selectedNode, x, y, time, coords);
			
			//		cout << "Move coord = " << coords << endl;
			app->MoveDataPoint(selectedNode, coords);
		}
		//else if (input->type == GRAPHICS_BUFFER_BUTTON_RELEASE)
		{
			std::cout << "Mouse released" << '\n';
			app->SmoothAlongTime();
			selectedNode = NULL;
		}
		
		return 0; // returning false means don't call the other input handlers;
}

int input_callback_image_shifting(struct Scene_viewer *scene_viewer, 
										 struct Graphics_buffer_input *input, void *viewer_frame_void)
{
	//	cout << "input_callback_image_shifting() : input_type = " << input->type << endl;
	
	//if (!(input->input_modifier & GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT))
	{
		return 1;
	}
	
	//	static Cmiss_node_id selectedNode = NULL; // Thread unsafe
	//	CAPClientWindow* frame = static_cast<CAPClientWindow*>(viewer_frame_void);
	
	double x = (double)(0.0); // input->position_x);
	double y = (double)(0.0); // (input->position_y);
	
	static double coords[3];
	static Cmiss_region_id selectedRegion;
	//if (input->type == GRAPHICS_BUFFER_BUTTON_PRESS)
	{
		// Select node or create one
		std::cout << "Mouse button number = " << 0 << "\n"; // input->button_number << '\n';
		
		CAPClientWindow* app = static_cast<CAPClientWindow*>(viewer_frame_void);
		Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
		selectedRegion = Cmiss_get_slice_region(scene_viewer, x, y, (double*)coords, (Cmiss_region_id)0);
		if (selectedRegion)
		{
			std::string sliceName = ""; // Cmiss_region_get_path(selectedRegion);
			std::cout << "selected = " << sliceName << '\n';
		}
		
	}
	//else if (input->type == GRAPHICS_BUFFER_MOTION_NOTIFY)
	{
		double new_coords[3];
		//Cmiss_region_id selectedRegion = Cmiss_get_slice_region(x, y, (double*)new_coords, selectedRegion);
		if (selectedRegion)
		{
			CAPClientWindow* app = static_cast<CAPClientWindow*>(viewer_frame_void);
			Cmiss_scene_viewer_id scene_viewer = app->GetCmissSceneViewer();
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
					double_t x, y, z;
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
	//else if (input->type == GRAPHICS_BUFFER_BUTTON_RELEASE)
	{
		std::cout << "Mouse released" << '\n';
	}
	
	return 0; // returning false means don't call the other input handlers;
}

int time_callback(Cmiss_time_notifier_id time, double current_time, void *user_data)
{
	//DEBUG
	std::cout << "Time_call_back time = " << current_time << '\n';
	
	CAPClientWindow* app = static_cast<CAPClientWindow*>(user_data);
	app->SetTime(current_time);
	
	app->RedrawNow(); // this forces refresh even when UI is being manipulated by user
	
	return 0;
}

}