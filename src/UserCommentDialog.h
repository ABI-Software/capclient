/*
 * UserCommentDialog.h
 *
 *  Created on: Sep 3, 2010
 *      Author: jchu014
 */

#ifndef USERCOMMENTDIALOG_H_
#define USERCOMMENTDIALOG_H_

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <string>

namespace cap
{

class UserCommentDialog : public wxDialog
{
public:
	UserCommentDialog(wxWindow* parent)
//	:
//		wxDialog( parent, -1, wxString("test"))
	{
		wxXmlResource::Get()->Load(wxT("UserCommentDialog.xrc"));
		wxXmlResource::Get()->LoadDialog(this,(wxWindow *)parent, _T("UserCommentDialog"));
		
//		Show(true);
//		Centre();

	}
	
	~UserCommentDialog()
	{
	}

//	void OnClose(wxCloseEvent& event)
//	{
////		Destroy();
//	};
	void OnOKButtonEvent(wxCommandEvent& event)
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

	std::string GetComment() const
	{
		wxTextCtrl* textCtrl = XRCCTRL(*this, "UserComment", wxTextCtrl);
		return std::string(textCtrl->GetValue().mb_str());
	}

	DECLARE_EVENT_TABLE();
	
};

BEGIN_EVENT_TABLE(UserCommentDialog, wxDialog)
	EVT_BUTTON(XRCID("wxID_OK"), UserCommentDialog::OnOKButtonEvent)
//	EVT_BUTTON(XRCID("wxID_CANCEL"), UserCommentDialog::OnCancelButtonEvent)
//	EVT_CLOSE(UserCommentDialog::OnClose)
END_EVENT_TABLE()

} // namespace cap

#endif /* USERCOMMENTDIALOG_H_ */
