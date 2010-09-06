/*
 * CAPEulaDialog.h
 *
 *  Created on: Sep 6, 2010
 *      Author: jchu014
 */

#ifndef CAPEULADIALOG_H_
#define CAPEULADIALOG_H_

#include "CAPHtmlWindow.h"

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include "wx/xrc/xmlres.h"

#include <string>

namespace cap
{

class CAPEulaDialog : public wxDialog
{
public:
	CAPEulaDialog()
	:
		wxDialog( 0, wxID_ANY, wxString(_("End User License Agreement")))
	{		
		wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
		
		wxHtmlWindow* html = new CAPHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(600, 400));
		html->SetBorders(0);
		html->LoadPage(wxT("Data/HTML/AboutCAPClient.html"));
		
		topsizer->Add(html, 1, wxALL, 10);
		
		wxBoxSizer* buttonsizer = new wxBoxSizer(wxHORIZONTAL);
		wxButton *bu1 = new wxButton(this, wxID_OK, _("Accept"));
//		bu1 -> SetDefault();
		wxButton *bu2 = new wxButton(this, wxID_CANCEL, _("Cancel"));
//		bu1 -> SetDefault();
				
		buttonsizer->Add(bu1, 0, wxALL | wxALIGN_RIGHT, 5);
		buttonsizer->Add(bu2, 0, wxALL | wxALIGN_RIGHT, 5);
		
		topsizer->Add(buttonsizer, 0, wxALL | wxALIGN_RIGHT, 5);
		
		SetSizer(topsizer);
		topsizer->Fit(this);
		
//		Center();
//		ShowModal();
	}

//	void OnClose(wxCloseEvent& event)
//	{
////		Destroy();
//	};
	void OnAcceptButtonEvent(wxCommandEvent& event)
	{
		if ( IsModal() )
		{
			EndModal(wxID_OK); // If modal
		}
		else
		{
			SetReturnCode(wxID_OK);
			this->Show(false); // If modeless
		}
	};
//	void OnCancelButtonEvent(wxCommandEvent& event)
//	{
//		Close();
//	};

	DECLARE_EVENT_TABLE();
	
};

BEGIN_EVENT_TABLE(CAPEulaDialog, wxDialog)
	EVT_BUTTON(XRCID("wxID_OK"), CAPEulaDialog::OnAcceptButtonEvent)
END_EVENT_TABLE()

} // namespace cap

#endif /* CAPEULADIALOG_H_ */
