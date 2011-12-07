

#include "logwindow.h"

#include "utils/filesystem.h"

namespace cap
{
	LogWindow* LogWindow::instance_ = 0;

	LogWindow::LogWindow()
		: previousSaveLocation_("")
	{
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
		std::string dirpath = "";//FileSystem::GetPath(previousSaveLocation_);

		wxFileDialog dialog(this, "Select a file", dirpath, filename, "*.*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		dialog.SetPath(previousSaveLocation_);
		dialog.Center();

		if (dialog.ShowModal() == wxID_OK)
		{
			previousSaveLocation_ = dialog.GetPath();
			std::cout << previousSaveLocation_ << std::endl;
		}
	}

	void LogWindow::OnExit(wxCommandEvent& event)
	{
		Hide();
	}

	void LogWindow::LogMessage(const std::string& message)
	{
		html_log_->AppendToPage(message);
	}

	//std::ostringstream& Log(LogLevelEnum level)
	//{
	//	//Logger log;
	//	//log.Log(level);
	//	return //log.Log(level);
	//}

	std::string logLevelString(LogLevelEnum level)
	{
		static const char* const buffer[] = {"ERROR", "WARNING", "INFORMATION", "DEBUG"};
		return buffer[level];
	}

	//Logger::Logger()
	//{
	//}
	//
	//Logger::~Logger()
	//{
	//	oss_ << std::endl;
	//	LogWindow::GetInstance()->LogMessage(oss_.str());
	//}

	//std::ostringstream& Logger::Log(LogLevelEnum level)
	//{
	//	oss_ << TimeNow();
	//	oss_ << " " << logLevelString(level) << ": ";

	//	return oss_;
	//}


}