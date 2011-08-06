#include "wx/xrc/xmlres.h"
#include <wx/dir.h>

//#include "wx/splitter.h"
#include <wx/aboutdlg.h>

#include "MainWindow.h"
#include "UserCommentDialog.h"
#include "CAPHtmlWindow.h"
#include "CAPBinaryVolumeParameterDialog.h"

#include <boost/foreach.hpp>

using namespace std;

namespace cap 
{
		
MainWindow::MainWindow(MainApp<MainWindow, CmguiManager>& mainApp)
:
		mainApp_(mainApp)
{
	// Load layout from .xrc file
	wxXmlResource::Get()->Load("MainWindow.xrc");
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("MainWindow"));
	
	// GUI initialization
	CreateStatusBar(0);
	UpdateFrameNumber(0);

	// Initialize check box list of scene objects (image slices)
	objectList_ = XRCCTRL(*this, "SliceList", wxCheckListBox);
	objectList_->SetSelection(wxNOT_FOUND);
	objectList_->Clear();
	
	m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	
	this->Fit();
	this->Centre();
}

MainWindow::~MainWindow()
{
	cout << __func__ << endl;
}

template <typename Widget>
Widget* MainWindow::GetWidgetByName(std::string const& name)
{
	Widget* widget = XRCCTRL(*this, name.c_str(), Widget);
	return  widget;
}

