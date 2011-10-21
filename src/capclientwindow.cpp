
#include <iostream>
#include <sstream>

#include <wx/xrc/xmlres.h>
#include <wx/dir.h>

//#include "wx/splitter.h"
#include <wx/aboutdlg.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#include <wx/button.h>

#include <boost/foreach.hpp>

extern "C"
{
#include <configure/cmgui_configure.h>
#include <api/cmiss_context.h>
#include <api/cmiss_field.h>
}

#include "utils/debug.h"
#include "capclientconfig.h"
#include "capclient.h"
#include "capclientapp.h"
#include "capclientwindow.h"
#include "UserCommentDialog.h"
#include "CAPHtmlWindow.h"
#include "CAPBinaryVolumeParameterDialog.h"
#include "cmgui/utilities.h"
#include "material.h"
#include "cmguicallbacks.h"


#include "images/capicon.xpm"

using namespace std;

namespace cap 
{

namespace
{
	const char* ModeStrings[] = 
	{
		"Apex",
		"Base",
		"RV Inserts",
		"Baseplane Points",
		"Guide Points"
	};//REVISE
}

CAPClientWindow::CAPClientWindow(CAPClient* mainApp)
	: CAPClientWindowUI()
	, mainApp_(mainApp)
	, cmissContext_(Cmiss_context_create("CAPClient"))
	, cmguiPanel_(0)
	, timeKeeper_(0)
	, timeNotifier_(0)
	, previousSaveLocation_("")
	, initialised_xmlUserCommentDialog_(false)
{
	Cmiss_context_enable_user_interface(cmissContext_, static_cast<void*>(wxTheApp));
	timeKeeper_ = Cmiss_context_get_default_time_keeper(cmissContext_);
	Cmiss_time_keeper_set_repeat_mode(timeKeeper_, CMISS_TIME_KEEPER_REPEAT_MODE_PLAY_LOOP);
	Cmiss_time_keeper_set_frame_mode(timeKeeper_, CMISS_TIME_KEEPER_FRAME_MODE_PLAY_REAL_TIME);

	cmguiPanel_ = new CmguiPanel(cmissContext_, "CAPClient", panel_Cmgui);
	SetIcon(wxICON(capicon));
	
	// GUI initialization
	//CreateStatusBar(0);
	//UpdateFrameNumber(0);

	// Initialize check box list of scene objects (image slices)
	//objectList_ = XRCCTRL(*this, "SliceList", wxCheckListBox);
	//objectList_->SetSelection(wxNOT_FOUND);
	//objectList_->Clear();
	//m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	
	checkListBox_Slice->SetSelection(wxNOT_FOUND);
	checkListBox_Slice->Clear();
	
	this->Fit();
	this->Centre();
	MakeConnections();
	EnterInitState();
}

CAPClientWindow::~CAPClientWindow()
{
	dbg(__func__);
	delete cmguiPanel_;

	Cmiss_context_destroy(&cmissContext_);
	Cmiss_time_keeper_destroy(&timeKeeper_);
	Cmiss_time_notifier_destroy(&timeNotifier_);
}

void CAPClientWindow::MakeConnections()
{
	cout << "CAPClientWindow::" << __func__ << endl;
	// Menus
	Connect(XRCID("menuItem_About"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnAbout));
	Connect(XRCID("menuItem_Quit"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnQuit));
	Connect(XRCID("menuItem_OpenImages"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnOpenImages));
	Connect(XRCID("menuItem_OpenModel"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnOpenModel));
	Connect(XRCID("menuItem_OpenAnnotation"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnOpenAnnotation));
	Connect(XRCID("menuItem_Save"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnSave));
	Connect(XRCID("menuItem_Export"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnExportModel));
	Connect(XRCID("menuItem_ExportToBinaryVolume"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(CAPClientWindow::OnExportModelToBinaryVolume));
	
	// Widgets (buttons, sliders ...)
	Connect(button_Play->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnTogglePlay));
	Connect(button_HideShowAll->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnToggleHideShowAll));
	Connect(button_HideShowOthers->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnToggleHideShowOthers));
	Connect(slider_Brightness->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnBrightnessSliderEvent));
	Connect(slider_Contrast->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnContrastSliderEvent));
	Connect(slider_Animation->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnAnimationSliderEvent));
	Connect(slider_AnimationSpeed->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, wxCommandEventHandler(CAPClientWindow::OnAnimationSpeedControlEvent));
	Connect(checkListBox_Slice->GetId(), wxEVT_COMMAND_LISTBOX_SELECTED, wxListEventHandler(CAPClientWindow::OnObjectCheckListSelected));
	Connect(checkListBox_Slice->GetId(), wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxListEventHandler(CAPClientWindow::OnObjectCheckListChecked));
	
	Connect(wxEVT_IDLE, wxIdleEventHandler(CAPClientWindow::OnIdle), 0, this);
	//Connect(wxEVT_QUIT, 
}
// BEGIN_EVENT_TABLE(CAPClientWindow, CAPClientWindowUI)
// 	EVT_BUTTON(XRCID("button_PlayButton"),CAPClientWindow::OnTogglePlay) // play button
// 	EVT_SLIDER(XRCID("slider_Animation"),CAPClientWindow::OnAnimationSliderEvent) // animation slider
// 	EVT_SLIDER(XRCID("slider_AnimationSpeed"),CAPClientWindow::OnAnimationSpeedControlEvent)
// 	EVT_CHECKLISTBOX(XRCID("checkListBox_Slice"), CAPClientWindow::OnObjectCheckListChecked)
// 	EVT_BUTTON(XRCID("button_HideShowAll"),CAPClientWindow::OnToggleHideShowAll) // hide all button
// 	EVT_BUTTON(XRCID("button_HideShowOthers"),CAPClientWindow::OnToggleHideShowOthers) // hide others button
// 	EVT_CHECKBOX(XRCID("checkBox_MII"),CAPClientWindow::OnMIICheckBox)
// 	EVT_CHECKBOX(XRCID("checkBox_Wireframe"),CAPClientWindow::OnWireframeCheckBox)
// 	EVT_LISTBOX(XRCID("checkListBox_Slice"), CAPClientWindow::OnObjectCheckListSelected)
// 	EVT_SLIDER(XRCID("slider_BrightnessSlider"),CAPClientWindow::OnBrightnessSliderEvent)
// 	EVT_SLIDER(XRCID("slider_ContrastSlider"),CAPClientWindow::OnContrastSliderEvent)
// 	EVT_BUTTON(XRCID("button_AcceptButton"),CAPClientWindow::OnAcceptButtonPressed)
// 	EVT_CHOICE(XRCID("choice_Mode"),CAPClientWindow::OnModellingModeChanged)
// 	EVT_CLOSE(CAPClientWindow::Terminate)
// 	EVT_MENU(XRCID("menuItem_OpenImagesMenuItem"), CAPClientWindow::OnOpenImages)
// 	EVT_MENU(XRCID("menuItem_OpenModelMenuItem"), CAPClientWindow::OnOpenModel)
// 	EVT_MENU(XRCID("menuItem_OpenAnnotationMenuItem"), CAPClientWindow::OnOpenAnnotation)
// 	EVT_MENU(XRCID("menuItem_SaveMenuItem"), CAPClientWindow::OnSave)
// 	EVT_MENU(XRCID("menuItem_ExportMenuItem"), CAPClientWindow::OnExportModel)
// 	EVT_MENU(XRCID("menuItem_ExportToBinaryVolumeMenuItem"), CAPClientWindow::OnExportModelToBinaryVolume)
// 	EVT_BUTTON(XRCID("button_PlaneShiftButton"), CAPClientWindow::OnPlaneShiftButtonPressed)
// END_EVENT_TABLE()

