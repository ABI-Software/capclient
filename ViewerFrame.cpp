// For compilers that don't support precompilation, include "wx/wx.h";
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/xrc/xmlres.h"
#include "ViewerFrame.h"

extern "C"
{
#include "time/time_keeper.h"
#include "command/cmiss.h"
}

ViewerFrame::ViewerFrame(const wxChar *title, int xpos, int ypos, int width, int height)
    //: wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height))//, m_pPanel(new wxPanel(this))
{
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ViewerFrame"));
	m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	m_pPanel->GetContainingSizer()->SetMinSize(600, 600);
	m_pPanel->GetContainingSizer()->SetDimension(-1, -1, 600, 600);
	this->GetSizer()->SetSizeHints(this);
#ifdef OLD
//  m_pTextCtrl = new wxTextCtrl(this, -1, wxT("Type some text..."),
//                             wxDefaultPosition,  wxDefaultSize, wxTE_MULTILINE);
  //m_pPanel = new wxPanel(this);// -1, wxDefaultPosition,  wxDefaultSize)
  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer( mainSizer );

  // Add MyPanel to the sizer, set the proportion flag to 1 and using the
  //  wxEXPAND flag to ensure that our control will fill entire content of

  // the frame
  mainSizer->Add(m_pPanel, 1, wxEXPAND);

#ifdef DEL
  m_pMenuBar = new wxMenuBar();
  // File Menu
  m_pFileMenu = new wxMenu();
  m_pFileMenu->Append(wxID_OPEN, _T("&Open"));
  m_pFileMenu->Append(wxID_SAVE, _T("&Save"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_EXIT, _T("&Quit"));
  m_pMenuBar->Append(m_pFileMenu, _T("&File"));
  // About menu
  m_pHelpMenu = new wxMenu();
  m_pHelpMenu->Append(wxID_ABOUT, _T("&About"));
  m_pMenuBar->Append(m_pHelpMenu, _T("&Help"));

  SetMenuBar(m_pMenuBar);
#endif
  CreateStatusBar(3);
  SetStatusText(wxT("Ready"), 0);
  //Layout();
#endif
}

ViewerFrame::ViewerFrame(Cmiss_command_data* command_data_)
	: command_data(command_data_), animationIsOn(false)
{
	time_keeper = Cmiss_command_data_get_default_time_keeper(command_data);
	
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ViewerFrame"));
	m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	m_pPanel->GetContainingSizer()->SetMinSize(1024, 768);
	m_pPanel->GetContainingSizer()->SetDimension(-1, -1, 1024, 768);
	this->GetSizer()->SetSizeHints(this);
}

ViewerFrame::~ViewerFrame()
{
}

wxPanel* ViewerFrame::getPanel()
{
	return m_pPanel;
}

void ViewerFrame::TogglePlay(wxCommandEvent& event)
{
	if (animationIsOn)
	{
		Time_keeper_stop(time_keeper);
		this->animationIsOn = false;
	}
	else
	{
		Time_keeper_play(time_keeper,TIME_KEEPER_PLAY_FORWARD);
		Time_keeper_set_play_loop(time_keeper);
		Time_keeper_set_play_every_frame(time_keeper);
		this->animationIsOn = true;
	}
	
	return;
}

void ViewerFrame::Terminate(wxCloseEvent& event)
{
	exit(0); //without this, the funny temporary window appears
}

BEGIN_EVENT_TABLE(ViewerFrame, wxFrame)
	EVT_BUTTON(XRCID("button_1"),ViewerFrame::TogglePlay)
	EVT_CLOSE(ViewerFrame::Terminate)
END_EVENT_TABLE()
