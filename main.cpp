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

#include "FileSystem.h" //test

using namespace std;

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
		frame->Show(true);

//		wxPanel *panel = frame->getPanel();
//		if (!panel)
//		{
//			printf("panel is null");
//			return 0;
//		}
//		Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().createSceneViewer(panel);

		frame->PopulateObjectList(); //refactor
		
		CAPModelLVPS4X4 heartModel("heart");
		heartModel.ReadModelFromFiles("test");	
		heartModel.SetRenderMode(CAPModelLVPS4X4::WIREFRAME);

//		frame->PopulateObjectList(); //refactor
		
//		Cmiss_scene_viewer_view_all(sceneViewer);
//		Cmiss_scene_viewer_set_perturb_lines(sceneViewer, 1 );
		Cmiss_command_data_main_loop(command_data);//app.OnRun()
		//DESTROY(Cmiss_command_data)(&command_data);
		return 1;
	}

	return 0;
} /* main */
