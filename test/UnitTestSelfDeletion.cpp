/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>

#include "utils/debug.h"

#include "selfdeletion.h"

namespace cap
{
	bool SelfDeletion::wxXmlInitialised_ = false;

	class TestApp : public wxApp
	{
	public:
		bool OnInit()
		{
			wxXmlResource::Get()->InitAllHandlers();
			sd_ = SelfDeletion::CreateSelfDeletion();  // Leaves a hanging SelfDeletion pointer that must delete itself.
			return true;
		}

		int OnExit()
		{
			dbg("TestApp::OnExit()");
			int r = wxApp::OnExit();
			//delete sd_;
			return r;
		}

		SelfDeletion* sd_;
	};
}

// Can't do multiple tests with this setup, one should be fine.
TEST(SelfDeletion, DeleteOnCancel)
{
	int argc = 0;
	char **argv = 0;
	using namespace cap;
	wxApp::SetInstance( new TestApp() );
	wxEntryStart( argc, argv );
	wxTheApp->OnInit();
	
	// you can create top level-windows here or in OnInit()
	// do your testing here
	
#if defined ENABLE_GUI_INTERACTION
	wxTheApp->OnRun(); // Do/Don't start main loop
#else
	TestApp *ta = static_cast<TestApp *>(wxTheApp);
	wxCommandEvent event;
	ta->sd_->sdw_->OnCancel(event);
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

