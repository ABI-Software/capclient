#include "wx/wxprec.h"

#ifndef WX_PRECOMP
   #include "wx/wx.h"
#endif
#include "wx/xrc/xmlres.h"

#include "Config.h"
#include "ViewerFrame.h"
//#include "CAPModelLVPS4X4.h"
//#include "DICOMImage.h"
//#include "CmguiExtensions.h"
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

int main(int argc,char *argv[])
{	
#if defined (DARWIN)
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif


	char** cmgui_argv;
	int cmgui_argc;
	
	if (argc == 1) // no command window
	{
		cmgui_argc = 2;
		cmgui_argv = (char**) calloc(3, sizeof(char*));
		*cmgui_argv = strdup(argv[0]);
		*(cmgui_argv +1) = strdup("-server");
		*(cmgui_argv +2) = 0;
	}
	else if (argc == 2 && (strcmp(argv[1],"-debug") == 0))
	{
		cmgui_argc = 1;
		cmgui_argv = (char**) calloc(2, sizeof(char*));
		*cmgui_argv = strdup(argv[0]);
		*(cmgui_argv + 1) = 0;
	}
	else
	{
		cout << "Error: Invalid command line options" << endl;
		return (0);
	}

	if (struct Cmiss_command_data *command_data = create_Cmiss_command_data(cmgui_argc, cmgui_argv, "0.0"))
	{

		CmguiManager cmguiManager(command_data);
		
		wxXmlResource::Get()->Load("ViewerFrame.xrc");

		ViewerFrame *frame = new ViewerFrame(command_data);
		frame->Show(true);

		Cmiss_command_data_main_loop(command_data);//app.OnRun()
		
		delete frame;
		destroy_Cmiss_command_data(&command_data);
	}

	// Free the memory from the strdup's
	char** string_ptr = cmgui_argv;
	while (*string_ptr)
	{
		free(*string_ptr);
		string_ptr++;
	}
	free(cmgui_argv);
	
	return 0;
} /* main */
