

#include "logmsg.h"

#include "logwindow.h"
#include "utils/debug.h"

namespace cap
{
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;

	Log::~Log()
	{
#ifndef CAP_CLIENT_RELEASE_BUILD
		if (level_ > LOGINFORMATION)
			dbgn(oss_.str());
#endif
		if (rawMsg_)
			LogWindow::GetInstance()->LogMessage(oss_.str());
		else
		{
			oss_ << std::endl;
			LogWindow::GetInstance()->LogMessage(time_, level_, oss_.str());
		}
	}


}

