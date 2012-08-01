/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>

#include "unittestconfigure.h"
#include "utils/debug.h"
#include "ui/euladialog.h"
#include "logmsg.h"

#define ENABLE_GUI_INTERACTION

namespace cap
{
	std::string TimeNow() { return ""; }
	Log::~Log() {}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;

	class TestApp : public wxApp
	{
	public:

		TestApp()
		{
		}

		bool OnInit()
		{
#ifdef ENABLE_GUI_INTERACTION
			wxXmlResource::Get()->InitAllHandlers();
			cap::EulaDialog eulaDialog;
			eulaDialog.ShowModal();
#endif
			return false;
		}
	};
}

TEST(EULADialog, Show)
{
	int argc = 0;
	char **argv = 0;
	using namespace cap;
	wxApp::SetInstance( new TestApp() );
	wxEntryStart( argc, argv );
#if defined ENABLE_GUI_INTERACTION
	wxTheApp->OnInit();
#endif
	wxTheApp->OnExit();
	wxEntryCleanup();
	wxApp::SetInstance(0);
}

