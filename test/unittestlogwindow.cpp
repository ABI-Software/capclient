/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>

#include "utils/debug.h"

#include "logwindow.h"
#include "ui/testlogwindowui.h"

// Manual testing mode.
#define ENABLE_GUI_INTERACTION

namespace cap
{
	class TestLogWindow : public TestLogWindowUI
	{
	public:
		TestLogWindow()
		{
			Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(TestLogWindow::OnCloseWindow));
			Connect(button_show_->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(TestLogWindow::ShowLogWindow));
		}

		void ShowLogWindow(wxCommandEvent& event)
		{
			LogWindow::GetInstance()->Show();
		}

		void OnCloseWindow(wxCloseEvent& event)
		{
			wxExit();;
		}
	};

	class TestApp : public wxApp
	{
	public:
		bool OnInit()
		{
			wxXmlResource::Get()->InitAllHandlers();
			wxXmlInit_logdialogui();

			//LogWindow::GetInstance()->Log(LOGINFORMATION) << "Logging message";
			//Logger::Log(LOGINFORMATION) << "seom test";
			LOG_MSG(LOGINFORMATION) << "some test";

			bool result = true;
			w_ = 0;
#ifdef ENABLE_GUI_INTERACTION
			wxXmlInit_testlogwindowui();
			w_ = new TestLogWindow();
			SetTopWindow(w_);
			result = w_->Show();
#endif

			for (int i = 0; i < 20; i++)
				LOG_MSG(LOGWARNING) << "Another test " << i;

			LOG_MSG(LOGDEBUG) << " A debug test: ";

			LOG_MSG(LOGERROR) << " And finally an error " << 3.1415973545;
			return result;
		}

		void CloseDialog()
		{
			w_->Destroy();
		}

		int OnExit()
		{
			dbg("TestApp::OnExit()");
			int r = wxApp::OnExit();
			LogWindow::GetInstance()->Destroy();
			w_->Destroy();
			if (w_)
				delete w_;
			return r;
		}

		TestLogWindow* w_;
	};
}

// Can't do multiple tests with this setup, one should be fine.
TEST(LogWindow, Log)
{
	int argc = 0;
	char **argv = 0;
	using namespace cap;
	wxApp::SetInstance( new TestApp() );
	wxEntryStart( argc, argv );
	wxTheApp->OnInit();
	
	// you can create top level-windows here or in OnInit()
	// do your testing here
#define ENABLE_GUI_INTERACTION
#if defined ENABLE_GUI_INTERACTION
	wxTheApp->OnRun(); // Do/Don't start main loop
#else
	TestApp *ta = static_cast<TestApp *>(wxTheApp);
//	wxCommandEvent event;
//	ta->sd_->sdw_->OnCancel(event);
#endif
	wxTheApp->OnExit();
	wxEntryCleanup();
	wxApp::SetInstance(0);
}

// Interaction code
//int main(int argc, char *argv[])
//{
//	using namespace cap;
//	wxApp::SetInstance( new TestApp() );
//	wxEntryStart( argc, argv );
//	wxTheApp->OnInit();
//	
//	// you can create top level-windows here or in OnInit()
//	// do your testing here
//	
//	wxTheApp->OnRun(); // Do/Don't start main loop
//	wxTheApp->OnExit();
//	wxEntryCleanup();
//
//	return 0;
//}

