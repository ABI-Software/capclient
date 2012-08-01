/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>

#include "unittestconfigure.h"

#include "imagebrowser.h"
#include "iimagebrowser.h"
#include "logmsg.h"


// Manual testing mode.
#define ENABLE_GUI_INTERACTION

namespace cap
{
	Log::~Log() {}
	LogLevelEnum Log::reportingLevel_ = LOGDEBUG;

	class FakeCAPClient : public IImageBrowser
	{
	public:
		virtual ~FakeCAPClient() {}
		void LoadCardiacAnnotations(const CardiacAnnotation &){}
		void LoadLabelledImages(const LabelledSlices &){}
		void SetImageLocation(const std::string& location){}
		void ResetModel() {}
	};

	class TestApp : public wxApp
	{
		bool OnInit()
		{
			wxXmlResource::Get()->InitAllHandlers();
#if defined ENABLE_GUI_INTERACTION
			CardiacAnnotation anno;
			ImageBrowser::CreateImageBrowser(DICOMIMAGE_IMAGEDIR, anno, &client_);
#endif
			return true;
		}

		FakeCAPClient client_;
	};

} // namespace cap

TEST(ImageBrowser, CreateUsingFactory)
{
	//_CrtDumpMemoryLeaks();
	//_CrtSetBreakAlloc(22240);
	int argc = 0;
	char **argv = 0;
	using namespace cap;
	FakeCAPClient client;
	wxApp::SetInstance( new TestApp() );
	wxEntryStart( argc, argv );
	wxTheApp->OnInit();
	
	// you can create top level-windows here or in OnInit()
	// do your testing here
#if defined ENABLE_GUI_INTERACTION
	wxTheApp->OnRun(); // Do/Don't start main loop
#else
	CardiacAnnotation anno;
	ImageBrowser::CreateImageBrowser(DICOMIMAGE_IMAGEDIR, anno, &client);
#endif
	
	wxTheApp->OnExit();
	wxEntryCleanup();
}

