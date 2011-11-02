
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
#include "CAPEulaDialog.h"

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
	CAPEulaDialog eulaDialog;
	eulaDialog.Center();
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
	//_CrtSetBreakAlloc(11782);
#endif
	std::cout << "CAPApp::" << __func__ << std::endl;
	wxXmlResource::Get()->InitAllHandlers();
	
	/**
	 * TODO: reinstate the end user license agreement.
	 */
	//if (!HandleEula())
	//	return false;
	
	// Create the main application model
	CAPClient* mainApp = CAPClient::CreateCAPClient();
	
	assert(mainApp);
	// We cannot initialise this from inside the CAPClientWindow constructor unfortunately.
	wxXmlInit_CAPClientWindowUI();
	CAPClientWindow *frame = new CAPClientWindow(mainApp);
	mainApp->SetCAPClientWindow(frame);
	SetTopWindow(frame);
	frame->Show(true);
	
	return true;
}

int CAPApp::OnExit()
{
	std::cout << "CAPApp::" << __func__ << std::endl;
	// Clean up anything started in OnInit();
	return 0;
}

void CAPApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	std::cout << __func__ << std::endl;
	parser.SetDesc(g_cmdLineDesc);
	// must refuse '/' as parameter starter or cannot use "/path" style paths
	parser.SetSwitchChars(wxT("-"));
}

bool CAPApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	std::cout << __func__ << std::endl;
	server_mode_ = parser.Found(wxT("s"));
	
	// to get at your unnamed parameters use
	wxArrayString files;
	for (unsigned int i = 0; i < parser.GetParamCount(); i++)
	{
		files.Add(parser.GetParam(i));
	}
	
	return true;
}

}

IMPLEMENT_APP(cap::CAPApp);

