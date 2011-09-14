

// #include "Config.h"
// #include "capclient.h"
// #include "capclient.h"
// //#include "CAPEulaDialog.h"
// #include "cmguipanel.h"
// 
// #if defined (DARWIN)
// #include <ApplicationServices/ApplicationServices.h>
// #endif
// 
// #include <iostream>

// using namespace std;
// /*
// bool HandleEula()
// {
// 	cap::CAPEulaDialog eulaDialog;	
// 	eulaDialog.Center();
// 	if (eulaDialog.ShowModal() != wxID_OK)
// 	{
// 		return false;
// 	}
// 	return true;
// }*/
// 
// //#include "AnnotationWindow.h"
// //#include "AnnotationEditor.h"
// 
// int main(int argc,char *argv[])
// {	
// #if defined (DARWIN)
// 	ProcessSerialNumber PSN;
// 	GetCurrentProcess(&PSN);
// 	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
// #endif
// 	
// 	const char** cmgui_argv;
// 	int cmgui_argc;
// 	
// 	if (argc == 1) // no command window
// 	{
// 		cmgui_argc = 2;
// 		cmgui_argv = (const char**) calloc(3, sizeof(char*));
// 		*cmgui_argv = strdup(argv[0]);
// 		*(cmgui_argv +1) = strdup("-server");
// 		*(cmgui_argv +2) = 0;
// 	}
// 	else if (argc == 2 && (strcmp(argv[1],"-debug") == 0))
// 	{
// 		cmgui_argc = 1;
// 		cmgui_argv = (const char**) calloc(2, sizeof(char*));
// 		*cmgui_argv = strdup(argv[0]);
// 		*(cmgui_argv + 1) = 0;
// 	}
// 	else
// 	{
// 		cout << "Error: Invalid command line options" << endl;
// 		return (1);
// 	}
// 
// 	Cmiss_context_id context = Cmiss_context_create("CAPClient");
// 	if (context != 0)
// 	{
// 		cap::CmguiPanel *manager = new cap::CmguiPanel(context);
// 		//using namespace cap;
// 		cap::CAPWindow *app = new cap::CAPWindow();
// 		//CAPClient* app = CAPClient::getCAPClient();
// 		wxApp::SetInstance(app);
// 		wxEntryStart(argc, argv);
// 		
// 		int ret = Cmiss_context_enable_user_interface(context, cmgui_argc, cmgui_argv, static_cast<void*>(app));
// 		//Cmiss_context_execute_command(context, "gfx"); //HACK until cmgui is fixed
// 		if (ret)
// 		{
// 			
// 			
// 			
// 			/**
// 			 * TODO: Must revisit this.
// 			 */
// 			//if (!HandleEula())
// 			//	return 0;
// 			
// 			app->OnInit();
// 			app->OnRun();
// 			//Cmiss_context_run_main_loop(context);//app.OnRun()
// 			app->OnExit();
// 			wxEntryCleanup();
// 		}
// 		delete manager;
// 		Cmiss_context_destroy(&context);
// 	}
// 	else
// 	{
// 		cout << "Error: Failed to create cmiss context" << endl;
// 		return (2);
// 	}
// 
// 	// Free the memory from the strdup's
// 	const char** string_ptr = cmgui_argv;
// 	while (*string_ptr)
// 	{
// 		free((void*)*string_ptr);
// 		string_ptr++;
// 	}
// 	free(cmgui_argv);
// 	
// 	return 0;
// } // main 
