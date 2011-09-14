
#include <ostream>

#include "capclientapp.h"

#include "capclient.h"
#include "capclientwindow.h"
#include "CAPEulaDialog.h"

namespace cap
{

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
	std::cout << "CAPApp::" << __func__ << std::endl;
	wxXmlResource::Get()->InitAllHandlers();
	
	//if (!HandleEula())
	//	return false;
	
	// Create the main application model
	CAPClient* mainApp = CAPClient::CreateCAPClient();
	
	assert(mainApp);
	// We cannot initialise this from inside the CAPClientWindow constructor unfortunately.
	wxXmlInit_CAPClientWindowUI(); 
	CAPClientWindow *frame = new CAPClientWindow(dynamic_cast<wxWindow *>(this), mainApp);
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

