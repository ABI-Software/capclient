// For compilers that don't support precompilation, include "wx/wx.h";
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include "ViewerFrame.h"

ViewerFrame::ViewerFrame(const wxChar *title, int xpos, int ypos, int width, int height)
    : wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height)), m_pPanel(new wxPanel(this))
{
//  m_pTextCtrl = new wxTextCtrl(this, -1, wxT("Type some text..."),
//                             wxDefaultPosition,  wxDefaultSize, wxTE_MULTILINE);
  //m_pPanel = new wxPanel(this);// -1, wxDefaultPosition,  wxDefaultSize)
  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer( mainSizer );

  // Add MyPanel to the sizer, set the proportion flag to 1 and using the
  //  wxEXPAND flag to ensure that our control will fill entire content of

  // the frame
  mainSizer->Add(m_pPanel, 1, wxEXPAND);


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

  CreateStatusBar(3);
  SetStatusText(wxT("Ready"), 0);
  //Layout();
}

ViewerFrame::~ViewerFrame()
{
}

wxPanel* ViewerFrame::getPanel()
{
	return m_pPanel;
}

void ViewerFrame::Terminate(wxCloseEvent& event)
{
	exit(0); //without this, the funny temporary window appears
}

BEGIN_EVENT_TABLE(ViewerFrame, wxFrame)
	 EVT_CLOSE(ViewerFrame::Terminate)
END_EVENT_TABLE()
