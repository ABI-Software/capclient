/*
 * ImageBrowseWindow.cpp
 *
 *  Created on: Jun 17, 2010
 *      Author: jchu014
 */
#include "ImageBrowseWindow.h"

#include "Config.h"
#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "DICOMImage.h"
#include "CAPMaterial.h"

#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/dir.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>

#include <iostream>

namespace
{

std::vector<std::string> EnumerateAllFiles(const std::string& dirname)
{
	using std::vector;
	using std::string;
	
	wxString wxDirname(dirname.c_str());
	wxDir dir(wxDirname);

	if ( !dir.IsOpened() )
	{
		// deal with the error here - wxDir would already log an error message
		// explaining the exact reason of the failure
		return vector<string>();
	}

	puts("Enumerating files in current directory:");

	vector<string> filenames;
	wxString filename;

	bool cont = dir.GetFirst(&filename, "*.dcm", wxDIR_FILES);
	while ( cont )
	{
		printf("%s\n", filename.c_str());
		filenames.push_back(filename.c_str());
		cont = dir.GetNext(&filename);
	}
	return filenames;
}

const char* TEST_DIR = "./temp/XMLZipTest";
} // end anonyous namespace

namespace cap
{

ImageBrowseWindow::ImageBrowseWindow(std::string const& archiveFilename, CmguiManager const& manager)
:
	archiveFilename_(archiveFilename),
	texturesCurrentlyOnDisplay_(0),
	cmguiManager_(manager)
{
	wxXmlResource::Get()->Load("ImageBrowseWindow.xrc");
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ImageBrowseWindow"));
	
	imageTable_ = XRCCTRL(*this, "ImageTable", wxListCtrl);

	assert(imageTable_);
	
	imageTable_->InsertColumn(0, _("Series Number"));
	imageTable_->InsertColumn(1, _("Series Desc"));
	imageTable_->InsertColumn(2, _("Sequence Name"));
	imageTable_->InsertColumn(3, _("Series Time"));
	imageTable_->InsertColumn(4, _("Num Images"));
	imageTable_->InsertColumn(5, _("Label"));
	
//	wxString str = imageTable_->GetItemText(0);
//	std::cout << str << '\n';
//	std::cout << GetCellContentsString(0, 2) << '\n';
	
	PopulateImageTable();
	
	wxPanel* panel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	sceneViewer_ = cmguiManager_.CreateSceneViewer(panel);
	
//		Cmiss_scene_viewer_view_all(sceneViewer_);
//		Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
	// TODO This window should use a separate scene from the default one (the one MainWindow uses)
	
	LoadImagePlaneModel();
	LoadImages();
}

ImageBrowseWindow::~ImageBrowseWindow()
{
	//TODO : destroy textures
}

void ImageBrowseWindow::PopulateImageTable()
{
	std::string dirname = TEST_DIR;
	std::vector<std::string> const& filenames = EnumerateAllFiles(dirname);
	
	std::cout << "num files = " << filenames.size() << '\n';
	
	BOOST_FOREACH(std::string const& filename, filenames)
	{
		std::cout << filename <<'\n';
		std::string fullpath = dirname + "/" + (filename);
		std::cout << fullpath <<'\n';
		DICOMPtr file(new DICOMImage(fullpath));
		int seriesNum = file->GetSeriesNumber();
		std::cout << "Series Num = " << seriesNum << '\n';
		Vector3D v = file->GetPosition() - Point3D(0,0,0);
		double distanceFromOrigin = v.Length();
		
		SliceKeyType key = std::make_pair(seriesNum, distanceFromOrigin);
		SliceMap::iterator itr = sliceMap_.find(key);
		if (itr != sliceMap_.end())
		{
			itr->second.push_back(file);
		}
		else
		{
			std::vector<DICOMPtr> v(1, file);
			sliceMap_.insert(std::make_pair(key, v));
		}		
	}
	
	int rowNumber = 0;
	BOOST_FOREACH(SliceMap::value_type& value, sliceMap_)
	{
		using boost::lexical_cast;
		using namespace boost::lambda;
		using std::string;
		
		std::vector<DICOMPtr>& images = value.second;
		std::sort(images.begin(), images.end(), *_1 < *_2);
		DICOMPtr image = images[0];
		long itemIndex = imageTable_->InsertItem(rowNumber, lexical_cast<string>(image->GetSeriesNumber()).c_str());
		imageTable_->SetItem(itemIndex, 1, image->GetSeriesDescription().c_str()); 
		imageTable_->SetItem(itemIndex, 2, image->GetSequenceName().c_str());
		double triggerTime = image->GetTriggerTime();
		std::string seriesTime = triggerTime < 0 ? "" : lexical_cast<string>(image->GetTriggerTime());// fix
		imageTable_->SetItem(itemIndex, 3, seriesTime.c_str());
		imageTable_->SetItem(itemIndex, 4, lexical_cast<string>(value.second.size()).c_str());
		
		imageTable_->SetItemData(itemIndex, reinterpret_cast<long int>(&value)); // Check !! is this safe??!!
		rowNumber++;
	}
}

void ImageBrowseWindow::LoadImages()
{
	using namespace std;
	
	// load some images and display
	
	BOOST_FOREACH(SliceMap::value_type& value, sliceMap_)
	{	
		vector<DICOMPtr>& images = value.second;
		vector<DICOMPtr>::const_iterator itr = images.begin();
		vector<DICOMPtr>::const_iterator end = images.end();
		vector<Cmiss_texture_id> textures;
		
		for (; itr != end; ++itr)
		{
			const string& filename = (*itr)->GetFilename();	
			Cmiss_texture_id texture_id = cmguiManager_.LoadCmissTexture(filename);
			textures.push_back(texture_id);
		}
		
		textureMap_.insert(make_pair(value.first, textures));
	}
	return;
}

void ImageBrowseWindow::SwitchSliceToDisplay(SliceKeyType const& key)
{
//	TextureMap::const_iterator itr = textureMap_.find(key);
	assert(textureMap_.find(key) != textureMap_.end());
	std::cout << __func__ << '\n';
	std::vector<Cmiss_texture_id> const& textures = textureMap_[key];
	texturesCurrentlyOnDisplay_ = &textures;
	
	DisplayImage(textures[0]);
	Cmiss_scene_viewer_view_all(sceneViewer_);
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
}

void ImageBrowseWindow::LoadImagePlaneModel()
{	
	std::string name("LA1"); // change
	
	cmguiManager_.ReadRectangularModelFiles(name);	
	material_ = cmguiManager_.CreateCAPMaterial("ImageBrowseWindow");
	cmguiManager_.AssignMaterialToObject(sceneViewer_, material_->GetCmissMaterial(), name);
	
	return;
}

void ImageBrowseWindow::DisplayImage(Cmiss_texture_id tex)
{	
	Cmiss_context_id cmissContext_ = cmguiManager_.GetCmissContext();
	if (material_)
	{
		cmguiManager_.SwitchMaterialTexture(material_->GetCmissMaterial(),tex, "LA1"); //FIX "LA1"
	}
	else
	{
		std::cout << "Error: cant find material\n";
	}

	return;
}

wxString ImageBrowseWindow::GetCellContentsString( long row_number, int column )
{
	wxListItem     row_info;  
	wxString       cell_contents_string;
	
	// Set what row it is
	row_info.SetId(row_number);
	// Set what column of that row we want to query for information.
	row_info.SetColumn(column);
	// Set text mask
	row_info.SetState(wxLIST_MASK_TEXT);
	
	// Get the info and store it in row_info variable.   
	imageTable_->GetItem( row_info );
	
	// Extract the text out that cell
	cell_contents_string = row_info.GetText(); 
	
	return cell_contents_string;
}

void ImageBrowseWindow::OnImageTableItemSelected(wxListEvent& event)
{
	std::cout << __func__ << '\n';
//	std::cout << "id = " << event.GetId() << '\n';
	std::cout << "index = " << event.GetIndex() << '\n';
//	std::cout << "text = " << event.GetText() << '\n';
//	std::cout << "item.text = " << event.GetItem().GetText() << '\n';
//	std::cout << "item.id = " << event.GetItem().GetId() << '\n';
	
	SliceMap::value_type* const sliceValuePtr = reinterpret_cast<SliceMap::value_type* const>(event.GetItem().GetData());
	std::cout << "Series Num = " << (*sliceValuePtr).first.first << '\n';
	std::cout << "Distance to origin = " << (*sliceValuePtr).first.second << '\n';
	
	std::cout << "Image filename = " << (*sliceValuePtr).second[0]->GetFilename() << '\n';
	
	// Display the images from the selected row.
	SwitchSliceToDisplay((*sliceValuePtr).first);
}

void ImageBrowseWindow::OnPlayToggleButtonPressed(wxCommandEvent& event)
{
}

void ImageBrowseWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	int value = slider->GetValue();
	