void CAPClientWindow::EnterInitState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	// Put the gui in the init state
	slider_Animation->Enable(false);
	slider_AnimationSpeed->Enable(false);
	button_Play->Enable(true);
	button_HideShowAll->Enable(false);
	button_HideShowOthers->Enable(false);
	checkBox_MII->Enable(false);
	checkBox_MII->SetValue(false);
	checkBox_Wireframe->Enable(false);
	checkBox_Wireframe->SetValue(false);
	slider_Contrast->Enable(false);
	slider_Brightness->Enable(false);
	choice_Mode->Enable(false);
	button_Accept->Enable(false);
	button_PlaneShift->Enable(false);

	//wxMenu *fileMenu = menuBar_Main->GetMenu(0);
	menuItem_OpenModel->Enable(true);
	menuItem_Save->Enable(false);
	menuItem_Export->Enable(false);
	menuItem_ExportToBinaryVolume->Enable(false);

//	objectList_->Clear();

//	this->Fit();
//	this->Centre();
	// HACK to make the cmgui scene viewer fit the enclosing wxPanel
	//this->Maximize(true); 
	//this->Maximize(false);
}

void CAPClientWindow::EnterImagesLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	slider_Animation->Enable(true);
	slider_Animation->SetValue(0);
	slider_AnimationSpeed->Enable(true);
	button_Play->Enable(true);
	button_HideShowAll->Enable(true);
	button_HideShowOthers->Enable(true);
	checkBox_MII->Enable(false);
	checkBox_Wireframe->Enable(false);
	slider_Brightness->Enable(true);
	slider_Contrast->Enable(true);
	choice_Mode->Enable(true);
	button_Accept->Enable(true);
	button_PlaneShift->Enable(true);

	menuItem_OpenModel->Enable(true);
	menuItem_Save->Enable(true);
	menuItem_Export->Enable(false);
	menuItem_ExportToBinaryVolume->Enable(false);

	StopCine();
	// Initialize timer for animation
	size_t numberOfLogicalFrames = mainApp_->GetMinimumNumberOfFrames();//-- TODO: set this properly!!! imageSet_->GetNumberOfFrames(); // smallest number of frames of all slices
	if (timeNotifier_)
	{
		Cmiss_time_keeper_remove_time_notifier(timeKeeper_, timeNotifier_);
		Cmiss_time_notifier_destroy(&timeNotifier_);
	}
	timeNotifier_ = Cmiss_time_keeper_create_notifier_regular(timeKeeper_, numberOfLogicalFrames, 0);
	Cmiss_time_notifier_add_callback(timeNotifier_, time_callback, (void*)this);
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_MINIMUM_TIME, 0.0);
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_MAXIMUM_TIME, 1.0);
	//		Time_keeper_set_minimum(timeKeeper_, 0); // FIXME time range is always 0~1
	//		Time_keeper_set_maximum(timeKeeper_, 1);
	
	SetAnimationSliderRange(0, numberOfLogicalFrames-1);
	
	//--gui_->SetTitle(wxString(imageSet_->GetPatientID().c_str(),wxConvUTF8));

}

