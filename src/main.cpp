extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "command/cmiss.h"
#include "time/time_keeper.h"
#include "time/time.h"
}

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
   #include "wx/wx.h"
#endif
#include "wx/xrc/xmlres.h"

#include "Config.h"
#include "ViewerFrame.h"
#include "CmguiManager.h"

#if defined (DARWIN)
#include <ApplicationServices/ApplicationServices.h>
#endif

#include "CAPTotalLeastSquares.h"
#include <iostream>

using namespace std;

int main(int argc,char *argv[])
{	
#if defined (DARWIN)
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif

//	PerformSVDTest();
//	exit(0);

	const char** cmgui_argv;
	int cmgui_argc;
	
	if (argc == 1) // no command window
	{
		cmgui_argc = 2;
		cmgui_argv = (const char**) calloc(3, sizeof(char*));
		*cmgui_argv = strdup(argv[0]);
		*(cmgui_argv +1) = strdup("-server");
		*(cmgui_argv +2) = 0;
	}
	else if (argc == 2 && (strcmp(argv[1],"-debug") == 0))
	{
		cmgui_argc = 1;
		cmgui_argv = (const char**) calloc(2, sizeof(char*));
		*cmgui_argv = strdup(argv[0]);
		*(cmgui_argv + 1) = 0;
	}
	else
	{
		cout << "Error: Invalid command line options" << endl;
		return (0);
	}
	
//	TestLSQR();
//	exit(0); 

	if (Cmiss_context_id context = Cmiss_context_create("CAPClient"))
	{

		int ret = Cmiss_context_enable_user_interface(context, cmgui_argc, cmgui_argv);
		if (ret)
		{
			cap::CmguiManager cmguiManager(context);
			
			Cmiss_context_execute_command(context, "gfx"); //HACK until cmgui is fixed
			
			wxXmlResource::Get()->Load("ViewerFrame.xrc");
	
			cap::ViewerFrame *frame = new cap::ViewerFrame(context);
			frame->Show(true);
	
			Cmiss_context_run_main_loop(context);//app.OnRun()
			
			delete frame;
		}
		Cmiss_context_destroy(&context);
	}

	// Free the memory from the strdup's
	const char** string_ptr = cmgui_argv;
	while (*string_ptr)
	{
		free((void*)*string_ptr);
		string_ptr++;
	}
	free(cmgui_argv);
	
	return 0;
} /* main */
