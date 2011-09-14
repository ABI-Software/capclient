
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
	#include <api/cmiss_context.h>
}

#include "capclient.h"
#include "capclientapp.h"
#include "capclientwindow.h"
#include "UserCommentDialog.h"
#include "CAPHtmlWindow.h"
#include "CAPBinaryVolumeParameterDialog.h"

#include "images/capicon.xpm"

using namespace std;

namespace cap 
{

namespace
{
	const char* ModeStrings[] = {
		"Apex",
		"Base",
		"RV Inserts",
		"Baseplane Points",
		"Guide Points"
	};//REVISE
	}

CAPClientWindow::CAPClientWindow(wxWindow* parent, CAPClient* mainApp)
	: CAPClientWindowUI(parent)
	, mainApp_(mainApp)
	, cmguiPanel_(new CmguiPanel("CAPClient", panel_Cmgui))
{
	SetIcon(wxIcon(capicon_xpm));
	
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
	cout << __func__ << endl;
	delete cmguiPanel_;
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
	
	// Buttons
	Connect(button_Play->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(CAPClientWindow::OnTogglePlay));
	
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
	button_Play->Enable(false);
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
	if (Cmiss_context_process_idle_event(cmguiPanel_->GetCmissContext()))
	{
		event.RequestMore();
	}
}

double CAPClientWindow::GetCurrentTime() const
{
	return Cmiss_time_keeper_get_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_TIME);
}

Cmiss_scene_viewer_id CAPClientWindow::GetCmissSceneViewer() const
{
	return cmguiPanel_->GetCmissSceneViewer();
}

Cmiss_context_id CAPClientWindow::GetCmissContext() const
{
	return cmguiPanel_->GetCmissContext();
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
	button_Play->SetLabel(wxT("stop"));
}

void CAPClientWindow::StopCine()
{
	button_Play->SetLabel(wxT("play"));
	wxCommandEvent event;
	OnAnimationSliderEvent(event); //HACK snap the slider to nearest frame time
}

void CAPClientWindow::OnTogglePlay(wxCommandEvent& event)
{	
	// mainApp_->OnTogglePlay();
	return;
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

void CAPClientWindow::OnObjectCheckListChecked(wxCommandEvent& event)
{
	int selection = event.GetInt();
	wxString name = checkListBox_Slice->GetString(selection);
	std::cout << "Check: " << name << std::endl;
	
	bool visibility = checkListBox_Slice->IsChecked(selection);
	// mainApp_->SetImageVisibility(visibility, std::string(name.mb_str()));
	
	panel_Cmgui->Refresh();
	this->Refresh();//test to see if this helps with the problem where 3d canvas doesnt update
}

void CAPClientWindow::OnObjectCheckListSelected(wxCommandEvent& event)
{
	wxString name = checkListBox_Slice->GetStringSelection();
	// mainApp_->OnSliceSelected(std::string(name.mb_str()));
	return;
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
	
	std::cout << __func__ << " : time = " << value << ", min = " << min << ", max = " << max << '\n';
	double time =  (double)(value - min) / (double)(max - min);
	
	// mainApp_->OnAnimationSliderEvent(time);
	
	return;
}

void CAPClientWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	int value = slider_AnimationSpeed->GetValue();
	int min = slider_AnimationSpeed->GetMin();
	int max = slider_AnimationSpeed->GetMax();
	
	double speed = (double)(value - min) / (double)(max - min) * 2.0;
	
	// mainApp_->OnAnimationSpeedControlEvent(speed);
	return;
}

void CAPClientWindow::UpdateFrameNumber(int frameNumber)
{
	std::ostringstream frameNumberStringStream;
	frameNumberStringStream << "Frame Number: " << frameNumber;
	SetStatusText(wxString(frameNumberStringStream.str().c_str(),wxConvUTF8), 0);
}

void CAPClientWindow::SetTime(double time)
{
	int min = slider_AnimationSpeed->GetMin();
	int max = slider_AnimationSpeed->GetMax();
	//cout << "min = " << min << " ,max = " << max <<endl; 
	slider_AnimationSpeed->SetValue(static_cast<int>(static_cast<double>(max-min)*time) + min);
}

