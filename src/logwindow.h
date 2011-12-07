


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
	/**
	 * @brief Form for viewing the log.  This LogWindow follows the singleton pattern making the LogWindow
	 * available anywhere.  The constuctor is private to not allow any copies of this window.  The
	 * LogWindow has the facility to save the log contents to file.
	 * 
	 * The intention here is that this class is not used directly except for calling Show().  The
	 * interaction and message sending is best done through the Log class and the macros LOG_MSG and
	 * LOG_MSG_RAW.
	 */
	class LogWindow : public LogWindowUI
	{
	public:

		/**
		 * Destructor.
		 */
		~LogWindow();

		/**
		 * Gets the instance.  If the instance is null then a new LogWindow will be created and that
		 * will be returned.
		 *
		 * @return	null if it fails, else the instance.
		 */
		static LogWindow* GetInstance()
		{
			if (instance_ == 0)
			{
				
				instance_ = new LogWindow();
			}

			return instance_;
		}

		/**
		 * Logs a message.  This function logs a basic message with no prefix or text colouring.
		 *
		 * @param	message	The message.
		 */
		void LogMessage(const std::string& message);

		/**
		 * Logs a message.  This logs a message with the current time and log level prefix, for systems
		 * some systems the text output will be coloured.
		 *
		 * @param	time   	The time.
		 * @param	level  	The level.
		 * @param	message	The message.
		 */
		void LogMessage(const std::string& time, LogLevelEnum level, const std::string& message);

	private:

		/**
		 * Default constructor.
		 */
		LogWindow();

		/**
		 * Makes the gui connections between ui widgets and this class.
		 */
		void MakeConnections();

		/**
		 * Executes the close window action.
		 *
		 * @param [in,out]	event	The event.
		 */
		void OnCloseWindow(wxCloseEvent& event);

		/**
		 * Executes the save action.
		 *
		 * @param [in,out]	event	The event.
		 */
		void OnSave(wxCommandEvent& event);

		/**
		 * Executes the exit action.
		 *
		 * @param [in,out]	event	The event.
		 */
		void OnExit(wxCommandEvent& event);

		static LogWindow* instance_;	/**< The one and only instance of this class */
		std::string previousSaveLocation_;  /**< The previous save location of the log file, defaults to the current working directory */
	};


}

#endif LOGWINDOW_H_

