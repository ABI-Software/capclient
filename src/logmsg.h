

#ifndef LOGMSG_H_
#define LOGMSG_H_

#include <iostream>
#include <sstream>

#include "utils/misc.h"

namespace cap
{
	/**
	 * Values that represent LogLevelEnum.
	 */
	enum LogLevelEnum 
	{
		LOGERROR,
		LOGWARNING,
		LOGINFORMATION,
		LOGDEBUG
	};

	/**
	 * Converts a LogLevelEnum to it's string representation.
	 *
	 * @param	level	The level.
	 *
	 * @return	String representation of the given level.
	 */
	std::string logLevelString(LogLevelEnum level);

	/**
	 * The log class is for sending messages to the LogWindow.  It could be set up to send the
	 * messages anywhere but for now it is sending messages to the LogWindow.  The Log class is not
	 * designed for use directly but through the macros LOG_MSG and LOG_MSG_RAW.  These macros
	 * create a local Log object that is destroyed when it goes out of scope.  When the Log object
	 * calls it's destructor the message contained in the Log object will be sent to the LogWindow.
	 * 
	 * @see LogWindow
	 */
	class Log
	{
	public:

		/**
		 * Default constructor.
		 */
		Log();

		/**
		 * Destructor.
		 */
		~Log();

		/**
		 * Gets message stream and sets it's level.
		 *
		 * @param	level	The level.
		 *
		 * @return	The stream.
		 */
		std::ostringstream& Msg(LogLevelEnum level);

		/**
		 * Gets the message stream.
		 *
		 * @return	The stream.
		 */
		std::ostringstream& Msg();

		/**
		 * Gets the reporting level.
		 *
		 * @return	The reporting level.
		 */
		static LogLevelEnum ReportingLevel() { return reportingLevel_; }

	private:
		std::ostringstream oss_;	/**< The oss */
		std::string time_;  /**< The time */
		LogLevelEnum level_;	/**< The level */
		bool rawMsg_;   /**< raw message flag, when no prefix is required set to true */
		static LogLevelEnum reportingLevel_;	/**< The reporting level */

		/**
		 * Copy constructor.  This is private so we cannot make copies of this class because it holds
		 * stringstreams.
		 *
		 * @param		Log class reference.
		 */
		Log(const Log&);

		/**
		 * Assignment operator.  This is private so we cannot make copies of this class because it holds
		 * stringstreams.
		 *
		 * @param		The.
		 *
		 * @return	A shallow copy of this object.
		 */
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

	/**
	 * Logs a message.  If the level is too high the message will not be logged.  For instance in
	 * the release build the maximum log level is set to LOGINFORMATION and any LOGDEBUG messages
	 * will not be seen.
	 *
	 * @see LOG_MSG_RAW
	 * 
	 * @param	level	The level.
	 */
	#define LOG_MSG(level) \
		if (level > Log::ReportingLevel()) ; \
		else Log().Msg(level)

	/**
	 * Logs a raw message.  This message log will not add any prefix but pass the message on as is.
	 * Dependent on the reporting level.
	 * 
	 * @see LOG_MSG.
	 *
	 * @param	level	The level.
	 */
	#define LOG_MSG_RAW(level) \
		if (level > Log::ReportingLevel()) ; \
		else Log().Msg()

}


#endif /* LOGMSG_H_ */

