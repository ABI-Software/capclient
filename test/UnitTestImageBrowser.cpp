/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>

#include <wx/xrc/xmlres.h>

#include "imagebrowser.h"
#include "iimagebrowserwindow.h"

namespace cap
{

class FakeCAPClient : public IImageBrowserWindow
{
public:
	virtual ~FakeCAPClient() {}
	virtual void LoadImagesFromImageBrowserWindow(SlicesWithImages const& slices, CardiacAnnotation const& anno)
	{}
};

// Link time test double
FileSystem::FileSystem(std::string const& path)
{}

std::vector<std::string> const& FileSystem::getAllFileNames()
{
	return filenames;
}

DICOMImage::DICOMImage(const std::string& filename)
{
}

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
	//...
	// do your testing here
	ImageBrowser* ib = ImageBrowser::CreateImageBrowser("dirname", &client);
	
	//wxTheApp->OnRun(); // Don't start main loop
	wxTheApp->OnExit();
	wxEntryCleanup();
	delete ib;
}

