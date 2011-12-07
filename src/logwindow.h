


#ifndef LOGWINDOW_H_
#define LOGWINDOW_H_

#include <string>

#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include <wx/xrc/xmlres.h>

#include "ui/logdialogui.h"
#include "logmsg.h"

namespace cap
{

	class LogWindow : public LogWindowUI
	{
	public:
		~LogWindow();
	
		static LogWindow* GetInstance()
		{
			if (instance_ == 0)
			{
				
				instance_ = new LogWindow();
			}

			return instance_;
		}

		void LogMessage(const std::string& message);
		void LogMessage(const std::string& time, LogLevelEnum level, const std::string& message);

	private:
		LogWindow();

		void MakeConnections();
		void OnCloseWindow(wxCloseEvent& event);
		void OnSave(wxCommandEvent& event);
		void OnExit(wxCommandEvent& event);

		static LogWindow* instance_;
		std::string previousSaveLocation_;
	};


}

#endif LOGWINDOW_H_

