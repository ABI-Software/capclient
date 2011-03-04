/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include "../src/ImageBrowser.h"
#include <gtest/gtest.h>

class Cmiss_textre;

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
	:
		browser_(browser)
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
	
	void Show(bool) {}
	
private:
	ImageBrowser<TestImageBrowseWindow, FakeCmguiManager>& browser_;
};

class TestImageBrowseWindowClient : public ImageBrowseWindowClient
{
public:
	virtual ~TestImageBrowseWindowClient() {}
	virtual void LoadImagesFromImageBrowseWindow(SlicesWithImages const& slices)
	{}
};

} // namespace cap
//TEST(ImageBrowser, Create)
//{
//	using namespace cap;
//	
//	TestImageBrowseWindowClient client;
//	FakeCmguiManager cmguiManager;
//	ImageBrowser<TestImageBrowseWindow, FakeCmguiManager> iBrowser("dirname", cmguiManager, client);
//	TestImageBrowseWindow gui(iBrowser);
//	iBrowser.SetImageBrowseWindow(&gui);
//	iBrowser.Initialize();
//	
//	iBrowser.CreateTexturesFromDICOMFiles();
//}

//TEST(ImageBrowser, CreateUsingFactory)
//{
//	using namespace cap;
//	
//	TestImageBrowseWindowClient client;
//	FakeCmguiManager cmguiManager;
//	
//	ImageBrowser<TestImageBrowseWindow, FakeCmguiManager>* ib = 
//			CreateImageBrowser<TestImageBrowseWindow, FakeCmguiManager>("dirname", cmguiManager, client);
//	
//	delete ib;
//}
