/*
 * euladialog.h
 *
 *  Created on: Sep 6, 2010
 *      Author: jchu014
 */

#ifndef UI_EULADIALOG_H_
#define UI_EULADIALOG_H_

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include "wx/xrc/xmlres.h"

#include <string>

#include "ui/htmlwindow.h"

namespace cap
{

class EulaDialog : public wxDialog
{
public:
	EulaDialog()
		: wxDialog( 0, wxID_ANY, wxString(_("End User License Agreement")), wxDefaultPosition, wxDefaultSize, -1)
	{		
		wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
		
		wxHtmlWindow* html = new HtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(800, 600));
		//html->LoadPage(wxT("Data/HTML/AboutCAPClient.html"));
		
		topsizer->Add(html, 1, wxALL, 10);
		
		wxBoxSizer* buttonsizer = new wxBoxSizer(wxHORIZONTAL);
		wxButton *bu1 = new wxButton(this, wxID_OK, _("Accept"));
//		bu1 -> SetDefault();
		wxButton *bu2 = new wxButton(this, wxID_CANCEL, _("Cancel"));
				
		buttonsizer->Add(bu1, 0, wxALL, 5);
		buttonsizer->Add(bu2, 0, wxALL, 5);
		
		topsizer->Add(buttonsizer, 0, wxALL | wxALIGN_RIGHT, 5);
		
		SetSizer(topsizer);
		topsizer->Fit(this);
	}

	void OnAcceptButtonEvent(wxCommandEvent& WXUNUSED(event))
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

	DECLARE_EVENT_TABLE();
	
};

BEGIN_EVENT_TABLE(EulaDialog, wxDialog)
	EVT_BUTTON(XRCID("wxID_OK"), EulaDialog::OnAcceptButtonEvent)
END_EVENT_TABLE()

} // namespace cap

#endif /* UI_EULADIALOG_H_ */
