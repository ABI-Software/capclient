
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

#include "MainWindow.h"
#include "capwindow.h"
#include "MainApp.h"
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

MainWindow::MainWindow(wxWindow* parent, MainApp* mainApp)
	: MainWindowUI(parent)
	, mainApp_(mainApp)
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

MainWindow::~MainWindow()
{
	cout << __func__ << endl;
}

void MainWindow::MakeConnections()
{
	cout << "MainWindow::" << __func__ << endl;
	// Menus
	Connect(XRCID("menuItem_About"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnAbout));
	Connect(XRCID("menuItem_Quit"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnQuit));
	Connect(XRCID("menuItem_OpenImages"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnOpenImages));
	Connect(XRCID("menuItem_OpenModel"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnOpenModel));
	Connect(XRCID("menuItem_OpenAnnotation"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnOpenAnnotation));
	Connect(XRCID("menuItem_Save"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnSave));
	Connect(XRCID("menuItem_Export"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnExportModel));
	Connect(XRCID("menuItem_ExportToBinaryVolume"), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainWindow::OnExportModelToBinaryVolume));
	
	// Buttons
	Connect(button_Play->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(MainWindow::OnTogglePlay));
	
	Connect(wxEVT_IDLE, wxIdleEventHandler(MainWindow::OnIdle), 0, this);
	//Connect(wxEVT_QUIT, 
}
// BEGIN_EVENT_TABLE(MainWindow, MainWindowUI)
// 	EVT_BUTTON(XRCID("button_PlayButton"),MainWindow::OnTogglePlay) // play button
// 	EVT_SLIDER(XRCID("slider_Animation"),MainWindow::OnAnimationSliderEvent) // animation slider
// 	EVT_SLIDER(XRCID("slider_AnimationSpeed"),MainWindow::OnAnimationSpeedControlEvent)
// 	EVT_CHECKLISTBOX(XRCID("checkListBox_Slice"), MainWindow::OnObjectCheckListChecked)
// 	EVT_BUTTON(XRCID("button_HideShowAll"),MainWindow::OnToggleHideShowAll) // hide all button
// 	EVT_BUTTON(XRCID("button_HideShowOthers"),MainWindow::OnToggleHideShowOthers) // hide others button
// 	EVT_CHECKBOX(XRCID("checkBox_MII"),MainWindow::OnMIICheckBox)
// 	EVT_CHECKBOX(XRCID("checkBox_Wireframe"),MainWindow::OnWireframeCheckBox)
// 	EVT_LISTBOX(XRCID("checkListBox_Slice"), MainWindow::OnObjectCheckListSelected)
// 	EVT_SLIDER(XRCID("slider_BrightnessSlider"),MainWindow::OnBrightnessSliderEvent)
// 	EVT_SLIDER(XRCID("slider_ContrastSlider"),MainWindow::OnContrastSliderEvent)
// 	EVT_BUTTON(XRCID("button_AcceptButton"),MainWindow::OnAcceptButtonPressed)
// 	EVT_CHOICE(XRCID("choice_Mode"),MainWindow::OnModellingModeChanged)
// 	EVT_CLOSE(MainWindow::Terminate)
// 	EVT_MENU(XRCID("menuItem_OpenImagesMenuItem"), MainWindow::OnOpenImages)
// 	EVT_MENU(XRCID("menuItem_OpenModelMenuItem"), MainWindow::OnOpenModel)
// 	EVT_MENU(XRCID("menuItem_OpenAnnotationMenuItem"), MainWindow::OnOpenAnnotation)
// 	EVT_MENU(XRCID("menuItem_SaveMenuItem"), MainWindow::OnSave)
// 	EVT_MENU(XRCID("menuItem_ExportMenuItem"), MainWindow::OnExportModel)
// 	EVT_MENU(XRCID("menuItem_ExportToBinaryVolumeMenuItem"), MainWindow::OnExportModelToBinaryVolume)
// 	EVT_BUTTON(XRCID("button_PlaneShiftButton"), MainWindow::OnPlaneShiftButtonPressed)
// END_EVENT_TABLE()

void MainWindow::EnterInitState()
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

void MainWindow::EnterImagesLoadedState()
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

void MainWindow::EnterModelLoadedState()
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

void MainWindow::OnIdle(wxIdleEvent& event)
{
	if (Cmiss_context_process_idle_event(mainApp_->GetCmguiManager()->GetCmissContext()))
	{
		event.RequestMore();
	}
}


void MainWindow::PlayCine()
{
	button_Play->SetLabel(wxT("stop"));
}

void MainWindow::StopCine()
{
	button_Play->SetLabel(wxT("play"));
	wxCommandEvent event;
	OnAnimationSliderEvent(event); //HACK snap the slider to nearest frame time
}

void MainWindow::OnTogglePlay(wxCommandEvent& event)
{	
	// mainApp_->OnTogglePlay();
	return;
}