void CAPClientWindow::EnterModelLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	slider_Animation->Enable(true);
	slider_AnimationSpeed->Enable(true);
	button_Play->Enable(true);
	button_HideShowAll->Enable(true);
	button_HideShowOthers->Enable(true);
	checkBox_MII->Enable(true);
	checkBox_Wireframe->Enable(true);
	checkBox_Wireframe->SetValue(true);
	slider_Brightness->Enable(true);
	slider_Contrast->Enable(true);
	choice_Mode->Enable(true);
	button_Accept->Enable(true);
	button_PlaneShift->Enable(true);

	menuItem_OpenModel->Enable(true);
	menuItem_Save->Enable(true);
	menuItem_Export->Enable(true);
	menuItem_ExportToBinaryVolume->Enable(true);

	//GetWidgetByName<wxCheckBox>("Wireframe")->SetValue(true);
}

void CAPClientWindow::OnIdle(wxIdleEvent& event)
{
	if (Cmiss_context_process_idle_event(cmissContext_))
	{
		event.RequestMore();
	}
}

void CAPClientWindow::CreateCAPIconInContext() const
{
	// Load in a funky picture
	Cmiss_context_execute_command(cmissContext_, "gfx read wave as heart src/heart_model.obj");
	Cmiss_region_id region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region_id icon_region = Cmiss_region_create_child(region, "cap_icon");
	Cmiss_context_execute_command(cmissContext_, "gfx mod g_element \"cap_icon\" point LOCAL glyph heart general size \"0.05*0.05*0.05\" material red");
	cmguiPanel_->LookHere();
	Cmiss_region_destroy(&icon_region);
	Cmiss_region_destroy(&region);
}

