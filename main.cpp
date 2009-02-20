#define UNIX
#define DARWIN

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
   #include "wx/wx.h"
#endif
#include "wx/xrc/xmlres.h"

#include "Config.h"
#include "ViewerFrame.h"
#include "CAPModelLVPS4X4.h"
#include "DICOMImage.h"
#include "CmguiExtensions.h"
#include "CmguiManager.h"

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

#include "time/time_keeper.h"

#include "api/cmiss_region.h"
#include "finite_element/import_finite_element.h"
#include "api/cmiss_texture.h"
#include "graphics/material.h"
#include "general/manager.h"
#include "finite_element/finite_element.h"
#include "time/time.h"
#include "user_interface/event_dispatcher.h"
}

#include <iostream>

#include "FileSystem.h" //test

using namespace std;

int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
//	cout << "Time_call_back time = " << current_time << endl;
	
	ImageSet* imageSet = reinterpret_cast<ImageSet*>(user_data);
	imageSet->setTime(current_time);
}

int main(int argc,char *argv[])
{
	struct Cmiss_command_data *command_data;


#if defined (DARWIN)
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif

	if (command_data = create_Cmiss_command_data(argc, argv, "0,0"))
	{

		CmguiManager cmguiManager(command_data);
		
		wxXmlResource::Get()->Load("ViewerFrame.xrc");

		ViewerFrame *frame = new ViewerFrame(command_data);
		frame->Show(TRUE);

		wxPanel *panel = frame->getPanel();

		if (!panel)
		{
			printf("panel is null");
			return 0;
		}
		
#define TEXTURE_ANIMATION
#ifdef TEXTURE_ANIMATION
		vector<string> sliceNames;
//		sliceNames.push_back("SA1");
//		sliceNames.push_back("SA2");
//		sliceNames.push_back("SA3");
//		sliceNames.push_back("SA4");
//		sliceNames.push_back("SA5");
//		sliceNames.push_back("SA6");
//		sliceNames.push_back("LA1");
//		sliceNames.push_back("LA2");
		sliceNames.push_back("LA3");
		
		ImageSet imageSet(sliceNames);
				
#define TIME_OBJECT_CALLBACK_TEST
#ifdef TIME_OBJECT_CALLBACK_TEST
		Time_object* time_object = create_Time_object("Texture_animation_timer");
		
		Time_object_add_callback(time_object,time_callback,(void*)&imageSet);
		struct Time_keeper* time_keeper = Cmiss_command_data_get_default_time_keeper(command_data);
		Time_object_set_time_keeper(time_object, time_keeper);
		Time_object_set_update_frequency(time_object,28);//BUG?? doesnt actually update 28 times -> only 27 
#endif		
#endif //TEXTURE_ANIMATION
		
		CAPModelLVPS4X4 heartModel("heart");
		heartModel.readModelFromFiles("test");	
		
		Cmiss_scene_viewer_id sceneViewer = create_Cmiss_scene_viewer_wx(Cmiss_command_data_get_scene_viewer_package(command_data),
				panel,
				CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
				CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
				/*minimum_colour_buffer_depth*/8,
				/*minimum_depth_buffer_depth*/8,
				/*minimum_accumulation_buffer_depth*/8);

//#define HARD_CODED_GUI
#ifdef HARD_CODED_GUI
		panel->GetContainingSizer()->SetMinSize(600, 600);
		panel->GetContainingSizer()->SetDimension(-1, -1, 600, 600);
		frame->GetSizer()->SetSizeHints(frame);		
#endif 
		
		Cmiss_scene_viewer_view_all(sceneViewer);
		
		Cmiss_command_data_main_loop(command_data);//app.OnRun()
		//DESTROY(Cmiss_command_data)(&command_data);
		return 1;
	}

	return 0;
} /* main */
