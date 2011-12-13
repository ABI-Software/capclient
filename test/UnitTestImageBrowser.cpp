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
	};

	class TestApp : public wxApp
	{
		bool OnInit()
		{
			wxXmlResource::Get()->InitAllHandlers();
			return true;
		}
	};

} // namespace cap

TEST(ImageBrowser, CreateUsingFactory)
{
	int argc = 0;
	char **argv = 0;
	using namespace cap;
	FakeCAPClient client;
	wxApp::SetInstance( new TestApp() );
	wxEntryStart( argc, argv );
	wxTheApp->OnInit();
	
	// you can create top level-windows here or in OnInit()
	// do your testing here
	ImageBrowser* ib = ImageBrowser::CreateImageBrowser(DICOMIMAGE_IMAGEDIR, &client);
	
	wxTheApp->OnRun(); // Do/Don't start main loop
	delete ib;
	wxTheApp->OnExit();
	wxEntryCleanup();
}

