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
#include <sstream>
#include <iomanip>

extern "C"
{
#include "finite_element/finite_element.h"
#include "graphics/scene_viewer.h"
}

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

std::string const ImageBrowseWindow::IMAGE_PREVIEW = std::string("ImagePreview");

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
	
	long columnIndex = 0;
	imageTable_->InsertColumn(columnIndex++, _("Series #"), wxLIST_FORMAT_CENTRE, 75);
	imageTable_->InsertColumn(columnIndex++, _("Series Description"), wxLIST_FORMAT_CENTRE, -1);
	imageTable_->InsertColumn(columnIndex++, _("Sequence Name"), wxLIST_FORMAT_CENTRE, 120);
//	imageTable_->InsertColumn(columnIndex++, _("Series Time"));
	imageTable_->InsertColumn(columnIndex++, _("Images"), wxLIST_FORMAT_CENTRE, 75);
	imageTable_->InsertColumn(columnIndex, _("Label"), wxLIST_FORMAT_CENTRE, 85);
	
//	wxString str = imageTable_->GetItemText(0);
//	std::cout << str << '\n';
//	std::cout << GetCellContentsString(0, 2) << '\n';
	
	SortDICOMFiles();
	PopulateImageTable();
	
	wxPanel* panel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	sceneViewer_ = cmguiManager_.CreateSceneViewer(panel);
	
//		Cmiss_scene_viewer_view_all(sceneViewer_);
	Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
	// TODO This window should use a separate scene from the default one (the one MainWindow uses)
	
	LoadImagePlaneModel();
	LoadImages();
	
	// Finally fit the window to meet the size requirements of its children
	// this also forces the cmgui panel to display correctly
	this->Fit();
	
	// This stops the window from getting too long in height 
	// when there are many items in the Image Table
	this->SetSize(-1, 768);
	this->Centre();
}

ImageBrowseWindow::~ImageBrowseWindow()
{
	//TODO : destroy textures
}

void ImageBrowseWindow::SortDICOMFiles()
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
		Vector3D v = file->GetImagePosition() - Point3D(0,0,0);
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
}

void ImageBrowseWindow::PopulateImageTable()
{	
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
		long columnIndex = 1;
		imageTable_->SetItem(itemIndex, columnIndex++, image->GetSeriesDescription().c_str()); 
		imageTable_->SetItem(itemIndex, columnIndex++, image->GetSequenceName().c_str());
//		double triggerTime = image->GetTriggerTime();
//		std::string seriesTime = triggerTime < 0 ? "" : lexical_cast<string>(image->GetTriggerTime());// fix
//		imageTable_->SetItem(itemIndex, columnIndex++, seriesTime.c_str());
		imageTable_->SetItem(itemIndex, columnIndex++, lexical_cast<string>(value.second.size()).c_str());
		
		imageTable_->SetItemData(itemIndex, reinterpret_cast<long int>(&value)); // Check !! is this safe??!!
		rowNumber++;
	}
	
	DICOMPtr const& firstImage = sliceMap_.begin()->second[0];
	UpdatePatientInfoPanel(firstImage);
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

void ImageBrowseWindow::SwitchSliceToDisplay(SliceMap::value_type const& slice)
{
//	std::cout << __func__ << '\n';
	SliceKeyType const& key = slice.first;
	std::vector<DICOMPtr> const& images = slice.second;
	assert(textureMap_.find(key) != textureMap_.end());
	std::vector<Cmiss_texture_id> const& textures = textureMap_[key];
	texturesCurrentlyOnDisplay_ = &textures;
	
	// Update the gui
	UpdateImageInfoPanel(images[0]); // should rename to Series Info?
	
	// Update image preview panel
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	assert(slider);	
	slider->SetMin(1);
	slider->SetMax(images.size());
	slider->SetValue(1);
	
	// Resize the rectangular cmgui model
	// according to the new dimensions
	// REVISE : Extract to function (possibly to a diff class)
	size_t width = images[0]->GetImageWidth();
	size_t height = images[0]->GetImageHeight();
	Cmiss_context_id cmissContext_ = cmguiManager_.GetCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region* region = Cmiss_region_find_subregion_at_path(root_region, IMAGE_PREVIEW.c_str());
	assert(region);
	
	Cmiss_node* node = Cmiss_region_get_node(region, "1");
	FE_node_set_position_cartesian(node, 0, 0.0, 0.0, 0.0);	
	node = Cmiss_region_get_node(region, "2");
	FE_node_set_position_cartesian(node, 0, width, 0.0, 0.0);
	node = Cmiss_region_get_node(region, "3");
	FE_node_set_position_cartesian(node, 0, 0.0, height, 0.0);
	node = Cmiss_region_get_node(region, "4");
	FE_node_set_position_cartesian(node, 0, width, height, 0.0);
	
	DisplayImage(textures[0]);
	Cmiss_scene_viewer_view_all(sceneViewer_);
	Scene_viewer_viewport_zoom(sceneViewer_, 20.0); //REVIEW
//	Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 ); //REVIEW
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
}

void ImageBrowseWindow::LoadImagePlaneModel()
{	
	cmguiManager_.ReadRectangularModelFiles(IMAGE_PREVIEW);	
	material_ = cmguiManager_.CreateCAPMaterial(IMAGE_PREVIEW);
	cmguiManager_.AssignMaterialToObject(sceneViewer_, material_->GetCmissMaterial(), IMAGE_PREVIEW);
	
	return;
}

