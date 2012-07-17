/*
 * ImageBrowserWindow.cpp
 *
 *  Created on: Jun 17, 2010
 *      Author: jchu014
 */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <set>

#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/slider.h>
#include <wx/dir.h>
#include <wx/progdlg.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include "images/capicon.xpm"

extern "C"
{
#include <zn/zinc_configure.h>
#include <zn/cmiss_graphics_module.h>
#include <zn/cmiss_scene.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_graphics_material.h>
}

#include "imagebrowserwindow.h"

#include "zinc/extensions.h"
#include "dicomimage.h"
#include "material.h"
#include "imagebrowser.h"
#include "utils/debug.h"
#include "logmsg.h"

namespace
{
static long const LABEL_COLUMN_INDEX = 4;

} // end anonymous namespace

namespace cap
{

std::string const ImageBrowserWindow::IMAGE_PREVIEW = std::string("ImagePreview");

ImageBrowserWindow::ImageBrowserWindow(ImageBrowser *browser)
	: browser_(browser)
	, cmissContext_(Cmiss_context_create(IMAGE_PREVIEW.c_str()))
	, cmguiPanel_(0)
	, material_(0)
{
	progressDialogPtr_.reset(0);
	Cmiss_context_enable_user_interface(cmissContext_, static_cast<void*>(wxTheApp));
	cmguiPanel_ = new SceneViewerPanel(cmissContext_, IMAGE_PREVIEW, panel_cmgui);
	SetIcon(wxICON(capicon));

	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_graphics_module_id graphics_module = Cmiss_context_get_default_graphics_module(cmissContext_);
	Cmiss_rendition_id rendition = Cmiss_graphics_module_get_rendition(graphics_module, root_region);
	SetAnnotationString(" ");
	Cmiss_rendition_execute_command(rendition, "point glyph none general size \"2*2*2\" label annotation centre 0.95,0.95,0.0 select_on material default selected_material default normalised_window_fit_left;");

	Cmiss_region_destroy(&root_region);
	Cmiss_graphics_module_destroy(&graphics_module);
	Cmiss_rendition_destroy(&rendition);

	choice_caseList->Enable(false);
	Fit();
	// This stops the window from getting too long in height 
	// when there are many items in the Image Table
	SetSize(-1, 768);
	Centre();
	MakeConnections();
	CreatePreviewScene();
	FindWindowById(XRCID("wxID_OK"))->Enable(false);
}

ImageBrowserWindow::~ImageBrowserWindow()
{
	dbg("ImageBrowserWindow::~ImageBrowserWindow()");
	if (material_)
		delete material_;

	delete cmguiPanel_;
	Cmiss_context_destroy(&cmissContext_);
}

void ImageBrowserWindow::MakeConnections()
{
	Connect(wxID_EXIT, wxEVT_CLOSE_WINDOW, wxCloseEventHandler(ImageBrowserWindow::OnCloseImageBrowserWindow));
	Connect(XRCID("wxID_OK"), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ImageBrowserWindow::OnOKButtonClicked));
	Connect(XRCID("wxID_CANCEL"), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ImageBrowserWindow::OnCancelButtonClicked));
	Connect(listCtrl_imageTable->GetId(), wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(ImageBrowserWindow::OnImageTableItemSelected));
	Connect(slider_previewSelection->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(ImageBrowserWindow::OnPreviewSelectionSliderEvent));
	Connect(slider_brightness->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(ImageBrowserWindow::OnBrightnessSliderEvent));
	Connect(slider_contrast->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(ImageBrowserWindow::OnContrastSliderEvent));
	Connect(button_longAxis->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ImageBrowserWindow::OnLongAxisButtonEvent));
	Connect(button_shortAxis->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ImageBrowserWindow::OnShortAxisButtonEvent));
	Connect(button_none->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ImageBrowserWindow::OnNoneButtonEvent));
	Connect(radioBox_orderBy->GetId(), wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(ImageBrowserWindow::OnOrderByRadioBox));
	Connect(choice_caseList->GetId(), wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler(ImageBrowserWindow::OnCaseSelected));

	Connect(XRCID("menuItem_close_"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ImageBrowserWindow::OnClose));
	Connect(XRCID("menuItem_openImages_"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ImageBrowserWindow::OnOpenImages));
	Connect(XRCID("menuItem_openArchive_"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ImageBrowserWindow::OnOpenArchive));
	Connect(XRCID("menuItem_openAnnotation_"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(ImageBrowserWindow::OnOpenAnnotation));

}