double CAPClientWindow::GetCurrentTime() const
{
	return Cmiss_time_keeper_get_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_TIME);
}

Cmiss_scene_viewer_id CAPClientWindow::GetCmissSceneViewer() const
{
	return cmguiPanel_->GetCmissSceneViewer();
}

void CAPClientWindow::AddDataPoint(Cmiss_node* dataPointID, Point3D const& position)
{
	mainApp_->AddDataPoint(dataPointID, position, GetCurrentTime());
}

void CAPClientWindow::MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition)
{
	mainApp_->MoveDataPoint(dataPointID, newPosition, GetCurrentTime());
}

void CAPClientWindow::RemoveDataPoint(Cmiss_node* dataPointID)
{
	mainApp_->RemoveDataPoint(dataPointID, GetCurrentTime());
}

void CAPClientWindow::SmoothAlongTime()
{
	mainApp_->SmoothAlongTime();
}

void CAPClientWindow::PlayCine()
{
	Cmiss_time_keeper_play(timeKeeper_, CMISS_TIME_KEEPER_PLAY_FORWARD);
	button_Play->SetLabel(wxT("stop"));
}

void CAPClientWindow::StopCine()
{
	Cmiss_time_keeper_stop(timeKeeper_);
	button_Play->SetLabel(wxT("play"));
	//wxCommandEvent event;
	//OnAnimationSliderEvent(event); //HACK snap the slider to nearest frame time
}

void CAPClientWindow::OnTogglePlay(wxCommandEvent& event)
{
	dbg("CAPClientWindow::OnTogglePlay");
	//--mainApp_->OnTogglePlay();

	if (button_Play->GetLabel() == wxT("play"))
	{
		// start stuff
		cmguiPanel_->LookingHere();
		PlayCine();
	}
	else
	{

		// stop stuff
		StopCine();
	}
}

void CAPClientWindow::Terminate(wxCloseEvent& event)
{
	cout << "CAPClientWindow::" << __func__ << endl;
	int answer = wxYES; //wxMessageBox(wxT("Quit program?"), wxT("Confirm"),
	                    //        wxYES_NO, this);
	if (answer == wxYES)
	{
//		Destroy();
//		exit(0); //without this, the funny temporary window appears
//		Cmiss_context_execute_command(context_, "QUIT");
		wxExit();
	}
}

void CAPClientWindow::CreateTextureSlice(const LabelledSlice& labelledSlice)
{
	std::string regionName = labelledSlice.GetLabel();
	Cmiss_graphics_module_id gModule = Cmiss_context_get_default_graphics_module(cmissContext_);
	CreatePlaneElement(cmissContext_, regionName);
	// Set the material and field images into a TextureSlice.
	boost::shared_ptr<Material> material = boost::make_shared<Material>(regionName, gModule);
	CreateTextureImageSurface(cmissContext_, regionName, material->GetCmissMaterial());
	std::vector<Cmiss_field_image_id> fieldImages = CreateFieldImages(labelledSlice);
	
	BOOST_FOREACH(DICOMPtr dicom, labelledSlice.GetDICOMImages())
	{
		ImagePlane* plane = dicom->GetImagePlane();
		RepositionPlaneElement(cmissContext_, regionName, plane);
	}
	textureSliceMap_.insert(std::make_pair(regionName, boost::make_shared<TextureSlice>(material, fieldImages)));
	ChangeTexture(regionName, fieldImages.at(0));
	cmguiPanel_->ViewAll();
}

