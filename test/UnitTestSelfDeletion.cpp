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
#include "ui/selfdeletionwindowui.h"

namespace cap
{
	class SelfDeletion;

	class SelfDeletionWindow : public SelfDeletionWindowUI
	{
	public:
		SelfDeletionWindow(SelfDeletion *sd)
			: sd_(sd)
		{
		}

		~SelfDeletionWindow()
		{
			dbg("~SelfDeletionWindow()");
		}

		SelfDeletion *sd_;
	};

	class SelfDeletion
	{
	public:
		void SetSelfDeletionWindow(SelfDeletionWindow *sdw)
		{
			sdw_ = sdw;
		}

		static SelfDeletion* CreateSelfDeletion()
		{
			if (!wxXmlInitialised_)
			{
				wxXmlInitialised_ = true;
				wxXmlInit_selfdeletionwindowui();
			}

			SelfDeletion* sd = new SelfDeletion();
			SelfDeletionWindow* frame = new SelfDeletionWindow(sd);
			frame->Show(true);
			sd->SetSelfDeletionWindow(frame);
			
			return sd;
		}

		SelfDeletion()
			: sdw_(0)
		{
		}

		~SelfDeletion()
		{
			dbg("~SelfDeletion()");
		}

		static bool wxXmlInitialised_;
		SelfDeletionWindow *sdw_;
	};
	bool SelfDeletion::wxXmlInitialised_ = false;

	class TestApp : public wxApp
	{
		bool OnInit()
		{
			wxXmlResource::Get()->InitAllHandlers();
			return true;
		}
	};
}

TEST(SelfDeletion, DeleteOnOk)
{
	int argc = 0;
	char **argv = 0;
	using namespace cap;
	wxApp::SetInstance( new TestApp() );
	wxEntryStart( argc, argv );
	wxTheApp->OnInit();
	
	// you can create top level-windows here or in OnInit()
	// do your testing here
	SelfDeletion* sd = SelfDeletion::CreateSelfDeletion();
	
	//wxTheApp->OnRun(); // Don't start main loop
	wxTheApp->OnExit();
	//delete sd;
	wxEntryCleanup();
}