void ImageBrowserWindow::CreateImageTableColumns()
{	
	long columnIndex = 0;
	listCtrl_imageTable->InsertColumn(columnIndex++, _("Series #"), wxLIST_FORMAT_CENTRE, 75);
	listCtrl_imageTable->InsertColumn(columnIndex++, _("Series Description"), wxLIST_FORMAT_CENTRE, -1);
	listCtrl_imageTable->InsertColumn(columnIndex++, _("Sequence Name"), wxLIST_FORMAT_CENTRE, 120);
//	listCtrl_imageTable->InsertColumn(columnIndex++, _("Series Time"));
	listCtrl_imageTable->InsertColumn(columnIndex++, _("Images"), wxLIST_FORMAT_CENTRE, 75);
	listCtrl_imageTable->InsertColumn(columnIndex, _("Label"), wxLIST_FORMAT_CENTRE, 85);
}

void ImageBrowserWindow::CreateAnnotationTableColumns()
{
	long columnIndex = 0;
	listCtrl_annotationTable->InsertColumn(columnIndex++, _("Label"), wxLIST_FORMAT_CENTRE, 100);
	listCtrl_annotationTable->InsertColumn(columnIndex++, _("RID"), wxLIST_FORMAT_CENTRE, 100);
	listCtrl_annotationTable->InsertColumn(columnIndex++, _("Scope"), wxLIST_FORMAT_CENTRE, 100);
}

void ImageBrowserWindow::CreateProgressDialog(std::string const& title, std::string const& message, int max)
{
	progressDialogPtr_.reset(new wxProgressDialog(wxString(title.c_str(),wxConvUTF8), wxString(message.c_str(),wxConvUTF8), max, this));
}

void ImageBrowserWindow::UpdateProgressDialog(int count)
{
	progressDialogPtr_->Update(count);
}

void ImageBrowserWindow::DestroyProgressDialog()
{
	progressDialogPtr_.reset(0);
}

void ImageBrowserWindow::SetAnnotationString(std::string text)
{
	std::string field_param = "string_constant '" + text + "'";
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "/");
	Cmiss_field_module_define_field(field_module, "annotation", field_param.c_str());

	Cmiss_field_module_destroy(&field_module);
}

void ImageBrowserWindow::PopulateImageTableRow(int rowNumber,
		int seriesNumber, std::string const& seriesDescription,
		std::string const& sequenceName, size_t numImages,
		long int const& userDataPtr)
{
	using namespace std;
	using namespace boost;
	long itemIndex = listCtrl_imageTable->InsertItem(rowNumber, wxString::Format(wxT("%i"),seriesNumber));
	long columnIndex = 1;
	listCtrl_imageTable->SetItem(itemIndex, columnIndex++, wxString(seriesDescription.c_str(),wxConvUTF8));
	listCtrl_imageTable->SetItem(itemIndex, columnIndex++, wxString(sequenceName.c_str(),wxConvUTF8));
	listCtrl_imageTable->SetItem(itemIndex, columnIndex++, wxString::Format(wxT("%i"),(int) numImages));
	
	listCtrl_imageTable->SetItemData(itemIndex, userDataPtr); // Check !! is this safe??!!
}

void ImageBrowserWindow::SelectFirstRowInImageTable()
{
	//listCtrl_imageTable->SetItemState(0 , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	listCtrl_imageTable->SetItemState(0 , wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void ImageBrowserWindow::ClearImageTable()
{
	listCtrl_imageTable->ClearAll();
}

void ImageBrowserWindow::ClearAnnotationTable()
{
	listCtrl_annotationTable->ClearAll();
}

void ImageBrowserWindow::PopulateAnnotationTableRow(int rowNumber, std::string const& label, std::string const& rid, std::string const& scope)
{
	long itemIndex = listCtrl_annotationTable->InsertItem(rowNumber, wxString(label.c_str(),wxConvUTF8));
	long columnIndex = 1;
	listCtrl_annotationTable->SetItem(itemIndex, columnIndex++, wxString(rid.c_str(),wxConvUTF8));
	listCtrl_annotationTable->SetItem(itemIndex, columnIndex++, wxString(scope.c_str(),wxConvUTF8));
}

void ImageBrowserWindow::SetAnimationSliderMax(size_t max)
{
	
	assert(slider_previewSelection);
	if (max == 1)
	{
		// special case where a slice contains only 1 image
		slider_previewSelection->Enable(false);
		slider_previewSelection->SetMin(1);
		slider_previewSelection->SetMax(2); // gtk requires max > min
		slider_previewSelection->SetValue(1);
	}
	else
	{
		slider_previewSelection->Enable(true);
		slider_previewSelection->SetMin(1);
		slider_previewSelection->SetMax(max);
		slider_previewSelection->SetValue(1);
	}
}

void ImageBrowserWindow::ResizePreviewImage(double width, double height)
{
	ResizePlaneElement(cmissContext_, IMAGE_PREVIEW, width, height);
}

void ImageBrowserWindow::ChangePreviewImage(Cmiss_field_image_id image)
{
	assert(material_);
	material_->ChangeTexture(image);
}

void ImageBrowserWindow::CreatePreviewScene()
{
	Cmiss_graphics_module_id gModule = Cmiss_context_get_default_graphics_module(cmissContext_);
	// boost::make_pair is faster than shared_ptr<Material>(new )
	material_ = new Material(IMAGE_PREVIEW, gModule);//boost::make_shared<Material>(IMAGE_PREVIEW, gModule);
	
	CreatePlaneElement(cmissContext_, IMAGE_PREVIEW);
	Cmiss_graphics_material_id graphics_material = material_->GetCmissMaterial();
	CreateTextureImageSurface(cmissContext_, IMAGE_PREVIEW, graphics_material);
	cmguiPanel_->SetTumbleRate(0.0);
	cmguiPanel_->ViewAll();

	Cmiss_graphics_material_destroy(&graphics_material);
	Cmiss_graphics_module_destroy(&gModule);
}

Cmiss_field_image_id ImageBrowserWindow::CreateFieldImage(const std::string& filename)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, IMAGE_PREVIEW);
	Cmiss_field_image_id image_field = Cmiss_field_module_create_image_texture(field_module, filename);
	Cmiss_field_module_destroy(&field_module);
	
	return image_field;
}

