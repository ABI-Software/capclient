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
#include "ImageBrowseWindowClient.h"

#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/progdlg.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <set>

extern "C"
{
#include "finite_element/finite_element.h"
#include "graphics/scene_viewer.h"
#include "graphics/scene.h"
#include "three_d_drawing/graphics_buffer.h"
}

namespace
{
//const char* TEST_DIR = "./temp/XMLZipTest";
static long const LABEL_COLUMN_INDEX = 4;

} // end anonymous namespace

namespace cap
{

std::string const ImageBrowseWindow::IMAGE_PREVIEW = std::string("ImagePreview");

static int dummy_input_callback(struct Scene_viewer *scene_viewer,
		struct Graphics_buffer_input *input, void *viewer_frame_void)
{
//	if (input->type == GRAPHICS_BUFFER_BUTTON_RELEASE)
//	{
//		static_cast<ImageBrowseWindow*>(viewer_frame_void)->ShowImageAnnotation();
//	}
	return 0; // returning false means don't call the other input handlers;
}


//ImageBrowseWindow::ImageBrowseWindow(SlicesWithImages const& slicesWithImages, CmguiManager const& manager, ImageBrowseWindowClient& client)
//:
//	texturesCurrentlyOnDisplay_(0),
//	cmguiManager_(manager),
//	client_(client),
//	sortingMode_(SERIES_NUMBER)
//{	
//	LoadWindowLayout();
//	CreatePreviewPanel();
//	
//	std::set<int> setOfSeriesNumbers;
//	BOOST_FOREACH(SliceInfo const& sliceInfo, slicesWithImages)
//	{
//		size_t numberOfFrames = sliceInfo.GetDICOMImages().size();
//		assert(numberOfFrames);
//		for (size_t i = 0; i < numberOfFrames; ++i)
//		{
//			// Need to assign unigue id for each image (here we use the file name)
//			// this id is used to map a DICOMFile object to its corresponding Cmiss_texture
//			DICOMPtr const& dicomPtr = sliceInfo.GetDICOMImages().at(i);
//			std::string const& filename = dicomPtr->GetFilename();
//			dicomFileTable_.insert(std::make_pair(filename, dicomPtr));
//			Cmiss_texture_id texture = sliceInfo.GetTextures().at(i);
//			textureTable_.insert(std::make_pair(filename, texture));
//		}
//		
//		// Check if sortingMode_ is SERIES_NUMBER or SERIES_NUMBER_AND_IMAGE_POSITION
//		// by looking at whether images from any two slices share the same series number
//		DICOMPtr const& dicomPtr = sliceInfo.GetDICOMImages().at(0);
//		int seriesNumber = dicomPtr->GetSeriesNumber();
//		if (setOfSeriesNumbers.count(seriesNumber))
//		{
//			sortingMode_ = SERIES_NUMBER_AND_IMAGE_POSITION;
//		}
//		setOfSeriesNumbers.insert(seriesNumber);
//	}
//	
//	PopulateImageTable();
//	// Now populate the Image Table with the correct labels
//	// Since the sliceMap may not be in the same sort order as slicesWithImages,
//	// We need to map the two 
//	// TODO: revise the data structures used so this mapping is not needed?
//	long index = 0;
//	BOOST_FOREACH(SliceMap::value_type const& value, sliceMap_)
//	{
//		// find the matching slice in the SlicesWithImages
//		// Not that this is O(n^2) operation but should be ok as the number of slices are usually small (20~40)
//		std::string const& filename1 = value.second.at(0)->GetSopInstanceUID();
//		BOOST_FOREACH(SliceInfo const& sliceInfo, slicesWithImages)
//		{
//			std::string const& filename2 = sliceInfo.GetDICOMImages().at(0)->GetSopInstanceUID();
//			if (filename1 == filename2) // matching slices
//			{
//				std::string const& label = sliceInfo.GetLabel();
//				std::string longLabel;
//				if (label.find("LA") == 0)
//				{
//					longLabel = "Long Axis";
//				}
//				else if (label.find("SA") == 0)
//				{
//					longLabel = "Short Axis";
//				}
//				imageTable_->SetItem(index, LABEL_COLUMN_INDEX, longLabel.c_str());
//				index ++;
//			}
//		}
//	}
//	
//	FitWindow();
//}

//ImageBrowseWindow::ImageBrowseWindow(DICOMTable const& dicomFileTable, TextureTable const& textureTable, CmguiManager const& manager, ImageBrowseWindowClient& client)
//:
//		dicomFileTable_(dicomFileTable),
//		textureTable_(textureTable),
//		cmguiManager_(manager),
//		client_(client)
//{	
//}

ImageBrowseWindow::ImageBrowseWindow(ImageBrowser<ImageBrowseWindow, CmguiManager>& browser, CmguiManager const& manager)
:
		browser_(browser),
		cmguiManager_(manager)
{}

void ImageBrowseWindow::LoadWindowLayout()
{
	wxXmlResource::Get()->Load(wxT("ImageBrowseWindow.xrc"));
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ImageBrowseWindow"));
	Show(true); // gtk crashes without this
	imageTable_ = XRCCTRL(*this, "ImageTable", wxListCtrl);
	annotationTable_ = XRCCTRL(*this, "AnnotationTable", wxListCtrl);
}

void ImageBrowseWindow::CreatePreviewPanel()
{
	wxPanel* panel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	sceneViewer_ = cmguiManager_.CreateSceneViewer(panel, IMAGE_PREVIEW);
	Cmiss_scene_viewer_add_input_callback(sceneViewer_,
			dummy_input_callback, (void*)this, 1/*add_first*/);

	LoadImagePlaneModel();
}

void ImageBrowseWindow::FitWindow()
{
	// fit the window to meet the size requirements of its children
	this->Fit();
	
	// This stops the window from getting too long in height 
	// when there are many items in the Image Table
	this->SetSize(-1, 768);
	this->Centre();
	
	// Hack for working around bug ID: 3053989
	// see https://sourceforge.net/tracker/?func=detail&aid=3053989&group_id=237340&atid=1103258 for more info
#ifdef WIN32	
	this->Maximize(true);
#endif
}

void ImageBrowseWindow::CreateImageTableColumns()
{	
	long columnIndex = 0;
	imageTable_->InsertColumn(columnIndex++, _("Series #"), wxLIST_FORMAT_CENTRE, 75);
	imageTable_->InsertColumn(columnIndex++, _("Series Description"), wxLIST_FORMAT_CENTRE, -1);
	imageTable_->InsertColumn(columnIndex++, _("Sequence Name"), wxLIST_FORMAT_CENTRE, 120);
//	imageTable_->InsertColumn(columnIndex++, _("Series Time"));
	imageTable_->InsertColumn(columnIndex++, _("Images"), wxLIST_FORMAT_CENTRE, 75);
	imageTable_->InsertColumn(columnIndex, _("Label"), wxLIST_FORMAT_CENTRE, 85);
}

void ImageBrowseWindow::CreateAnnotationTableColumns()
{
	long columnIndex = 0;
	annotationTable_->InsertColumn(columnIndex++, _("Label"), wxLIST_FORMAT_CENTRE, 100);
	annotationTable_->InsertColumn(columnIndex++, _("RID"), wxLIST_FORMAT_CENTRE, 100);
	annotationTable_->InsertColumn(columnIndex++, _("Scope"), wxLIST_FORMAT_CENTRE, 100);	
}

ImageBrowseWindow::~ImageBrowseWindow()
{
	Cmiss_scene_viewer_destroy(&sceneViewer_);
	Cmiss_context_execute_command(cmguiManager_.GetCmissContext(),
			("gfx destroy scene " + IMAGE_PREVIEW).c_str());
	//TODO : destroy textures??
}

void ImageBrowseWindow::CreateProgressDialog(std::string const& title, std::string const& message, int max)
{
	progressDialogPtr_.reset(new wxProgressDialog(wxString(title.c_str(),wxConvUTF8), wxString(message.c_str(),wxConvUTF8), max, this));
}

void ImageBrowseWindow::UpdateProgressDialog(int count)
{
	progressDialogPtr_->Update(count);
}

void ImageBrowseWindow::DestroyProgressDialog()
{
	progressDialogPtr_.reset(0);
}

void ImageBrowseWindow::PopulateImageTableRow(int rowNumber,
		int seriesNumber, std::string const& seriesDescription,
		std::string const& sequenceName, size_t numImages,
		long int const& userDataPtr)
{
	using namespace std;
	using namespace boost;
	long itemIndex = imageTable_->InsertItem(rowNumber, wxString::Format(wxT("%i"),seriesNumber));
	long columnIndex = 1;
	imageTable_->SetItem(itemIndex, columnIndex++, wxString(seriesDescription.c_str(),wxConvUTF8));
	imageTable_->SetItem(itemIndex, columnIndex++, wxString(sequenceName.c_str(),wxConvUTF8));
//		double triggerTime = image->GetTriggerTime();
//		std::string seriesTime = triggerTime < 0 ? "" : lexical_cast<string>(image->GetTriggerTime());// fix
//		imageTable_->SetItem(itemIndex, columnIndex++, seriesTime.c_str());
	imageTable_->SetItem(itemIndex, columnIndex++, wxString::Format(wxT("%i"),(int) numImages));
	
	imageTable_->SetItemData(itemIndex, userDataPtr); // Check !! is this safe??!!
}

void ImageBrowseWindow::SelectFirstRowInImageTable()
{
	imageTable_->SetItemState(0 , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	imageTable_->SetItemState(0 , wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void ImageBrowseWindow::ClearImageTable()
{
	imageTable_->ClearAll();
}

void ImageBrowseWindow::ClearAnnotationTable()
{
	annotationTable_->ClearAll();
}

void ImageBrowseWindow::PopulateAnnotationTableRow(int rowNumber, std::string const& label, std::string const& rid, std::string const& scope)
{
	long itemIndex = annotationTable_->InsertItem(rowNumber, wxString(label.c_str(),wxConvUTF8));
	long columnIndex = 1;
	annotationTable_->SetItem(itemIndex, columnIndex++, wxString(rid.c_str(),wxConvUTF8));
	annotationTable_->SetItem(itemIndex, columnIndex++, wxString(scope.c_str(),wxConvUTF8));
}

void ImageBrowseWindow::SetAnimationSliderMax(size_t max)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	assert(slider);	
	if (max == 1)
	{
		// special case where a slice contains only 1 image
		slider->Enable(false);
		slider->SetMin(1);
		slider->SetMax(2); // gtk requires max > min
		slider->SetValue(1);
	}
	else
	{
		slider->Enable(true);
		slider->SetMin(1);
		slider->SetMax(max);
		slider->SetValue(1);
	}
}

void ImageBrowseWindow::ResizePreviewImage(int width, int height)
{
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

	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

void ImageBrowseWindow::LoadImagePlaneModel()
{	
	cmguiManager_.ReadRectangularModelFiles(IMAGE_PREVIEW, IMAGE_PREVIEW);	
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

std::string ImageBrowseWindow::GetCellContentsString( long row_number, int column ) const
{
	wxListItem     row_info;  
	wxString       cell_contents_string;
	
	// Set what row it is (m_itemId is a member of the regular wxListCtrl class)
	row_info.m_itemId = row_number;
	// Set what column of that row we want to query for information.
	row_info.m_col = column;
	// Set text mask
	row_info.m_mask = wxLIST_MASK_TEXT;
	
	// Get the info and store it in row_info variable.   
	imageTable_->GetItem( row_info );
	
	// Extract the text out that cell
	cell_contents_string = row_info.m_text; 
	
	return std::string(cell_contents_string.mb_str());
}

void ImageBrowseWindow::SetInfoField(std::string const& fieldName, std::string const& data)
{
	long int id = wxXmlResource::GetXRCID(wxString(fieldName.c_str(),wxConvUTF8));
	wxStaticText* st = wxStaticCast((*this).FindWindow(id), wxStaticText);
	//wxStaticText* st = XRCCTRL(*this, wxString(fieldName.c_str(), wxConvUTF8), wxStaticText);
	st->SetLabel(wxString(data.c_str(),wxConvUTF8));
}

void ImageBrowseWindow::FitSceneViewer(double radius)
{
	double centre_x, centre_y, centre_z, clip_factor, //radius,
		size_x, size_y, size_z, width_factor;
	int return_code;

	if (sceneViewer_)
	{	
		Scene_get_graphics_range(Scene_viewer_get_scene(sceneViewer_),
			&centre_x,&centre_y,&centre_z,&size_x,&size_y,&size_z);
//		radius = 0.3*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);
		
		/* enlarge radius to keep image within edge of window */
		/*???RC width_factor should be read in from defaults file */
		width_factor = 1.05;
		radius *= width_factor;
			
		/*???RC clip_factor should be read in from defaults file: */
		clip_factor = 10.0;		
		return_code = Scene_viewer_set_view_simple(sceneViewer_, centre_x, centre_y,
			centre_z, radius, 40, clip_factor*radius);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_view_all.  Invalid argument(s)");
		return_code=0;
	}
	
	if (return_code == 0)
	{
		throw std::logic_error("Invalid arguments");
	}

	return;
}

void ImageBrowseWindow::RefreshPreviewPanel()
{
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
}

void ImageBrowseWindow::OnImageTableItemSelected(wxListEvent& event)
{	
	browser_.OnImageTableItemSelected(event.GetItem().GetData());
}

void ImageBrowseWindow::OnPlayToggleButtonPressed(wxCommandEvent& event)
{
}

void ImageBrowseWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	int value = event.GetInt();
	int textureIndex = value - 1; // tex index is 0 based while slider value is 1 based
//	std::cout << "textureIndex = " << textureIndex << '\n';
////	Time_keeper_request_new_time(timeKeeper_, time);
	browser_.OnAnimationSliderEvent(textureIndex);
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

void ImageBrowseWindow::PutLabelOnSelectedSlice(std::string const& label)
{
	std::cout << __func__ << '\n';
	long index = imageTable_->GetNextItem(-1,
						wxLIST_NEXT_ALL,
						wxLIST_STATE_SELECTED);
	if (index != -1)
	{
		SetImageTableRowLabel(index, label);
		if (index < imageTable_->GetItemCount() - 1)
		{
			imageTable_->SetItemState(index  , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			imageTable_->SetItemState(index+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
		else //HACK to refresh the table
		{
			imageTable_->SetItemState(index , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			imageTable_->SetItemState(index , wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
	}
}

void ImageBrowseWindow::SetImageTableRowLabel(long int index, std::string const& label)
{
	imageTable_->SetItem(index, LABEL_COLUMN_INDEX, wxString(label.c_str(),wxConvUTF8));
//		std::cout <<  "label = " << GetCellContentsString(index, LABEL_COLUMN_INDEX) << '\n';
}

void ImageBrowseWindow::SetImageTableRowLabelByUserData(long int userDataPtr, std::string const& label)
{
	long index = imageTable_->GetItemCount() -1;
	while (index >= 0)
	{
		long ptr = imageTable_->GetItemData(index);
		if (ptr == userDataPtr)
		{
//			std::cout << __func__ << ": found matchig userData, row index = " << index <<'\n';
			SetImageTableRowLabel(index, label);
			return;
		}
		index--;
	}
	
	throw std::invalid_argument("No such user date in Image Table");
	return;
}

void ImageBrowseWindow::OnShortAxisButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	browser_.OnShortAxisButtonEvent();
}

void ImageBrowseWindow::OnLongAxisButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	browser_.OnLongAxisButtonEvent();
}

void ImageBrowseWindow::OnNoneButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	browser_.OnNoneButtonEvent();
}

std::vector<std::pair<std::string, long int> > ImageBrowseWindow::GetListOfLabelsFromImageTable() const
{
	std::vector<std::pair<std::string, long int> > labels;
	
	long index = imageTable_->GetItemCount() - 1;
	while (index >= 0) // iterate from the bottom of the list ( to be compatible with CIM's setup)
	{
		std::string label = GetCellContentsString(index, LABEL_COLUMN_INDEX);
		std::cout << "index = " << index << ", label = " << label << '\n';		
		if (label.length())
		{
			long ptr = imageTable_->GetItemData(index);
			labels.push_back(std::make_pair(label, ptr));
		}
		index--;
	}
	
	return labels;
}

void ImageBrowseWindow::CreateMessageBox(std::string const& message, std::string const& caption)
{
	wxMessageBox(wxString(message.c_str(),wxConvUTF8), wxString(caption.c_str(),wxConvUTF8), wxOK, this);
}

void ImageBrowseWindow::OnOKButtonEvent(wxCommandEvent& event)
{
	browser_.OnOKButtonEvent();
	
	Close();
}

void ImageBrowseWindow::OnCancelButtonEvent(wxCommandEvent& event)
{
	browser_.OnCancelButtonEvent();
	Close();
}

void ImageBrowseWindow::OnOrderByRadioBox(wxCommandEvent& event)
{
//	std::cout << __func__ << " event.GetInt() = " << event.GetInt() << '\n';
	browser_.OnOrderByRadioBox(event.GetInt());
}

void ImageBrowseWindow::OnCloseImageBrowseWindow(wxCloseEvent& event)
{
	// TODO DO clean up!!
	Destroy();
//	exit(0);
}

//void ImageBrowseWindow::ShowImageAnnotation()
//{	
//	browser_.ShowImageAnnotationCurrentlyOnDisplay();
//}


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
	EVT_BUTTON(XRCID("wxID_OK"), ImageBrowseWindow::OnOKButtonEvent)
	EVT_BUTTON(XRCID("wxID_CANCEL"), ImageBrowseWindow::OnCancelButtonEvent)
	EVT_RADIOBOX(XRCID("OrderByRadioBox"), ImageBrowseWindow::OnOrderByRadioBox)
	EVT_CLOSE(ImageBrowseWindow::OnCloseImageBrowseWindow)
END_EVENT_TABLE()

} // end namespace cap
