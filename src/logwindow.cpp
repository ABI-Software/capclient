

#include "logwindow.h"


namespace cap
{
	LogWindow* LogWindow::instance_ = 0;

	LogWindow::LogWindow()
	{
	}

	LogWindow::~LogWindow()
	{
	}

	void MakeConnections()
	{
	}

	std::ostringstream& LogWindow::Log(LogLevelEnum level)
	{
		return oss_;
	}

}