	int min = slider->GetMin();
	int max = slider->GetMax();
	double time =  (double)(value - min) / (double)(max - min);
//	double prevFrameTime = heartModel_.MapToModelFrameTime(time);
//	if ((time - prevFrameTime) < (0.5)/(heartModel_.GetNumberOfModelFrames()))
//	{
//		time = prevFrameTime;
//	}
//	else
//	{
//		time = prevFrameTime + (float)1/(heartModel_.GetNumberOfModelFrames());
//	}
	slider->SetValue(time * (max - min));
//	cout << "time = " << time << endl;;	
//	imageSet_->SetTime(time);
	time = (time > 0.99) ? 0 : time;
	
//	Time_keeper_request_new_time(timeKeeper_, time);
	
//	RefreshCmguiCanvas(); // forces redraw while silder is manipulated
	return;
}

void ImageBrowseWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSpeedControl", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	double speed = (double)(value - min) / (double)(max - min) * 2.0;
//	Time_keeper_set_speed(timeKeeper_, speed);
//	
//	RefreshCmguiCanvas(); // forces redraw while silder is manipulated
	return;
}

void ImageBrowseWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
	std::cout << __func__ << "- event.GetInt() = " << value << "\n";
//	wxSlider* slider = XRCCTRL(*this, "BrightnessSlider", wxSlider);
//	int value = slider->GetValue();
//	int min = slider->GetMin();
//	int max = slider->GetMax();
//	
//	float brightness = (float)(value - min) / (float)(max - min);
//	material_->SetBrightness(brightness);
	
