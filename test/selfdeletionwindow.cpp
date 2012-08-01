
#include "selfdeletionwindow.h"
#include "selfdeletion.h"

namespace cap
{
	void SelfDeletionWindow::OnCloseWindow(wxCloseEvent& event)
	{
		dbg("OnCloseWindow");
		sd_->OnCancel();
	}

	void SelfDeletionWindow::OnOK(wxCommandEvent& event)
	{
		dbg("SelfDeletionWindow::OnOK");
		sd_->OnOK();
	}

	void SelfDeletionWindow::OnCancel(wxCommandEvent& event)
	{
		dbg("SelfDeletionWindow::OnCancel");
		sd_->OnCancel();
	}
}

