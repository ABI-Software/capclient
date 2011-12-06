


#ifndef LOGWINDOW_H_
#define LOGWINDOW_H_

#include <iostream>
#include <sstream>

//#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include <wx/xrc/xmlres.h>

#include "ui/logdialogui.h"

namespace cap
{
	enum LogLevelEnum 
	{
		LOGERROR,
		LOGWARNING,
		LOGINFORMATION,
		LOGDEBUG
	};

	class LogWindow : public LogDialogUI
	{
	public:
		~LogWindow();
	
		static LogWindow* Logger()
		{
			if (instance_ == 0)
			{
				
				instance_ = new LogWindow();
			}

			return instance_;
		}

		std::ostringstream& Log(LogLevelEnum level);

	private:
		LogWindow();

		void MakeConnections();

		static LogWindow* instance_;
		std::ostringstream oss_;
	};

}

#endif LOGWINDOW_H_

