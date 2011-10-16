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
#include <configure/cmgui_configure.h>
#include <api/cmiss_graphics_module.h>
#include <api/cmiss_scene.h>
#include <api/cmiss_field.h>
#include <api/cmiss_field_module.h>
}

#include "imagebrowserwindow.h"

#include "cmgui/utilities.h"
#include "CmguiExtensions.h"
#include "DICOMImage.h"
#include "material.h"
#include "imagebrowser.h"

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
{
	Cmiss_context_enable_user_interface(cmissContext_, static_cast<void*>(wxTheApp));
	cmguiPanel_ = new CmguiPanel(cmissContext_, IMAGE_PREVIEW, panel_cmgui);
	
	SetIcon(wxICON(capicon));

	Fit();
	// This stops the window from getting too long in height 
	// when there are many items in the Image Table
	SetSize(-1, 768);
	Centre();
	// Work around for bug ID: 3053989
	// see https://sourceforge.net/tracker/?func=detail&aid=3053989&group_id=237340&atid=1103258 for more info
	#ifdef WIN32
	this->Maximize(true);
	#endif
	MakeConnections();
	CreatePreviewScene();
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

ImageBrowserWindow::~ImageBrowserWindow()
{
	std::cout << "ImageBrowserWindow::~ImageBrowserWindow()" << std::endl;
	delete cmguiPanel_;
	Cmiss_context_destroy(&cmissContext_);
	//Cmiss_context_execute_command(cmguiPanel_->GetCmissContext(),
	//		("gfx destroy scene " + IMAGE_PREVIEW).c_str());
	//TODO : destroy textures??
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
	material_ = boost::make_shared<Material>(IMAGE_PREVIEW, gModule);
	
	CreatePlaneElement(cmissContext_, IMAGE_PREVIEW);
	CreateTextureImageSurface(cmissContext_, IMAGE_PREVIEW, material_->GetCmissMaterial());
	cmguiPanel_->SetTumbleRate(0.0);
	cmguiPanel_->ViewAll();
}

Cmiss_field_image_id ImageBrowserWindow::CreateFieldImage(DICOMPtr dicom)
{
	Cmiss_field_module_id field_module = GetFieldModuleForRegion(cmissContext_, IMAGE_PREVIEW);
	Cmiss_field_image_id image_field = CreateCmissImageTexture(field_module, dicom);
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
		std::cout << "Warning: ImageBrowserWindow::SetInfoField() Unknown fieldName - " << fieldName << std::endl;
	}
}

void ImageBrowserWindow::FitSceneViewer(double radius)
{
	cmguiPanel_->SetViewingVolume(radius);
}

void ImageBrowserWindow::RefreshPreviewPanel()
{
	cmguiPanel_->RedrawNow();
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
	
	RefreshPreviewPanel();
}

void ImageBrowserWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
	int min = slider_contrast->GetMin();
	int max = slider_contrast->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	assert(material_);
	material_->SetContrast(contrast);
	
	RefreshPreviewPanel();
}

void ImageBrowserWindow::OnShortAxisButtonEvent(wxCommandEvent& WXUNUSED(event))
{
	//	std::cout << __func__ << '\n';
	browser_->OnShortAxisButtonEvent();
}

void ImageBrowserWindow::OnLongAxisButtonEvent(wxCommandEvent& WXUNUSED(event))
{
	//	std::cout << __func__ << '\n';
	browser_->OnLongAxisButtonEvent();
}

void ImageBrowserWindow::OnNoneButtonEvent(wxCommandEvent& WXUNUSED(event))
{
	//	std::cout << __func__ << '\n';
	browser_->OnNoneButtonEvent();
}

void ImageBrowserWindow::OnOKButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	browser_->OnOKButtonClicked();
	Close();
}

void ImageBrowserWindow::OnCancelButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	browser_->OnCancelButtonClicked();
	Close();
}

void ImageBrowserWindow::OnOrderByRadioBox(wxCommandEvent& event)
{
	//std::cout << __func__ << " event.GetInt() = " << event.GetInt() << '\n';
	browser_->OnOrderByRadioBox(event.GetInt());
}

void ImageBrowserWindow::OnCloseImageBrowserWindow(wxCloseEvent& WXUNUSED(event))
{
	// TODO DO clean up!!
	Destroy();
	//	exit(0);
}

void ImageBrowserWindow::OnCaseSelected(wxCommandEvent& WXUNUSED(event))
{
	std::cout << "ImageBrowserWindow::OnCaseSelected" << std::endl;
}

void ImageBrowserWindow::PutLabelOnSelectedSlice(std::string const& label)
{
	std::cout << "ImageBrowserWindow::" << __func__ << std::endl;
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
//			std::cout << __func__ << ": found matchig userData, row index = " << index <<'\n';
			SetImageTableRowLabel(index, label);
			return;
		}
		index--;
	}
	
	throw std::invalid_argument("No such user date in Image Table");
	return;
}

std::vector<std::pair<std::string, long int> > ImageBrowserWindow::GetListOfLabelsFromImageTable() const
{
	std::vector<std::pair<std::string, long int> > labels;
	
	long index = listCtrl_imageTable->GetItemCount() - 1;
	while (index >= 0) // iterate from the bottom of the list ( to be compatible with CIM's setup)
	{
		std::string label = GetCellContentsString(index, LABEL_COLUMN_INDEX);
		std::cout << "index = " << index << ", label = " << label << '\n';		
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