void MainWindow::EnterInitState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	// Put the gui in the init state
	GetWidgetByName<wxSlider>("AnimationSlider")->Enable(false);
	GetWidgetByName<wxSlider>("AnimationSpeedControl")->Enable(false);
	GetWidgetByName<wxButton>("PlayButton")->Enable(false);
	GetWidgetByName<wxButton>("HideShowAll")->Enable(false);
	GetWidgetByName<wxButton>("HideShowOthers")->Enable(false);
	GetWidgetByName<wxCheckBox>("MII")->Enable(false);
	GetWidgetByName<wxCheckBox>("MII")->SetValue(false);
	GetWidgetByName<wxCheckBox>("Wireframe")->Enable(false);
	GetWidgetByName<wxCheckBox>("Wireframe")->SetValue(false);
	GetWidgetByName<wxSlider>("BrightnessSlider")->Enable(false);
	GetWidgetByName<wxSlider>("ContrastSlider")->Enable(false);
	GetWidgetByName<wxChoice>("ModeChoice")->Enable(false);
	GetWidgetByName<wxButton>("AcceptButton")->Enable(false);
	GetWidgetByName<wxButton>("PlaneShiftButton")->Enable(false);

	GetMenuBar()->FindItem(XRCID("OpenModelMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("SaveMenuItem"))->Enable(false);
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(false);
	GetMenuBar()->FindItem(XRCID("ExportToBinaryVolumeMenuItem"))->Enable(false);

//	objectList_->Clear();

//	this->Fit();
//	this->Centre();
	// HACK to make the cmgui scene viewer fit the enclosing wxPanel
	this->Maximize(true); 
	this->Maximize(false);
}

void MainWindow::EnterImagesLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	GetWidgetByName<wxSlider>("AnimationSlider")->Enable(true);
	GetWidgetByName<wxSlider>("AnimationSlider")->SetValue(0);
	GetWidgetByName<wxSlider>("AnimationSpeedControl")->Enable(true);
	GetWidgetByName<wxButton>("PlayButton")->Enable(true);
	GetWidgetByName<wxButton>("HideShowAll")->Enable(true);
	GetWidgetByName<wxButton>("HideShowOthers")->Enable(true);
	GetWidgetByName<wxCheckBox>("MII")->Enable(false);
	GetWidgetByName<wxCheckBox>("Wireframe")->Enable(false);
	GetWidgetByName<wxSlider>("BrightnessSlider")->Enable(true);
	GetWidgetByName<wxSlider>("ContrastSlider")->Enable(true);
	GetWidgetByName<wxChoice>("ModeChoice")->Enable(true);
	GetWidgetByName<wxButton>("AcceptButton")->Enable(true);
	GetWidgetByName<wxButton>("PlaneShiftButton")->Enable(true);

	GetMenuBar()->FindItem(XRCID("OpenModelMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("SaveMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(false);
	GetMenuBar()->FindItem(XRCID("ExportToBinaryVolumeMenuItem"))->Enable(false);
}

void MainWindow::EnterModelLoadedState()
{
	// TODO also set the state of the ui to the init state
	// i.e uncheck mii & wireframe check boxes, init HideShowAll button etc
	
	GetWidgetByName<wxSlider>("AnimationSlider")->Enable(true);
	GetWidgetByName<wxSlider>("AnimationSpeedControl")->Enable(true);
	GetWidgetByName<wxButton>("PlayButton")->Enable(true);
	GetWidgetByName<wxButton>("HideShowAll")->Enable(true);
	GetWidgetByName<wxButton>("HideShowOthers")->Enable(true);
	GetWidgetByName<wxCheckBox>("MII")->Enable(true);
	GetWidgetByName<wxCheckBox>("Wireframe")->Enable(true);
	GetWidgetByName<wxSlider>("BrightnessSlider")->Enable(true);
	GetWidgetByName<wxSlider>("ContrastSlider")->Enable(true);
	GetWidgetByName<wxChoice>("ModeChoice")->Enable(true);
	GetWidgetByName<wxButton>("AcceptButton")->Enable(true);
	GetWidgetByName<wxButton>("PlaneShiftButton")->Enable(true);

	GetMenuBar()->FindItem(XRCID("OpenModelMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("SaveMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("ExportMenuItem"))->Enable(true);
	GetMenuBar()->FindItem(XRCID("ExportToBinaryVolumeMenuItem"))->Enable(true);

	GetWidgetByName<wxCheckBox>("Wireframe")->SetValue(true);
}

void MainWindow::PlayCine()
{
	wxButton* button = XRCCTRL(*this, "PlayButton", wxButton);
	button->SetLabel("stop");
}

void MainWindow::StopCine()
{
	wxButton* button = XRCCTRL(*this, "PlayButton", wxButton);
	button->SetLabel("play");
	wxCommandEvent event;
	OnAnimationSliderEvent(event); //HACK snap the slider to nearest frame time
}

void MainWindow::OnTogglePlay(wxCommandEvent& event)
{	
	mainApp_.OnTogglePlay();
	return;
}

void MainWindow::Terminate(wxCloseEvent& event)
{
	int answer = wxMessageBox("Quit program?", "Confirm",
	                            wxYES_NO, this);
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
	objectList_->Clear();
	
	size_t index = 0;
	BOOST_FOREACH(std::string const& sliceName, sliceNames)
	{
		std::cout << "Slice name = " << sliceName << '\n';
		objectList_->Append(sliceName.c_str());
		bool visible = visibilities.at(index);
		/* default selection */
		if ( visible )
		{
			objectList_->Check((objectList_->GetCount()-1),1);
		}
		objectList_->SetSelection(wxNOT_FOUND);
		index ++;
	}
}

void MainWindow::OnObjectCheckListChecked(wxCommandEvent& event)
{
	int selection = event.GetInt();
	wxString name = objectList_->GetString(selection);
	std::cout << "Check: " << name << std::endl;
	
	bool visibility = objectList_->IsChecked(selection);
	mainApp_.SetImageVisibility(visibility, name.mb_str());
	
	m_pPanel->Refresh();
	this->Refresh();//test to see if this helps with the problem where 3d canvas doesnt update
}

void MainWindow::OnObjectCheckListSelected(wxCommandEvent& event)
{
	wxString name = objectList_->GetStringSelection();
	mainApp_.OnSliceSelected(name.c_str());
	return;
}

void MainWindow::SetAnimationSliderRange(int min, int max)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	slider->SetMin(min);
	slider->SetMax(max);
}

void MainWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	std::cout << __func__ << " : time = " << value << ", min = " << min << ", max = " << max << '\n';
	double time =  (double)(value - min) / (double)(max - min);
	
	mainApp_.OnAnimationSliderEvent(time);
	
	return;
}

void MainWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSpeedControl", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	double speed = (double)(value - min) / (double)(max - min) * 2.0;
	
	mainApp_.OnAnimationSpeedControlEvent(speed);
	return;
}

void MainWindow::UpdateFrameNumber(int frameNumber)
{
	std::ostringstream frameNumberStringStream;
	frameNumberStringStream << "Frame Number: " << frameNumber;
	SetStatusText(wxT(frameNumberStringStream.str().c_str()), 0);
}

void MainWindow::SetTime(double time, int frameNumber)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	int min = slider->GetMin();
	int max = slider->GetMax();
	//cout << "min = " << min << " ,max = " << max <<endl; 
	slider->SetValue(static_cast<int>(static_cast<double>(max-min)*time) + min);

	UpdateFrameNumber(frameNumber);
	return;
}

void MainWindow::OnToggleHideShowAll(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "HideShowAll", wxButton);
	bool hideAll_ = button->GetLabel() == "Hide All" ? true : false;//FIXME
	if (hideAll_) //means the button says hide all rather than show all
	{
		hideAll_ = false;
		mainApp_.SetImageVisibility(false);
		button->SetLabel("Show All");
	}
	else
	{
		hideAll_ = true;	
		mainApp_.SetImageVisibility(true);
		button->SetLabel("Hide All");
	}
	
	int numberOfSlices = objectList_->GetCount();
	for (int i=0;i<numberOfSlices;i++)
	{
		objectList_->Check(i, hideAll_);
	}
	this->Refresh(); // work around for the refresh bug
}

