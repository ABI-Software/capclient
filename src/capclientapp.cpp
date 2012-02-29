
#include <ostream>

#include "capclientapp.h"

extern "C"
{
#include <zn/cmiss_field_image.h>
}

#include "capclientconfig.h"
#include "dicomimage.h"
#include "labelledslice.h"
#include "capclient.h"
#include "capclientwindow.h"
#include "ui/euladialog.h"
#include "logwindow.h"

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
	//_CrtDumpMemoryLeaks();
	//_CrtSetBreakAlloc(12123);
	//_CrtSetBreakAlloc(91387);
#endif
	wxXmlResource::Get()->InitAllHandlers();
	wxXmlInit_logdialogui();
	/**
	 * TODO: reinstate the end user license agreement.
	 */
	if (!HandleEula())
		return false;
	
	// Create the main application model
	cc_ = CAPClient::GetInstance();
	
	// We cannot initialise this from inside the CAPClientWindow constructor unfortunately.
	wxXmlInit_CAPClientWindowUI();
	CAPClientWindow *frame = new CAPClientWindow(cc_);
	cc_->SetCAPClientWindow(frame);
	SetTopWindow(frame);
	
	return frame->Show(true);
}

int CAPApp::OnExit()
{
	int r = wxApp::OnExit();
	// Clean up anything started in OnInit();
	//CAPClient* mainApp = CAPClient::GetInstance();
	delete cc_;

	LogWindow::GetInstance()->Destroy();
	delete LogWindow::GetInstance();

	return r;
}

}

IMPLEMENT_APP(cap::CAPApp);