std::string ImageBrowserWindow::GetCellContentsString( long row_number, int column ) const
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
	listCtrl_imageTable->GetItem( row_info );
	
	// Extract the text out that cell
	cell_contents_string = row_info.m_text; 
	
	return std::string(cell_contents_string.mb_str());
}

void ImageBrowserWindow::SetInfoField(std::string const& fieldName, std::string const& data)
{
	if(fieldName == "PatientName")
		staticText_patientName->SetLabel(data.c_str());
	else if(fieldName == "PatientID")
		staticText_patientID->SetLabel(data.c_str());
	else if(fieldName == "ScanDate")
		staticText_scanDate->SetLabel(data.c_str());
	else if(fieldName == "DateOfBirth")
		staticText_dateOfBirth->SetLabel(data.c_str());
	else if(fieldName == "GenderAndAge")
		staticText_genderAndAge->SetLabel(data.c_str());
	else if(fieldName == "ImageSize")
		staticText_imageSize->SetLabel(data.c_str());
	else if(fieldName == "ImagePosition")
		staticText_imagePosition->SetLabel(data.c_str());
	else
	{
        LOG_MSG(LOGWARNING) << "Warning: ImageBrowserWindow::SetInfoField() Unknown fieldName - " << fieldName;
	}
}

void ImageBrowserWindow::FitSceneViewer(double radius)
{
	cmguiPanel_->SetViewingVolume(radius);
}

void ImageBrowserWindow::OnClose(wxCommandEvent& /*event*/)
{
	browser_->OnCancelButtonClicked();
}

void ImageBrowserWindow::OnOpenArchive(wxCommandEvent& /*event*/)
{
	browser_->ChooseArchiveFile();
}

void ImageBrowserWindow::OnOpenAnnotation(wxCommandEvent& /*event*/)
{
	browser_->ChooseAnnotationFile();
}

void ImageBrowserWindow::OnOpenImages(wxCommandEvent& /*event*/)
{
	browser_->ChooseImageDirectory();
}

void ImageBrowserWindow::OnImageTableItemSelected(wxListEvent& event)
{	
	browser_->OnImageTableItemSelected(event.GetItem().GetData());
}

void ImageBrowserWindow::OnPreviewSelectionSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
	int textureIndex = value - 1; // tex index is 0 based while slider value is 1 based
	browser_->ChangePreviewImage(textureIndex);
}

void ImageBrowserWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
	int min = slider_contrast->GetMin();
	int max = slider_contrast->GetMax();

	float brightness = (float)(value - min) / (float)(max - min);
	material_->SetBrightness(brightness);
	
	//RefreshPreviewPanel();
}

void ImageBrowserWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
	int min = slider_contrast->GetMin();
	int max = slider_contrast->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	assert(material_);
	material_->SetContrast(contrast);
	
	//RefreshPreviewPanel();
}

void ImageBrowserWindow::OnShortAxisButtonEvent(wxCommandEvent& WXUNUSED(event))
{
	browser_->OnShortAxisButtonEvent();
	if (IsAtLeastOneImageLabelled())
		FindWindowById(XRCID("wxID_OK"))->Enable(true);
}

void ImageBrowserWindow::OnLongAxisButtonEvent(wxCommandEvent& WXUNUSED(event))
{
	browser_->OnLongAxisButtonEvent();
	if (IsAtLeastOneImageLabelled())
		FindWindowById(XRCID("wxID_OK"))->Enable(true);
}