void MainWindow::OnToggleHideShowOthers(wxCommandEvent& event)
{
	wxButton* button = XRCCTRL(*this, "HideShowOthers", wxButton);
	static bool showOthers = true;
	
	static std::vector<int> indicesOfOthers;
	if (showOthers) //means the button says hide all rather than show all
	{
		showOthers = false;
		// remember which ones were visible
		indicesOfOthers.clear();
		for (int i=0;i<objectList_->GetCount();i++)
		{
			if (objectList_->IsChecked(i) && objectList_->GetSelection() != i)
			{
				indicesOfOthers.push_back(i);
				mainApp_.SetImageVisibility(false, i);
				objectList_->Check(i, false);
			}
		}
		button->SetLabel("Show Others");
	}
	else
	{
		showOthers = true;	

		std::vector<int>::iterator itr = indicesOfOthers.begin();
		std::vector<int>::const_iterator end = indicesOfOthers.end();
		for (; itr!=end ; ++itr)
		{
			mainApp_.SetImageVisibility(true, *itr);
			objectList_->Check(*itr, true);
		}
	
		button->SetLabel("Hide Others");
	}

	this->Refresh(); // work around for the refresh bug
}

void MainWindow::OnMIICheckBox(wxCommandEvent& event)
{
	mainApp_.OnMIICheckBox(event.IsChecked());
}

void MainWindow::OnWireframeCheckBox(wxCommandEvent& event)
{
	mainApp_.OnWireframeCheckBox(event.IsChecked());
}

void MainWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
//	cout << "MainWindow::OnBrightnessSliderEvent" << endl;
	wxSlider* slider = XRCCTRL(*this, "BrightnessSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	float brightness = (float)(value - min) / (float)(max - min);
	mainApp_.SetImageBrightness(brightness);
}

void MainWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
//	cout << "MainWindow::OnContrastSliderEvent" << endl;
	wxSlider* slider = XRCCTRL(*this, "ContrastSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	float contrast = (float)(value - min) / (float)(max - min);
	mainApp_.SetImageContrast(contrast);
}

void MainWindow::UpdateModeSelectionUI(int newMode)
{
	wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);

	ResetModeChoice();
	for (size_t i = 1; i <= newMode; i++)
	{
		choice->Append(ModeStrings[i]);
	}
	choice->SetSelection(newMode);	
}

void MainWindow::OnAcceptButtonPressed(wxCommandEvent& event)
{
	std::cout << "Accept" << std::endl;
	mainApp_.ProcessDataPointsEnteredForCurrentMode();
}

void MainWindow::OnModellingModeChanged(wxCommandEvent& event)
{
	wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
	std::cout << "MODE = " << choice->GetStringSelection() << endl;

	int selectionIndex = choice->GetSelection();
	mainApp_.ChangeModellingMode(selectionIndex);
}

void MainWindow::OnAbout(wxCommandEvent& event)
{
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
	wxString defaultPath = wxGetCwd();;
	
	const wxString& dirname = wxDirSelector("Choose the folder that contains the images", defaultPath);
	if ( !dirname.empty() )
	{
		cout << __func__ << " - Dir name: " << dirname.c_str() << endl;
	}
	else
	{
		return;
	}
	
	mainApp_.OpenImages(dirname.c_str());
}

void MainWindow::ResetModeChoice()
{
	// Resets the mode choice UI widget to Apex mode
	wxChoice* choice = XRCCTRL(*this, "ModeChoice", wxChoice);
	int numberOfItems = choice->GetCount();
	for (int i = numberOfItems-1; i > 0; i--)
	{
		// Remove all items except Apex
		choice->Delete(i);
	}
	choice->SetSelection(0);
}

void MainWindow::OnOpenModel(wxCommandEvent& event)
{
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = "";
	wxString defaultExtension = "xml";
	wxString wildcard = "";
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector("Choose a model file to open",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		mainApp_.OpenModel(filename.c_str());
	}
}

void MainWindow::OnOpenAnnotation(wxCommandEvent& event)
{
	wxString defaultPath = wxGetCwd();
	wxString defaultFilename = "";
	wxString defaultExtension = "xml";
	wxString wildcard = "";
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector("Choose an annotation file to open",
			defaultPath, defaultFilename, defaultExtension, wildcard, flags);
	if ( !filename.empty() )
	{
	    // work with the file
		cout << __func__ << " - File name: " << filename.c_str() << endl;

		const wxString& dirname = wxDirSelector("Choose the folder that contains the images", defaultPath);
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

		mainApp_.OpenAnnotation(filename.c_str(), dirname.c_str());
	}
}