void ImageBrowseWindow::DisplayImage(Cmiss_texture_id tex)
{	
	if (material_)
	{
		material_->ChangeTexture(tex);
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

void ImageBrowseWindow::SetInfoField(std::string const& fieldName, std::string const& data)
{
	wxStaticText* st = XRCCTRL(*this, fieldName.c_str(), wxStaticText);
	st->SetLabel(data.c_str());
}

void ImageBrowseWindow::UpdatePatientInfoPanel(DICOMPtr const& image)
{
	using std::string;
	string const& name = image->GetPatientName();
	SetInfoField("PatientName", name);
	string const& id = image->GetPatientID();
	SetInfoField("PatientID", id);
	string const& scanDate = image->GetScanDate();
	SetInfoField("ScanDate", scanDate);
	string const& dob = image->GetDateOfBirth();
	SetInfoField("DateOfBirth", dob);
	string const& gender = image->GetGender();
	string const& age = image->GetAge();
	SetInfoField("GenderAndAge", gender + " " + age);
}

void ImageBrowseWindow::OnImageTableItemSelected(wxListEvent& event)
{	
	SliceMap::value_type* const sliceValuePtr = reinterpret_cast<SliceMap::value_type* const>(event.GetItem().GetData());
//	std::cout << "Series Num = " << (*sliceValuePtr).first.first << '\n';
//	std::cout << "Distance to origin = " << (*sliceValuePtr).first.second << '\n';
//	std::cout << "Image filename = " << (*sliceValuePtr).second[0]->GetFilename() << '\n';
		
	// Display the images from the selected row.
	SwitchSliceToDisplay(*sliceValuePtr);
}

void ImageBrowseWindow::UpdateImageInfoPanel(DICOMPtr const& dicomPtr)
{
	using std::string;
	using boost::lexical_cast;
	
	size_t width = dicomPtr->GetImageWidth();
	size_t height = dicomPtr->GetImageHeight();
	string size = lexical_cast<string>(width) + " x " + lexical_cast<string>(height);
	SetInfoField("ImageSize", size);
	
	Point3D position = dicomPtr->GetImagePosition();
	std::stringstream ss;
	ss << std::setprecision(5) << position.x << " ";
	ss << std::setprecision(5) << position.x << " ";
	ss << std::setprecision(5) << position.z;
	string const &posStr(ss.str());
	SetInfoField("ImagePosition", posStr);
}

void ImageBrowseWindow::OnPlayToggleButtonPressed(wxCommandEvent& event)
{
}

void ImageBrowseWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
	int textureIndex = value - 1; // tex index is 0 based while slider value is 1 based
	
	DisplayImage((*texturesCurrentlyOnDisplay_)[value-1]);
//	Time_keeper_request_new_time(timeKeeper_, time);
	
	// force redraw while silder is manipulated
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
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
//	std::cout << __func__ << "- event.GetInt() = " << value << "\n";
	float brightness = static_cast<float>(value)/100.0;
	material_->SetBrightness(brightness);
	
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
}

void ImageBrowseWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "ContrastSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
//	std::cout << __func__ << "- slide.GetMin() = " << min << "\n";
//	std::cout << __func__ << "- sliderGetMax() = " << max << "\n";
	
	float contrast = (float)(value - min) / (float)(max - min);
	assert(material_);
	material_->SetContrast(contrast);
	
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
}

void ImageBrowseWindow::PutLabelOnSelectedSlice(wxListCtrl* imageTable, std::string const& label)
{
//	std::cout << __func__ << '\n';
	long index = imageTable->GetNextItem(-1,
						wxLIST_NEXT_ALL,
						wxLIST_STATE_SELECTED);
	if (index != -1)
	{
		static long const LABEL_COLUMN_INDEX = 4;
		imageTable->SetItem(index, LABEL_COLUMN_INDEX, label.c_str());
		if (index < imageTable->GetItemCount() - 1)
		{
			imageTable->SetItemState(index  , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			imageTable->SetItemState(index+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
	}
}

void ImageBrowseWindow::OnShortAxisButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	PutLabelOnSelectedSlice(imageTable_, "Short Axis");
}

void ImageBrowseWindow::OnLongAxisButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	PutLabelOnSelectedSlice(imageTable_, "Long Axis");
}

void ImageBrowseWindow::OnNoneButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	PutLabelOnSelectedSlice(imageTable_, "");
}

void ImageBrowseWindow::ImageBrowseWindow::OnCloseImageBrowseWindow(wxCloseEvent& event)
{
	// TODO DO clean up!!
	exit(0);
}

BEGIN_EVENT_TABLE(ImageBrowseWindow, wxFrame)
	EVT_LIST_ITEM_SELECTED(XRCID("ImageTable"), ImageBrowseWindow::OnImageTableItemSelected)
	EVT_SLIDER(XRCID("AnimationSlider"),ImageBrowseWindow::OnAnimationSliderEvent)
	EVT_BUTTON(XRCID("PlayButton"),ImageBrowseWindow::OnPlayToggleButtonPressed) 
	EVT_SLIDER(XRCID("AnimationSpeedControl"),ImageBrowseWindow::OnAnimationSpeedControlEvent)
	EVT_SLIDER(XRCID("BrightnessSlider"),ImageBrowseWindow::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),ImageBrowseWindow::OnContrastSliderEvent)
	EVT_BUTTON(XRCID("ShortAxisButton"), ImageBrowseWindow::OnShortAxisButtonEvent)
	EVT_BUTTON(XRCID("LongAxisButton"), ImageBrowseWindow::OnLongAxisButtonEvent)
	EVT_BUTTON(XRCID("NoneButton"), ImageBrowseWindow::OnNoneButtonEvent)
	EVT_CLOSE(ImageBrowseWindow::OnCloseImageBrowseWindow)
END_EVENT_TABLE()

} // end namespace cap