std::vector<Cmiss_field_image_id> CAPClientWindow::CreateFieldImages(const LabelledSlice& labelledSlice)
{
	Cmiss_field_module_id field_module = GetFieldModuleForRegion(cmissContext_, labelledSlice.GetLabel());
	
	std::vector<Cmiss_field_image_id> field_images;
	BOOST_FOREACH(DICOMPtr dicom, labelledSlice.GetDICOMImages())
	{
		Cmiss_field_image_id image_field = CreateCmissImageTexture(field_module, dicom);
		field_images.push_back(image_field);
	}
	Cmiss_field_module_destroy(&field_module);
	
	return field_images;
}

void CAPClientWindow::CreateScene(const std::string& regionName)
{
	CreatePlaneElement(cmissContext_, regionName);
	CreateTextureImageSurface(cmissContext_, regionName, textureSliceMap_[regionName]->GetCmissMaterial());
}

void CAPClientWindow::ChangeTexture(const std::string& name, Cmiss_field_image_id fieldImage)
{
	TextureSliceMap::const_iterator it = textureSliceMap_.find(name);
	if (it != textureSliceMap_.end())
		(*it).second->ChangeTexture(fieldImage);
}

void CAPClientWindow::PopulateSliceList(std::vector<std::string> const& sliceNames, std::vector<bool> const& visibilities)
{
	std::cout << __func__ << '\n';
	checkListBox_Slice->Clear();
	
	size_t index = 0;
	BOOST_FOREACH(std::string const& sliceName, sliceNames)
	{
		std::cout << "Slice name = " << sliceName << '\n';
		checkListBox_Slice->Append(wxString(sliceName.c_str(), wxConvUTF8));
		bool visible = visibilities.at(index);
		/* default selection */
		if ( visible )
		{
			checkListBox_Slice->Check((checkListBox_Slice->GetCount()-1),1);
		}
		checkListBox_Slice->SetSelection(wxNOT_FOUND);
		index ++;
	}
}

void CAPClientWindow::OnObjectCheckListChecked(wxListEvent& event)
{
	int selection = event.GetInt();
	wxString name = checkListBox_Slice->GetString(selection);
	bool visibility = checkListBox_Slice->IsChecked(selection);

	TextureSliceMap::const_iterator cit = textureSliceMap_.find(std::string(name.c_str()));
	if (cit != textureSliceMap_.end())
	{
		SetVisibilityForRegion(cmissContext_, cit->first, visibility);
	}
}

void CAPClientWindow::OnObjectCheckListSelected(wxListEvent& event)
{
	dbg("CAPClientWindow::OnObjectCheckListSelected");
	int selection = event.GetInt();
	wxString name = checkListBox_Slice->GetString(selection);
	//called from OnObjectCheckListSelected
	
	const ImagePlane& plane = mainApp_->GetImagePlane(name.c_str());
	
	cmguiPanel_->SetViewingPlane(plane);
	//gui_->RedrawNow();
}

void CAPClientWindow::SetAnimationSliderRange(int min, int max)
{
	slider_Animation->SetMin(min);
	slider_Animation->SetMax(max);
}

void CAPClientWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	int value = slider_Animation->GetValue();
	int min = slider_Animation->GetMin();
	int max = slider_Animation->GetMax();
	
	//dbg( std::string(__func__) + " : time = " + toString(value) + ", min = " + toString(min) + ", max = " + toString(max) );
	double time =  (double)(value - min) / (double)(max - min);
	
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (; cit != textureSliceMap_.end(); cit++)
	{
		cit->second->ChangeTextureNearestTo(time);
	}
	//--Time_keeper_request_new_time(timeKeeper_, time);
	// mainApp_->OnAnimationSliderEvent(time);
	//--gui_->SetTime(time);
	//-- need this for next 	int frameNumber = heartModelPtr_->MapToModelFrameNumber(time);

	//--gui_->UpdateFrameNumber(frameNumber);
	//--Refresh3DCanvas(); // forces redraw while silder is being manipulated
}

void CAPClientWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	//std::cout << "CAPClientWindow::OnAnimationSpeedControlEvent" << std::endl;
	int value = slider_AnimationSpeed->GetValue();
	int min = slider_AnimationSpeed->GetMin();
	int max = slider_AnimationSpeed->GetMax();
	
	double speed = (double)(value - min) / (double)(max - min) * 2.0;
	
	Cmiss_time_keeper_set_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_SPEED, speed);
}

void CAPClientWindow::UpdateFrameNumber(int frameNumber)
{
	std::ostringstream frameNumberStringStream;
	frameNumberStringStream << "Frame Number: " << frameNumber;
	SetStatusText(wxString(frameNumberStringStream.str().c_str(),wxConvUTF8), 0);
}

void CAPClientWindow::SetTime(double time)
{
	int min = slider_Animation->GetMin();
	int max = slider_Animation->GetMax();
	//cout << "min = " << min << " ,max = " << max <<endl; 
	slider_Animation->SetValue(static_cast<int>(static_cast<double>(max-min)*time) + min + 0.5);
	wxCommandEvent event;
	OnAnimationSliderEvent(event); // Setting the slider value doesn't trigger a slider event so we do it here for it.
}

void CAPClientWindow::OnToggleHideShowAll(wxCommandEvent& event)
{
	bool visibility;
	if (button_HideShowAll->GetLabel() == wxT("Hide All"))
	{
		visibility = false;
		button_HideShowAll->SetLabel(wxT("Show All"));
	}
	else
	{
		visibility = true;
		button_HideShowAll->SetLabel(wxT("Hide All"));
	}

	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (unsigned int i = 0; cit != textureSliceMap_.end(); cit++, i++)
	{
		checkListBox_Slice->Check(i, visibility);
		SetVisibilityForRegion(cmissContext_, cit->first, visibility);
	}
}

void CAPClientWindow::OnToggleHideShowOthers(wxCommandEvent& event)
{
	bool visibility; // Of the non-selected items, if any.
	
	if (button_HideShowOthers->GetLabel() == wxT("Hide Others"))
	{
		visibility = false;
		button_HideShowOthers->SetLabel(wxT("Show Others"));
	}
	else
	{
		visibility = true;
		button_HideShowOthers->SetLabel(wxT("Hide Others"));
	}

	int currentSelection = checkListBox_Slice->GetSelection();
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (unsigned int i = 0; cit != textureSliceMap_.end(); cit++, i++)
	{
		if (currentSelection != i)
		{
			checkListBox_Slice->Check(i, visibility);
			SetVisibilityForRegion(cmissContext_, cit->first, visibility);
		}
	}
}

void CAPClientWindow::OnMIICheckBox(wxCommandEvent& event)
{
	dbg(__func__);
	mainApp_->OnMIICheckBox(event.IsChecked());
}

void CAPClientWindow::OnWireframeCheckBox(wxCommandEvent& event)
{
	dbg(__func__);
	//mainApp_->OnWireframeCheckBox(event.IsChecked());
}

void CAPClientWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
	int value = slider_Brightness->GetValue();
	int min = slider_Brightness->GetMin();
	int max = slider_Brightness->GetMax();
	
	float brightness = (float)(value - min) / (float)(max - min);
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (;cit != textureSliceMap_.end(); cit++)
		cit->second->SetBrightness(brightness);
}

void CAPClientWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
	int value = slider_Contrast->GetValue();
	int min = slider_Contrast->GetMin();
	int max = slider_Contrast->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	TextureSliceMap::const_iterator cit = textureSliceMap_.begin();
	for (;cit != textureSliceMap_.end(); cit++)
		cit->second->SetContrast(contrast);
}

void CAPClientWindow::UpdateModeSelectionUI(size_t newMode)
{
	ResetModeChoice();
	for (size_t i = 1; i <= newMode; i++)
	{
		choice_Mode->Append(wxString(ModeStrings[i],wxConvUTF8));
	}
	choice_Mode->SetSelection(newMode);	
}

void CAPClientWindow::OnAcceptButtonPressed(wxCommandEvent& event)
{
	std::cout << "Accept" << std::endl;
	// mainApp_->ProcessDataPointsEnteredForCurrentMode();
}

void CAPClientWindow::OnModellingModeChanged(wxCommandEvent& event)
{
	std::cout << "MODE = " << choice_Mode->GetStringSelection() << endl;

	int selectionIndex = choice_Mode->GetSelection();
	// mainApp_->ChangeModellingMode(selectionIndex);
}

void CAPClientWindow::OnAbout(wxCommandEvent& event)
{
	std::cout << "CAPClientWindow::" << __func__ << std::endl;
	wxBoxSizer *topsizer;
	wxHtmlWindow *html;
	wxDialog dlg(this, wxID_ANY, wxString(_("About CAP Client")));
	
	topsizer = new wxBoxSizer(wxVERTICAL);
	
	html = new CAPHtmlWindow(&dlg, wxID_ANY, wxDefaultPosition, wxSize(600, 400));
	html -> SetBorders(0);
	html -> LoadPage(wxT("Data/HTML/AboutCAPClient.html"));
	//html -> SetSize(html -> GetInternalRepresentation() -> GetWidth(),
	//				html -> GetInternalRepresentation() -> GetHeight());
	
	topsizer -> Add(html, 1, wxALL, 10);
	
	wxButton *bu1 = new wxButton(&dlg, wxID_OK, _("OK"));
	bu1 -> SetDefault();
	
	topsizer -> Add(bu1, 0, wxALL | wxALIGN_RIGHT, 15);
	
	dlg.SetSizer(topsizer);
	topsizer -> Fit(&dlg);
	
	dlg.Center();
	dlg.ShowModal();
}

void CAPClientWindow::OnOpenImages(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	wxString defaultPath = wxGetCwd();;
	
	const wxString& dirname = wxDirSelector(wxT("Choose the folder that contains the images"), defaultPath);
	if ( !dirname.empty() )
	{
		cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
		mainApp_->OpenImages(std::string(dirname.mb_str()));
	}
}

void CAPClientWindow::ResetModeChoice()
{
	// Resets the mode choice UI widget to Apex mode
	int numberOfItems = choice_Mode->GetCount();
	for (int i = numberOfItems-1; i > 0; i--)
	{
		// Remove all items except Apex
		choice_Mode->Delete(i);
	}
	choice_Mode->SetSelection(0);
}