void MainWindow::OnSave(wxCommandEvent& event)
{
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = "";
	wxString defaultExtension = "";
	wxString wildcard = "";
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector("Save file",
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
	
	mainApp_.SaveModel(dirname.c_str(), userComment);
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
			
			int answer = wxMessageBox("Please enter user comment", "Empty Comment",
											wxOK, this);
		}
		else
		{
			int answer = wxMessageBox("Cancel Save?", "Confirm",
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
	Close();
}

void MainWindow::OnPlaneShiftButtonPressed(wxCommandEvent& event)
{
	static bool isPlaneShiftModeOn = false;
	
	wxButton* button = XRCCTRL(*this, "PlaneShiftButton", wxButton);
	assert(button);
	
	if (!isPlaneShiftModeOn)
	{
		isPlaneShiftModeOn = true;
		button->SetLabel("End Shifting");

		mainApp_.StartPlaneShift();
	}
	else
	{
		isPlaneShiftModeOn = false;
		button->SetLabel("Start Shifting");
		
		mainApp_.FinishPlaneShift();
	}
	
	return;
}

void MainWindow::OnExportModel(wxCommandEvent& event)
{
	cout << __func__ << "\n";
	
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = "";
	wxString defaultExtension = "";
	wxString wildcard = "";
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector("Export to binary files",
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
	
	mainApp_.OnExportModel(dirname.c_str());
	return;
}

void MainWindow::OnExportModelToBinaryVolume(wxCommandEvent& event)
{
	cout << __func__ << "\n";
	
	CAPBinaryVolumeParameterDialog  dlg(this);
	if ( dlg.ShowModal() != wxID_OK )
	{
		return;
	}
	
	double apexMargin, baseMargin, spacing;
	dlg.GetParams(apexMargin, baseMargin, spacing);
	
	wxString defaultPath = wxGetCwd();;
	wxString defaultFilename = "";
	wxString defaultExtension = "";
	wxString wildcard = "";
	int flags = wxSAVE;
	
	wxString dirname = wxFileSelector("Export to binary volume",
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
	
	mainApp_.OnExportModelToBinaryVolume(dirname.c_str(), apexMargin, baseMargin, spacing);
	return;
}

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_BUTTON(XRCID("PlayButton"),MainWindow::OnTogglePlay) // play button
	EVT_SLIDER(XRCID("AnimationSlider"),MainWindow::OnAnimationSliderEvent) // animation slider
	EVT_SLIDER(XRCID("AnimationSpeedControl"),MainWindow::OnAnimationSpeedControlEvent)
	EVT_CHECKLISTBOX(XRCID("SliceList"), MainWindow::OnObjectCheckListChecked)
	EVT_BUTTON(XRCID("HideShowAll"),MainWindow::OnToggleHideShowAll) // hide all button
	EVT_BUTTON(XRCID("HideShowOthers"),MainWindow::OnToggleHideShowOthers) // hide others button
	EVT_CHECKBOX(XRCID("MII"),MainWindow::OnMIICheckBox)
	EVT_CHECKBOX(XRCID("Wireframe"),MainWindow::OnWireframeCheckBox)
	EVT_LISTBOX(XRCID("SliceList"), MainWindow::OnObjectCheckListSelected)
	EVT_SLIDER(XRCID("BrightnessSlider"),MainWindow::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),MainWindow::OnContrastSliderEvent)
	EVT_BUTTON(XRCID("AcceptButton"),MainWindow::OnAcceptButtonPressed)
	EVT_CHOICE(XRCID("ModeChoice"),MainWindow::OnModellingModeChanged)
	EVT_CLOSE(MainWindow::Terminate)
	EVT_MENU(XRCID("QuitMenuItem"),  MainWindow::OnQuit)
	EVT_MENU(XRCID("AboutMenuItem"), MainWindow::OnAbout)
	EVT_MENU(XRCID("OpenImagesMenuItem"), MainWindow::OnOpenImages)
	EVT_MENU(XRCID("OpenModelMenuItem"), MainWindow::OnOpenModel)
	EVT_MENU(XRCID("OpenAnnotationMenuItem"), MainWindow::OnOpenAnnotation)
	EVT_MENU(XRCID("SaveMenuItem"), MainWindow::OnSave)
	EVT_MENU(XRCID("ExportMenuItem"), MainWindow::OnExportModel)
	EVT_MENU(XRCID("ExportToBinaryVolumeMenuItem"), MainWindow::OnExportModelToBinaryVolume)
	EVT_BUTTON(XRCID("PlaneShiftButton"), MainWindow::OnPlaneShiftButtonPressed)
END_EVENT_TABLE()

} // end namespace cap
