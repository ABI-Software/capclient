

#include <wx/wx.h>
#include <wx/cmdline.h>

namespace cap
{

/**
 * wxWidgets main application class holds the main frame.
 */
class CAPApp : public wxApp
{
public:
	/**
	 * Initialise the program, by creating a new mainwindow.
	 */
	virtual bool OnInit();
	
	/**
	 * Override to clean up.
	 */
	int OnExit();
	
	/**
	 * Set the command line description in this function.
	 */
	void OnInitCmdLine(wxCmdLineParser& parser);
	
	/**
	 * Actually do the parsing here.
	 */
	bool OnCmdLineParsed(wxCmdLineParser& parser);
	
private:
	bool server_mode_;
	
};

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{ wxCMD_LINE_SWITCH, wxT("h"), wxT("help"), wxT("displays help on the command line parameters"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_SWITCH, wxT("t"), wxT("test"), wxT("test switch"), wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_MANDATORY  },
	{ wxCMD_LINE_SWITCH, wxT("s"), wxT("server"), wxT("disables the GUI") },
	{ wxCMD_LINE_NONE }
};

}

