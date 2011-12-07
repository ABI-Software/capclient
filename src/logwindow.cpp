

#include "logwindow.h"

#include <iostream>
#include <fstream>

#include "images/capicon.xpm"
#include "utils/filesystem.h"

namespace cap
{
	LogWindow* LogWindow::instance_ = 0;
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;

	LogWindow::LogWindow()
		: previousSaveLocation_("")
	{
		SetSize(800, 450);
		SetIcon(wxICON(capicon));
		text_log_->HideNativeCaret();
		MakeConnections();
	}

	LogWindow::~LogWindow()
	{
	}

	void LogWindow::MakeConnections()
	{
		Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LogWindow::OnCloseWindow));
		Connect(button_exit_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LogWindow::OnExit));
		Connect(button_save_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LogWindow::OnSave));
	}

	void LogWindow::OnCloseWindow(wxCloseEvent& event)
	{
		Hide();
	}

	void LogWindow::OnSave(wxCommandEvent& event)
	{
		if (previousSaveLocation_.length() == 0)
			previousSaveLocation_ = wxGetCwd();

		std::string filename = FileSystem::GetFileName(previousSaveLocation_);
		std::string dirpath = FileSystem::GetPath(previousSaveLocation_);

		wxFileDialog dialog(this, "Select a file", dirpath, filename, "*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		dialog.SetPath(previousSaveLocation_);
		dialog.Center();

		if (dialog.ShowModal() == wxID_OK)
		{
			previousSaveLocation_ = dialog.GetPath();
			std::cout << previousSaveLocation_ << std::endl;
			std::ofstream file;
			file.open(previousSaveLocation_.c_str(), std::ios::out);
			file << text_log_->GetValue();
		}
	}

	void LogWindow::OnExit(wxCommandEvent& event)
	{
		Hide();
	}

	void LogWindow::LogMessage(const std::string& message)
	{
		wxTextAttr red = wxTextAttr(wxColour("red"));
		text_log_->SetStyle(0, 20, red);
		text_log_->AppendText(message);
	}

	void LogWindow::LogMessage(const std::string& time, LogLevelEnum level, const std::string& message)
	{
		long p1, p2;
		wxTextAttr black = wxTextAttr(wxColour("black"));
		wxTextAttr red = wxTextAttr(wxColour(205, 0, 0));
		wxTextAttr green = wxTextAttr(wxColour(34, 139, 34));
		wxTextAttr blue = wxTextAttr(wxColour(0, 0, 205));
		wxTextAttr orange = wxTextAttr(wxColour(255, 140, 0));
		wxTextAttr logLevelAttr = black;
		p1 = text_log_->GetInsertionPoint();
		text_log_->AppendText(time + "   ");
		p2 = text_log_->GetInsertionPoint();
		text_log_->SetStyle(p1, p2, green);
		p1 = text_log_->GetInsertionPoint();
		text_log_->AppendText(logLevelString(level) + ": ");
		p2 = text_log_->GetInsertionPoint();
		switch (level)
		{
		case LOGERROR:
			logLevelAttr = red;
			break;
		case LOGWARNING:
			logLevelAttr = orange;
			break;
		case LOGINFORMATION:
			logLevelAttr = blue;
			break;
		}
		text_log_->SetStyle(p1, p2, logLevelAttr);
		p1 = text_log_->GetInsertionPoint();
		text_log_->AppendText(message);
		p2 = text_log_->GetInsertionPoint();
		text_log_->SetStyle(p1, p2, black);

	}

	std::string logLevelString(LogLevelEnum level)
	{
		static const char* const buffer[] = {"ERROR", "WARNING", "INFORMATION", "DEBUG"};
		return buffer[level];
	}

}