void CAPClientWindow::OnToggleHideShowAll(wxCommandEvent& event)
{
	bool hideAll_ = button_HideShowAll->GetLabel() == wxT("Hide All") ? true : false;
	if (hideAll_) //means the button says hide all rather than show all
	{
		hideAll_ = false;
		// mainApp_->SetImageVisibility(false);
		button_HideShowAll->SetLabel(wxT("Show All"));
	}
	else
	{
		hideAll_ = true;	
		// mainApp_->SetImageVisibility(true);
		button_HideShowAll->SetLabel(wxT("Hide All"));
	}
	
	int numberOfSlices = checkListBox_Slice->GetCount();
	for (int i=0;i<numberOfSlices;i++)
	{
		checkListBox_Slice->Check(i, hideAll_);
	}
	this->Refresh(); // work around for the refresh bug
}

void CAPClientWindow::OnToggleHideShowOthers(wxCommandEvent& event)
{
	static bool showOthers = true;
	
	static std::vector<int> indicesOfOthers;
	if (showOthers) //means the button says hide all rather than show all
	{
		showOthers = false;
		// remember which ones were visible
		indicesOfOthers.clear();
		for (int i=0;i<checkListBox_Slice->GetCount();i++)
		{
			if (checkListBox_Slice->IsChecked(i) && checkListBox_Slice->GetSelection() != i)
			{
				indicesOfOthers.push_back(i);
				// mainApp_->SetImageVisibility(false, i);
				checkListBox_Slice->Check(i, false);
			}
		}
		button_HideShowOthers->SetLabel(wxT("Show Others"));
	}
	else
	{
		showOthers = true;	

		std::vector<int>::iterator itr = indicesOfOthers.begin();
		std::vector<int>::const_iterator end = indicesOfOthers.end();
		for (; itr!=end ; ++itr)
		{
			// mainApp_->SetImageVisibility(true, *itr);
			checkListBox_Slice->Check(*itr, true);
		}
	
		button_HideShowOthers->SetLabel(wxT("Hide Others"));
	}

	this->Refresh(); // work around for the refresh bug
}

void CAPClientWindow::OnMIICheckBox(wxCommandEvent& event)
{
	// mainApp_->OnMIICheckBox(event.IsChecked());
}

void CAPClientWindow::OnWireframeCheckBox(wxCommandEvent& event)
{
	// mainApp_->OnWireframeCheckBox(event.IsChecked());
}

void CAPClientWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
//	cout << "CAPClientWindow::OnBrightnessSliderEvent" << endl;
	int value = slider_Brightness->GetValue();
	int min = slider_Brightness->GetMin();
	int max = slider_Brightness->GetMax();
	
	float brightness = (float)(value - min) / (float)(max - min);
	// mainApp_->SetImageBrightness(brightness);
}

void CAPClientWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
//	cout << "CAPClientWindow::OnContrastSliderEvent" << endl;
	int value = slider_Contrast->GetValue();
	int min = slider_Contrast->GetMin();
	int max = slider_Contrast->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	// mainApp_->SetImageContrast(contrast);
}

void CAPClientWindow::UpdateModeSelectionUI(int newMode)
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
	cout << "CAPClientWindow::" << __func__ << endl;
	wxString defaultPath = wxGetCwd();;
	
	const wxString& dirname = wxDirSelector(wxT("Choose the folder that contains the images"), defaultPath);
	if ( !dirname.empty() )
	{
		cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
		mainApp_->OpenImages(std::string(dirname.mb_str()));
	}
	else
	{
		return;
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
	cout << "CAPClientWindow::" << __func__ << endl;
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
	cout << "CAPClientWindow::" << __func__ << endl;
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
	cout << "CAPClientWindow::" << __func__ << endl;
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("");
	wxString wildcard = wxT("");
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector(wxT("Save file"),
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if (dirname.empty())
	{
		return;
	}
	
	std::string const& userComment = PromptForUserComment();
	std::cout << "User comment = " << userComment << "\n";
	if (userComment.empty())
	{
		// save has been canceled 
		return;
	}

	if (!wxMkdir(dirname.c_str()))
	{
		std::cout << __func__ << " - Error: can't create directory: " << dirname << std::endl;
		return;
	}
	
	// mainApp_->SaveModel(std::string(dirname.mb_str()), userComment);
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
	cout << "CAPClientWindow::" << __func__ << endl;
	
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
	cout << "CAPClientWindow::" << __func__ << endl;
	cout << __func__ << "\n";
	
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
