
#include <ostream>

#include "capclientapp.h"

extern "C"
{
#include <api/cmiss_field_image.h>
}

#include "capclientconfig.h"
#include "DICOMImage.h"
#include "abstractlabelled.h"
#include "labelledslice.h"
#include "labelledtexture.h"
#include "capclient.h"
#include "capclientwindow.h"
#include "ui/euladialog.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

/**
 * \mainpage CAP Client
 * CAP Client provides a visualization tool to browse cardiac MR images and 3D CAP models. Main features include:
 * 
 *   - 3D visualization of cardiac MR images
 *   - Cardiac surface model rendering
 *   - Animation of 3D heart beating
 * 
 * CAP Client is an application being developed at the Auckland Bioengeineering Institute (ABI) as part of the 
 * Cardiac Atlas Project (CAP).  The client side software is responsible for visualising, model fitting
 * and analyzing the cardiac magnetic resonance (CMR) data that reside in the CAP database.
 */ 


namespace cap
{

/**
 * Show the End User License Agreement page
 */
bool HandleEula()
{
	EulaDialog eulaDialog;
	//eulaDialog.Center();
	if (eulaDialog.ShowModal() != wxID_OK)
	{
		return false;
	}
	return true;
}

bool CAPApp::OnInit()
{
#ifdef _MSC_VER
	_CrtDumpMemoryLeaks();
	//_CrtSetBreakAlloc(12123);
	//_CrtSetBreakAlloc(12119);
#endif
	wxXmlResource::Get()->InitAllHandlers();
	
	/**
	 * TODO: reinstate the end user license agreement.
	 */
	//if (!HandleEula())
	//	return false;
	
	// Create the main application model
	CAPClient* mainApp = CAPClient::GetCAPClientInstance();
	
	// We cannot initialise this from inside the CAPClientWindow constructor unfortunately.
	wxXmlInit_CAPClientWindowUI();
	CAPClientWindow *frame = new CAPClientWindow(mainApp);
	mainApp->SetCAPClientWindow(frame);
	SetTopWindow(frame);
	
	return frame->Show(true);
}

int CAPApp::OnExit()
{
	int r = wxApp::OnExit();
	// Clean up anything started in OnInit();
	CAPClient* mainApp = CAPClient::GetCAPClientInstance();
	delete mainApp;

	return r;
}

//void CAPApp::OnInitCmdLine(wxCmdLineParser& parser)
//{
//	dbg(__func__);
//	parser.SetDesc(g_cmdLineDesc);
//	// must refuse '/' as parameter starter or cannot use "/path" style paths
//	parser.SetSwitchChars(wxT("-"));
//}

//bool CAPApp::OnCmdLineParsed(wxCmdLineParser& parser)
//{
//	dbg(__func__);
//	server_mode_ = parser.Found(wxT("s"));
//	
//	// to get at your unnamed parameters use
//	wxArrayString files;
//	for (unsigned int i = 0; i < parser.GetParamCount(); i++)
//	{
//		files.Add(parser.GetParam(i));
//	}
//	
//	return true;
//}

}

IMPLEMENT_APP(cap::CAPApp);

