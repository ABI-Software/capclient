


#ifndef LOGWINDOW_H_
#define LOGWINDOW_H_

#include <iostream>
#include <sstream>

//#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include <wx/xrc/xmlres.h>

#include "ui/logdialogui.h"
#include "utils/time.h"
#include "utils/debug.h"

namespace cap
{
	enum LogLevelEnum 
	{
		LOGERROR,
		LOGWARNING,
		LOGINFORMATION,
		LOGDEBUG
	};

	std::string logLevelString(LogLevelEnum level);

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

	class Log
	{
	public:
		Log();
		virtual ~Log();
		std::ostringstream& Get(LogLevelEnum level = LOGINFORMATION);
		static LogLevelEnum ReportingLevel() { return reportingLevel_; }

	protected:
		std::ostringstream oss_;
		std::string time_;
		LogLevelEnum level_;
		static LogLevelEnum reportingLevel_;

	private:
		Log(const Log&);
		Log& operator =(const Log&);
	};

	inline Log::Log()
		: time_()
		, level_()
	{
#if defined CAP_CLIENT_RELEASE_BUILD
		reportingLevel_ = LOGINFORMATION;
#endif
	}

	inline std::ostringstream& Log::Get(LogLevelEnum level)
	{
		time_ = TimeNow();
		level_ = level;

		return oss_;
	}

	inline Log::~Log()
	{
		oss_ << std::endl;
		LogWindow::GetInstance()->LogMessage(time_, level_, oss_.str());
		dbgn(oss_.str());
	}

	#define LOG_MSG(level) \
		if (level > Log::ReportingLevel()) ; \
		else Log().Get(level)


}

#endif LOGWINDOW_H_

