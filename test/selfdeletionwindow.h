
#ifndef SELFDELETIONWINDOW_H_
#define SELFDELETIONWINDOW_H_

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>

#include "utils/debug.h"

#include "ui/selfdeletionwindowui.h"

namespace cap
{
	class SelfDeletion;

	class SelfDeletionWindow : public SelfDeletionWindowUI
	{
	public:
		SelfDeletionWindow(SelfDeletion *sd)
			: sd_(sd)
		{
			Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(SelfDeletionWindow::OnCloseWindow));
			Connect(button_Cancel_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SelfDeletionWindow::OnCancel));
			Connect(button_OK_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(SelfDeletionWindow::OnOK));
		}

		void OnCloseWindow(wxCloseEvent& event);

		void OnOK(wxCommandEvent& event);

		void OnCancel(wxCommandEvent& event);

		~SelfDeletionWindow()
		{
			dbg("~SelfDeletionWindow()");
		}

		SelfDeletion *sd_;
	};
}

#endif /* SELFDELETIONWINDOW_H_ */

