/*
 * usercommentdialog.h
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

#include "ui/usercommentdialogui.h"
#include "utils/debug.h"

namespace cap
{
/**
 * Dialog for setting the user comment.  This class should only return wxID_OK if
 * the text ctrl string is not empty.
 */
class UserCommentDialog : public UserCommentDialogUI
{
public:

	/**
	 * Constructor.
	 *
	 * @param [in,out]	parent	If non-null, the parent.
	 */
	UserCommentDialog(wxWindow* parent)
		: UserCommentDialogUI(parent)
	{
		button_ok->Enable(false);
		Center();
		MakeConnections();
	}

	/**
	 * Destructor.
	 */
	~UserCommentDialog()
	{
	}

	/**
	 * Sets the directory for the directory control.
	 *
	 * @param	dirname	Pathname of the directory.
	 */
	void SetDirectory(const std::string& dirname)
	{
		textCtrl_directory->SetValue(dirname.c_str());
	}

	/**
	 * Sets the comment for the user comment widget.
	 * @param comment   The comment.
	 */
	void SetComment(const std::string& comment)
	{
		text_userComment->SetValue(comment.c_str());
	}

	/**
	 * Gets the directory from the directory control.
	 *
	 * @return	The directory.
	 */
	std::string GetDirectory() const
	{
		return std::string(textCtrl_directory->GetValue().c_str());
	}

	/**
	 * Gets the comment, the comment is non-empty if the OK button
	 * has been used to exit the dialog.
	 *
	 * @return	The comment.
	 */
	std::string GetComment() const
	{
		return std::string(text_userComment->GetValue().mb_str());
		//wxTextCtrl* textCtrl = XRCCTRL(*this, "UserComment", wxTextCtrl);
		//return std::string(textCtrl->GetValue().mb_str());
	}

private:

	/**
	 * Executes the ok clicked action.
	 *
	 * @param [in,out]	event	The event.
	 */
	void OnOKClicked(wxCommandEvent& /* event */)
	{
		EndModal(wxID_OK);
	}

	/**
	 * Executes the cancel clicked action.
	 *
	 * @param [in,out]	event	The event.
	 */
	void OnCancelClicked(wxCommandEvent& /* event */)
	{
		EndModal(wxID_CANCEL);
	}

	/**
	 * Executes the directory chooser clicked action.  Raises a directory
	 * dialog to select a dialog.  If OK is returned from the dialog then
	 * the directory text widget will be updated with the new value.
	 *
	 * @param [in,out]	event	The event.
	 */
	void OnDirectoryChooserClicked(wxCommandEvent& /* event */)
	{
		wxDirDialog dirDlg(this, "Choose output directory", textCtrl_directory->GetValue(),
			wxDD_DEFAULT_STYLE);
		if (dirDlg.ShowModal() != wxID_OK)
		{
			return; // Cancelled save
		}
		textCtrl_directory->SetValue(dirDlg.GetPath());
	}

	/**
	 * Executes the text updated action.
	 *
	 * @param [in,out]	event	The event.
	 */
	void OnTextUpdated(wxCommandEvent& event)
	{
		button_ok->Enable(!event.GetString().IsEmpty());
	}

	/**
	 * Make the connections between the widgets and this class.
	 */
	void MakeConnections()
	{
		Connect(button_directoryChooser->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(UserCommentDialog::OnDirectoryChooserClicked));
		Connect(button_ok->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(UserCommentDialog::OnOKClicked));
		Connect(button_cancel->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(UserCommentDialog::OnCancelClicked));
		Connect(text_userComment->GetId(), wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(UserCommentDialog::OnTextUpdated));
	}

};

} // namespace cap

#endif /* USERCOMMENTDIALOG_H_ */