void MainWindow::Terminate(wxCloseEvent& event)
{
	cout << "MainWindow::" << __func__ << endl;
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

void MainWindow::PopulateSliceList(std::vector<std::string> const& sliceNames, std::vector<bool> const& visibilities)
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

void MainWindow::OnObjectCheckListChecked(wxCommandEvent& event)
{
	int selection = event.GetInt();
	wxString name = checkListBox_Slice->GetString(selection);
	std::cout << "Check: " << name << std::endl;
	
	bool visibility = checkListBox_Slice->IsChecked(selection);
	// mainApp_->SetImageVisibility(visibility, std::string(name.mb_str()));
	
	panel_Cmgui->Refresh();
	this->Refresh();//test to see if this helps with the problem where 3d canvas doesnt update
}

void MainWindow::OnObjectCheckListSelected(wxCommandEvent& event)
{
	wxString name = checkListBox_Slice->GetStringSelection();
	// mainApp_->OnSliceSelected(std::string(name.mb_str()));
	return;
}

void MainWindow::SetAnimationSliderRange(int min, int max)
{
	slider_Animation->SetMin(min);
	slider_Animation->SetMax(max);
}

void MainWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	int value = slider_Animation->GetValue();
	int min = slider_Animation->GetMin();
	int max = slider_Animation->GetMax();
	
	std::cout << __func__ << " : time = " << value << ", min = " << min << ", max = " << max << '\n';
	double time =  (double)(value - min) / (double)(max - min);
	
	// mainApp_->OnAnimationSliderEvent(time);
	
	return;
}

void MainWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	int value = slider_AnimationSpeed->GetValue();
	int min = slider_AnimationSpeed->GetMin();
	int max = slider_AnimationSpeed->GetMax();
	
	double speed = (double)(value - min) / (double)(max - min) * 2.0;
	
	// mainApp_->OnAnimationSpeedControlEvent(speed);
	return;
}

void MainWindow::UpdateFrameNumber(int frameNumber)
{
	std::ostringstream frameNumberStringStream;
	frameNumberStringStream << "Frame Number: " << frameNumber;
	SetStatusText(wxString(frameNumberStringStream.str().c_str(),wxConvUTF8), 0);
}

void MainWindow::SetTime(double time, int frameNumber)
{
	int min = slider_AnimationSpeed->GetMin();
	int max = slider_AnimationSpeed->GetMax();
	//cout << "min = " << min << " ,max = " << max <<endl; 
	slider_AnimationSpeed->SetValue(static_cast<int>(static_cast<double>(max-min)*time) + min);

	UpdateFrameNumber(frameNumber);
	return;
}

void MainWindow::OnToggleHideShowAll(wxCommandEvent& event)
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

void MainWindow::OnToggleHideShowOthers(wxCommandEvent& event)
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

void MainWindow::OnMIICheckBox(wxCommandEvent& event)
{
	// mainApp_->OnMIICheckBox(event.IsChecked());
}

void MainWindow::OnWireframeCheckBox(wxCommandEvent& event)
{
	// mainApp_->OnWireframeCheckBox(event.IsChecked());
}

void MainWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
//	cout << "MainWindow::OnBrightnessSliderEvent" << endl;
	int value = slider_Brightness->GetValue();
	int min = slider_Brightness->GetMin();
	int max = slider_Brightness->GetMax();
	
	float brightness = (float)(value - min) / (float)(max - min);
	// mainApp_->SetImageBrightness(brightness);
}

void MainWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
//	cout << "MainWindow::OnContrastSliderEvent" << endl;
	int value = slider_Contrast->GetValue();
	int min = slider_Contrast->GetMin();
	int max = slider_Contrast->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	// mainApp_->SetImageContrast(contrast);
}

void MainWindow::UpdateModeSelectionUI(int newMode)
{
	ResetModeChoice();
	for (size_t i = 1; i <= newMode; i++)
	{
		choice_Mode->Append(wxString(ModeStrings[i],wxConvUTF8));
	}
	choice_Mode->SetSelection(newMode);	
}

void MainWindow::OnAcceptButtonPressed(wxCommandEvent& event)
{
	std::cout << "Accept" << std::endl;
	// mainApp_->ProcessDataPointsEnteredForCurrentMode();
}

void MainWindow::OnModellingModeChanged(wxCommandEvent& event)
{
	std::cout << "MODE = " << choice_Mode->GetStringSelection() << endl;

	int selectionIndex = choice_Mode->GetSelection();
	// mainApp_->ChangeModellingMode(selectionIndex);
}

void MainWindow::OnAbout(wxCommandEvent& event)
{
	std::cout << "MainWindow::" << __func__ << std::endl;
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

void MainWindow::OnOpenImages(wxCommandEvent& event)
{
	cout << "MainWindow::" << __func__ << endl;
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

void MainWindow::ResetModeChoice()
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

void MainWindow::OnOpenModel(wxCommandEvent& event)
{
	cout << "MainWindow::" << __func__ << endl;
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

void MainWindow::OnOpenAnnotation(wxCommandEvent& event)
{
	cout << "MainWindow::" << __func__ << endl;
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

void MainWindow::OnSave(wxCommandEvent& event)
{
	cout << "MainWindow::" << __func__ << endl;
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

std::string MainWindow::PromptForUserComment()
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

void MainWindow::OnQuit(wxCommandEvent& event)
{
	std::cout << "MainWindow::" << __func__ << std::endl;
	wxExit();
}

void MainWindow::OnPlaneShiftButtonPressed(wxCommandEvent& event)
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

void MainWindow::OnExportModel(wxCommandEvent& event)
{
	cout << "MainWindow::" << __func__ << endl;
	
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

void MainWindow::OnExportModelToBinaryVolume(wxCommandEvent& event)
{
	cout << "MainWindow::" << __func__ << endl;
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
