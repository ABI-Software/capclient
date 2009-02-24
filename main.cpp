//#define UNIX
//#define DARWIN

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
#include "command/cmiss.h"
#include "time/time_keeper.h"
#include "time/time.h"
}

#include <iostream>


using namespace std;

int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
	cout << "Time_call_back time = " << current_time << endl;

	ImageSet* imageSet = reinterpret_cast<ImageSet*>(user_data);
	imageSet->setTime(current_time);
	
//	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();
//	Scene_viewer_redraw(sceneViewer);
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
		
		Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().createSceneViewer(panel);

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
		//Time_object_set_update_frequency(time_object,28);//BUG?? doesnt actually update 28 times -> only 27
		
		Time_keeper_set_minimum(time_keeper, 0);
		Time_keeper_set_maximum(time_keeper, 1);
		
#endif		
#endif //TEXTURE_ANIMATION

		CAPModelLVPS4X4 heartModel("heart");
		heartModel.readModelFromFiles("test");	
		
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
