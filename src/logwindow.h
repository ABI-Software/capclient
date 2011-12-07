


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
	//std::ostringstream& Log(LogLevelEnum level);

	class LogWindow : public LogDialogUI
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

	private:
		LogWindow();

		void MakeConnections();
		void OnCloseWindow(wxCloseEvent& event);
		void OnSave(wxCommandEvent& event);
		void OnExit(wxCommandEvent& event);

		static LogWindow* instance_;
		std::string previousSaveLocation_;
	};

	/*class Logger
	{
	public:
		Logger();
		virtual ~Logger();

		std::ostringstream& Log(LogLevelEnum level = LOGINFORMATION);

	private:
		Logger(const Logger&);
		Logger& operator =(const Logger&);

	protected:
		std::ostringstream& oss_;
	};*/

	class Log
	{
	public:
		Log();
		virtual ~Log();
		std::ostringstream& Get(LogLevelEnum level = LOGINFORMATION);

	protected:
		std::ostringstream os;
	private:
		Log(const Log&);
		Log& operator =(const Log&);
	};

	inline Log::Log()
	{
	}

	inline std::ostringstream& Log::Get(LogLevelEnum level)
	{
		os << TimeNow();
		os << " " << logLevelString(level) << ": ";

		return os;
	}

	inline Log::~Log()
	{
		os << std::endl;
		LogWindow::GetInstance()->LogMessage(os.str());
		//fprintf(stderr, "%s", os.str().c_str());
		//fflush(stderr);
	}

}

#endif LOGWINDOW_H_