//	RefreshCmguiCanvas();
}

void ImageBrowseWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "ContrastSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	std::cout << __func__ << "- slide.GetMin() = " << min << "\n";
	std::cout << __func__ << "- sliderGetMax() = " << max << "\n";
	
	float contrast = (float)(value - min) / (float)(max - min);
	material_->SetContrast(contrast);
		
//	RefreshCmguiCanvas();
}

void ImageBrowseWindow::ImageBrowseWindow::OnCloseImageBrowseWindow(wxCloseEvent& event)
{
	// TODO DO clean up!!
}

BEGIN_EVENT_TABLE(ImageBrowseWindow, wxFrame)
	EVT_LIST_ITEM_SELECTED(XRCID("ImageTable"), ImageBrowseWindow::OnImageTableItemSelected)
	EVT_SLIDER(XRCID("AnimationSlider"),ImageBrowseWindow::OnAnimationSliderEvent)
	EVT_BUTTON(XRCID("PlayButton"),ImageBrowseWindow::OnPlayToggleButtonPressed) 
	EVT_SLIDER(XRCID("AnimationSpeedControl"),ImageBrowseWindow::OnAnimationSpeedControlEvent)
	EVT_SLIDER(XRCID("BrightnessSlider"),ImageBrowseWindow::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),ImageBrowseWindow::OnContrastSliderEvent)
	EVT_CLOSE(ImageBrowseWindow::OnCloseImageBrowseWindow)
END_EVENT_TABLE()

} // end namespace cap
