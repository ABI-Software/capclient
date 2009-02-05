#define UNIX
#define DARWIN

//extern "C" {
//#include <math.h>
//#include <stdio.h>
//#include <general/time.h>
//#include "general/compare.h"
//#include "general/debug.h"
//#include "general/list.h"
//#include "general/indexed_list_private.h"
//#include "general/object.h"
//#include "user_interface/message.h"
//#include "user_interface/event_dispatcher.h"
//}
//
///* After the event_dispatcher.h has set up these variables */
//#include <wx/wx.h>
//#include <wx/apptrait.h>
//extern "C" {
//#include "user_interface/user_interface.h"
//#include "command/cmiss.h"
//}

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
   #include "wx/wx.h"
#endif

#include "ViewerFrame.h"
//#include "SceneViewer.h"

#if defined (DARWIN)
#include <ApplicationServices/ApplicationServices.h>
#endif

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_scene_viewer_private.h"
#include "general/debug.h"
#include "graphics/scene_viewer.h"
#include "graphics/transform_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"
#include "graphics/scene.h"

#include "command/cmiss.h"
#include "general/debug.h"
#include "user_interface/message.h"
}


Cmiss_scene_viewer_id create_Cmiss_scene_viewer_wx(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	wxPanel* panel,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified
<port> window handle.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_Carbon);
	/* Not implemented yet */
//	USE_PARAMETER(minimum_colour_buffer_depth);
//	USE_PARAMETER(minimum_accumulation_buffer_depth);
//	USE_PARAMETER(minimum_depth_buffer_depth);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_wx(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			panel,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth, (Graphics_buffer *)NULL);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_Carbon.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
}

#include "time/time_keeper.h"

int Viewer_view_all(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
{

	int return_code;


	struct Scene* scene = Scene_viewer_get_scene(scene_viewer);

	if (scene_viewer && scene)
	{
		return_code = 1;
		double centre_x,centre_y,centre_z,clip_factor,radius,
			size_x,size_y,size_z,width_factor;

		Scene_get_graphics_range(scene,
			&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z);
		radius = 0.5*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);

		double left, right, bottom, top, near_plane, far_plane;
		if (0 == radius)
		{
			/* get current "radius" from first scene viewer */
			Scene_viewer_get_viewing_volume(scene_viewer,
				&left, &right, &bottom, &top, &near_plane, &far_plane);
			radius = 0.5*(right - left);
		}
		else
		{
			/*???RC width_factor should be read in from defaults file */
			width_factor = 1.05;
			/* enlarge radius to keep image within edge of window */
			radius *= width_factor;
		}

		/*???RC clip_factor should be read in from defaults file: */
		clip_factor = 10.0;

		return_code = Scene_viewer_set_view_simple(
			scene_viewer, centre_x, centre_y, centre_z,
			radius, 40, clip_factor*radius);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_all.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}

int main(int argc,char *argv[])

{
	struct Cmiss_command_data *command_data;


#if defined (DARWIN)
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif

//	initCmgui();
	if (command_data = create_Cmiss_command_data(argc, argv, "0,0"))
	{

//		wxEntry(argc, argv); // wxEntryStart() + CallOnInit() + OnRun();
		// note wxEntryStart has already been invoked while creating user_interface

//		wxCmguiApp &app = wxGetApp();
//		app.CallOnInit(); //create main frame
//		app.OnRun();

		ViewerFrame *frame = new ViewerFrame(
			   wxT("Test Viewer"), 100, 100, 600, 600);
		frame->Show(TRUE);
//		wxGetApp().SetTopWindow(frame);

		wxPanel *panel = frame->getPanel();

		if (!panel)
		{
			printf("panel is null");
			return 0;
		}

		Cmiss_scene_viewer_id sceneViewer = create_Cmiss_scene_viewer_wx(Cmiss_command_data_get_scene_viewer_package(command_data),
				panel,
				CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
				CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
				/*minimum_colour_buffer_depth*/8,
				/*minimum_depth_buffer_depth*/8,
				/*minimum_accumulation_buffer_depth*/8);

		panel->GetContainingSizer()->SetMinSize(600, 600);
		panel->GetContainingSizer()->SetDimension(-1, -1, 600, 600);
		frame->GetSizer()->SetSizeHints(frame);
//		frame->Fit();
//		panel->SetMinSize(wxSize(10,10));

		struct Scene* scene = Scene_viewer_get_scene(sceneViewer);

		struct Time_keeper* time_keeper = Scene_get_default_time_keeper(scene);
//
		Time_keeper_play(time_keeper,TIME_KEEPER_PLAY_FORWARD);

		Viewer_view_all(sceneViewer);

//		Cmiss_scene_viewer_redraw_now(sceneViewer);

		Cmiss_command_data_main_loop(command_data);//app.OnRun()
		//DESTROY(Cmiss_command_data)(&command_data);
		return 1;
	}

	return 0;
} /* main */