void CAPClientWindow::OnOpenModel(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("xml");
	wxString wildcard = wxT("");
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector(wxT("Choose a model file to open"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
		// work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		mainApp_->OpenModel(std::string(filename.mb_str()));
	}
}

void CAPClientWindow::OnOpenAnnotation(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("xml");
	wxString wildcard = wxT("");
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector(wxT("Choose an annotation file to open"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		const wxString& dirname = wxDirSelector(wxT("Choose the folder that contains the images"), defaultPath);
		if ( !dirname.empty() )
		{
			std::cout << __func__ << " - Dir name: " << dirname.c_str() << '\n';
		}
		else
		{
			// User cancelled the operation
			cout << "Invalid directory\n";
			return;
		}

		// mainApp_->OpenAnnotation(std::string(filename.mb_str()), std::string(dirname.mb_str()));
	}
}

void CAPClientWindow::OnSave(wxCommandEvent& event)
{
	if (previousSaveLocation_.length() == 0)
		previousSaveLocation_ = wxGetCwd();
	
	
	if (!initialised_xmlUserCommentDialog_)
	{
		initialised_xmlUserCommentDialog_ = true;
		wxXmlInit_UserCommentDialogUI();
	}
	UserCommentDialog userCommentDlg(this);
	userCommentDlg.SetDirectory(previousSaveLocation_);
	userCommentDlg.Center();
	if (userCommentDlg.ShowModal() != wxID_OK)
	{
		return; // Cancelled save
	}
	previousSaveLocation_ = userCommentDlg.GetDirectory();
	std::string userComment = userCommentDlg.GetComment();
	dbg("User comment = " + userComment);
	dbg("Directory = " + previousSaveLocation_);
	
	mainApp_->SaveModel(previousSaveLocation_, userComment);
}

std::string CAPClientWindow::PromptForUserComment()
{
	UserCommentDialog dialog(this);
	dialog.Center();
	std::string comment;
	
	while (true)
	{
		if (dialog.ShowModal() == wxID_OK)
		{
			comment = dialog.GetComment();
			if (!comment.empty())
			{
				break;
			}
			
			int answer = wxMessageBox(wxT("Please enter user comment"), wxT("Empty Comment"),
											wxOK, this);
		}
		else
		{
			int answer = wxMessageBox(wxT("Cancel Save?"), wxT("Confirm"),
											wxYES_NO, this);
			if (answer == wxYES)
			{
				break;
			}
		}
	}
	
	return comment;
}

void CAPClientWindow::OnQuit(wxCommandEvent& event)
{
	std::cout << "CAPClientWindow::" << __func__ << std::endl;
	wxExit();
}

void CAPClientWindow::OnPlaneShiftButtonPressed(wxCommandEvent& event)
{
	static bool isPlaneShiftModeOn = false;
	
	if (!isPlaneShiftModeOn)
	{
		isPlaneShiftModeOn = true;
		button_PlaneShift->SetLabel(wxT("End Shifting"));

		// mainApp_->StartPlaneShift();
	}
	else
	{
		isPlaneShiftModeOn = false;
		button_PlaneShift->SetLabel(wxT("Start Shifting"));
		
		// mainApp_->FinishPlaneShift();
	}
	
	return;
}

void CAPClientWindow::OnExportModel(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("");
	wxString wildcard = wxT("");
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector(wxT("Export to binary files"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if (dirname.empty())
	{
		return;
	}

	if (!wxMkdir(dirname.c_str()))
	{
		std::cout << __func__ << " - Error: can't create directory: " << dirname << std::endl;
		return;
	}
	
	//mainApp_->OnExportModel(std::string(dirname.mb_str()));
	return;
}

void CAPClientWindow::OnExportModelToBinaryVolume(wxCommandEvent& event)
{
	//cout << "CAPClientWindow::" << __func__ << endl;
	//cout << __func__ << "\n";
	
	CAPBinaryVolumeParameterDialog  dlg(this);
	if ( dlg.ShowModal() != wxID_OK )
	{
		return;
	}
	
	double apexMargin, baseMargin, spacing;
	dlg.GetParams(apexMargin, baseMargin, spacing);
	
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("");
	wxString wildcard = wxT("");
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector(wxT("Export to binary volume"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if (dirname.empty())
	{
		return;
	}

	if (!wxMkdir(dirname.c_str()))
	{
		std::cout << __func__ << " - Error: can't create directory: " << dirname << std::endl;
		return;
	}
	
	//mainApp_->OnExportModelToBinaryVolume(std::string(dirname.mb_str()), apexMargin, baseMargin, spacing);
	return;
}

} // end namespace cap