void ImageBrowserWindow::OnNoneButtonEvent(wxCommandEvent& WXUNUSED(event))
{
	browser_->OnNoneButtonEvent();
	if (!IsAtLeastOneImageLabelled())
		FindWindowById(XRCID("wxID_OK"))->Enable(false);
}

void ImageBrowserWindow::OnOKButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	browser_->OnOKButtonClicked();
}

void ImageBrowserWindow::OnCancelButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	browser_->OnCancelButtonClicked();
}

void ImageBrowserWindow::OnOrderByRadioBox(wxCommandEvent& event)
{
	//std::cout << __func__ << " event.GetInt() = " << event.GetInt() << '\n';
	browser_->OnOrderByRadioBox(event.GetInt());
}

void ImageBrowserWindow::OnCloseImageBrowserWindow(wxCloseEvent& WXUNUSED(event))
{
	browser_->OnCancelButtonClicked();
}

void ImageBrowserWindow::OnCaseSelected(wxCommandEvent& WXUNUSED(event))
{
	dbg("ImageBrowserWindow::OnCaseSelected - Unfortunately case filtering is not yet completed.");
}

void ImageBrowserWindow::SetCaseList(const std::vector<std::string>& cases)
{
	choice_caseList->Clear();
	choice_caseList->Enable(false);
	std::vector<std::string>::const_iterator cit = cases.begin();
	for (; cit != cases.end(); cit++)
		choice_caseList->Append(*cit);

	if (cases.size() > 0)
	{
		choice_caseList->SetSelection(0);
		choice_caseList->Enable(true);
	}
}

void ImageBrowserWindow::PutLabelOnSelectedSlice(std::string const& label)
{
	long index = listCtrl_imageTable->GetNextItem(-1,
						wxLIST_NEXT_ALL,
						wxLIST_STATE_SELECTED);
	if (index != -1)
	{
		SetImageTableRowLabel(index, label);
		if (index < listCtrl_imageTable->GetItemCount() - 1)
		{
			listCtrl_imageTable->SetItemState(index  , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			listCtrl_imageTable->SetItemState(index+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
		else //HACK to refresh the table
		{
			listCtrl_imageTable->SetItemState(index , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			listCtrl_imageTable->SetItemState(index , wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
	}
}

void ImageBrowserWindow::SetImageTableRowLabel(long int index, std::string const& label)
{
	listCtrl_imageTable->SetItem(index, LABEL_COLUMN_INDEX, wxString(label.c_str(),wxConvUTF8));
//		std::cout <<  "label = " << GetCellContentsString(index, LABEL_COLUMN_INDEX) << '\n';
}

void ImageBrowserWindow::SetImageTableRowLabelByUserData(long int userDataPtr, std::string const& label)
{
	long index = listCtrl_imageTable->GetItemCount() -1;
	while (index >= 0)
	{
		long ptr = listCtrl_imageTable->GetItemData(index);
		if (ptr == userDataPtr)
		{
			SetImageTableRowLabel(index, label);
			if (IsAtLeastOneImageLabelled())
				FindWindowById(XRCID("wxID_OK"))->Enable(true);
			return;
		}
		index--;
	}
	
	throw std::invalid_argument("No such user data in Image Table");
	return;
}

bool ImageBrowserWindow::IsAtLeastOneImageLabelled() const
{
	bool oneImageLabelled = false;
	long index = listCtrl_imageTable->GetItemCount() - 1;
	while(index >= 0 && !oneImageLabelled)
	{
		std::string label = GetCellContentsString(index, LABEL_COLUMN_INDEX);
		if (label.length() > 0)
			oneImageLabelled = true;
		else
			index--;
	}

	return oneImageLabelled;
}

std::vector<std::pair<std::string, long int> > ImageBrowserWindow::GetListOfLabelsFromImageTable() const
{
	std::vector<std::pair<std::string, long int> > labels;
	
	long index = listCtrl_imageTable->GetItemCount() - 1;
	while (index >= 0) // iterate from the bottom of the list ( to be compatible with CIM's setup)
	{
		std::string label = GetCellContentsString(index, LABEL_COLUMN_INDEX);
		if (label.length())
		{
			long ptr = listCtrl_imageTable->GetItemData(index);
			labels.push_back(std::make_pair(label, ptr));
		}
		index--;
	}
	
	return labels;
}

void ImageBrowserWindow::CreateMessageBox(std::string const& message, std::string const& caption)
{
	wxMessageBox(wxString(message.c_str(),wxConvUTF8), wxString(caption.c_str(),wxConvUTF8), wxOK, this);
}

} // end namespace cap
