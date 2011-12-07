

#ifndef LOGMSG_H_
#define LOGMSG_H_

#include <iostream>
#include <sstream>

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

	class Log
	{
	public:
		Log();
		~Log();
		std::ostringstream& Msg(LogLevelEnum level);
		std::ostringstream& Msg();
		static LogLevelEnum ReportingLevel() { return reportingLevel_; }

	private:
		std::ostringstream oss_;
		std::string time_;
		LogLevelEnum level_;
		bool rawMsg_;
		static LogLevelEnum reportingLevel_;

	private:
		Log(const Log&);
		Log& operator =(const Log&);
	};

	inline Log::Log()
		: time_()
		, level_(LOGINFORMATION)
		, rawMsg_(false)
	{
#if defined CAP_CLIENT_RELEASE_BUILD
		reportingLevel_ = LOGINFORMATION;
#endif
	}

	inline std::ostringstream& Log::Msg(LogLevelEnum level)
	{
		time_ = TimeNow();
		level_ = level;

		return oss_;
	}

	inline std::ostringstream& Log::Msg()
	{
		rawMsg_ = true;

		return oss_;
	}

	#define LOG_MSG(level) \
		if (level > Log::ReportingLevel()) ; \
		else Log().Msg(level)

	#define LOG_MSG_RAW(level) \
		if (level > Log::ReportingLevel()) ; \
		else Log().Msg()

}


#endif /* LOGMSG_H_ */

