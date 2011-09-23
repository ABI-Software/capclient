/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <wx/wxprec.h>
#include <wx/notebook.h>
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <gtest/gtest.h>

#include "imagebrowser.h"

namespace cap
{

class FakeCmguiManager
{
public:
	Cmiss_texture* LoadCmissTexture(std::string const& filename) const 
	{
		return 0;
	}
};

class TestImageBrowseWindow
{
public:
	TestImageBrowseWindow(ImageBrowser<TestImageBrowseWindow, FakeCmguiManager>& browser,
			FakeCmguiManager const& manager)
		: browser_(browser)
	{}
	
	void CreateProgressDialog(std::string const& title, std::string const& msg, int max) {}
	void UpdateProgressDialog(int count) {}
	void DestroyProgressDialog() {}
	
	void LoadWindowLayout() {}
	void CreatePreviewPanel() {}
	void FitWindow() {}
	
	void SetInfoField(std::string const& name, std::string const& value) {}
	
	void ClearImageTable() {}
	void CreateImageTableColumns() {}
	void PopulateImageTableRow(int rowNumber,
			int seriesNumber, std::string const& seriesDescription,
			std::string const& sequenceName, size_t numImages,
			long int const& userDataPtr)
	{}
	
	void SelectFirstRowInImageTable() {}
	void SetImageTableRowLabelByUserData(long int, std::string const&)
	{}
	void Show(bool) {}
	
private:
	ImageBrowser<TestImageBrowseWindow, FakeCmguiManager>& browser_;
};

class TestImageBrowseWindowClient : public IImageBrowseWindow
{
public:
	virtual ~TestImageBrowseWindowClient() {}
	virtual void LoadImagesFromImageBrowseWindow(SlicesWithImages const& slices, CardiacAnnotation const& anno)
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

} // namespace cap
//TEST(ImageBrowser, Create)
//{
//	using namespace cap;
//	
//	TestImageBrowseWindowClient client;
//	FakeCmguiManager cmguiManager;
//	ImageBrowser<TestImageBrowseWindow, FakeCmguiManager> iBrowser("dirname", cmguiManager, client);
//	TestImageBrowseWindow gui(iBrowser, cmguiManager);
//	iBrowser.SetImageBrowseWindow(&gui);
//	iBrowser.Initialize();
//	
//	iBrowser.CreateTexturesFromDICOMFiles();
//}

TEST(ImageBrowser, CreateUsingFactory)
{
	using namespace cap;
	
	TestImageBrowseWindowClient client;
	FakeCmguiManager cmguiManager;
	
	ImageBrowser* ib = 
			ImageBrowser::CreateImageBrowser("dirname", client);
	
	delete ib;
}
