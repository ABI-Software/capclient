

#include <wx/wx.h>
#include <wx/cmdline.h>

namespace cap
{

/**
 * @brief CAPAPP is the wxWidgets main application class and holds the main frame.
 */
class CAPApp : public wxApp
{
public:
	/**
	 * Initialise the program, by creating a new mainwindow.
	 */
	bool OnInit();
	
	/**
	 * Override to clean up.
	 */
	int OnExit();
	
private:
	bool server_mode_;
	
};

}

