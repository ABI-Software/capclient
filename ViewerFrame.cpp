// For compilers that don't support precompilation, include "wx/wx.h";

#include "wx/xrc/xmlres.h"
#include "wx/splitter.h"
#include "ViewerFrame.h"
#include "CmguiManager.h"
#include <iostream>
#include <vector>
#include <string>

extern "C"
{
#include "time/time_keeper.h"
#include "time/time.h"
#include "command/cmiss.h"
	
#include "general/debug.h"
}


using namespace std;

ViewerFrame::ViewerFrame(const wxChar *title, int xpos, int ypos, int width, int height)
    //: wxFrame((wxFrame *) NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height))//, m_pPanel(new wxPanel(this))
{
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ViewerFrame"));
	m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	m_pPanel->GetContainingSizer()->SetMinSize(600, 600);
	m_pPanel->GetContainingSizer()->SetDimension(-1, -1, 600, 600);
	this->GetSizer()->SetSizeHints(this);
	this->Fit();
	
#ifdef OLD
//  m_pTextCtrl = new wxTextCtrl(this, -1, wxT("Type some text..."),
//                             wxDefaultPosition,  wxDefaultSize, wxTE_MULTILINE);
  //m_pPanel = new wxPanel(this);// -1, wxDefaultPosition,  wxDefaultSize)
  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer( mainSizer );

  // Add MyPanel to the sizer, set the proportion flag to 1 and using the
  //  wxEXPAND flag to ensure that our control will fill entire content of

  // the frame
  mainSizer->Add(m_pPanel, 1, wxEXPAND);

#ifdef DEL
  m_pMenuBar = new wxMenuBar();
  // File Menu
  m_pFileMenu = new wxMenu();
  m_pFileMenu->Append(wxID_OPEN, _T("&Open"));
  m_pFileMenu->Append(wxID_SAVE, _T("&Save"));
  m_pFileMenu->AppendSeparator();
  m_pFileMenu->Append(wxID_EXIT, _T("&Quit"));
  m_pMenuBar->Append(m_pFileMenu, _T("&File"));
  // About menu
  m_pHelpMenu = new wxMenu();
  m_pHelpMenu->Append(wxID_ABOUT, _T("&About"));
  m_pMenuBar->Append(m_pHelpMenu, _T("&Help"));

  SetMenuBar(m_pMenuBar);
#endif
  CreateStatusBar(3);
  SetStatusText(wxT("Ready"), 0);
  //Layout();
#endif
}

int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
	cout << "Time_call_back time = " << current_time << endl;
	
	ImageSet* imageSet = reinterpret_cast<ImageSet*>(user_data);
	imageSet->SetTime(current_time);
	
//	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();
//	Scene_viewer_redraw(sceneViewer);
}

ViewerFrame::ViewerFrame(Cmiss_command_data* command_data_)
	: command_data(command_data_), animationIsOn(false)
{
	time_keeper = Cmiss_command_data_get_default_time_keeper(command_data);
	
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ViewerFrame"));
	
	//HACK
	this->Show(true);
	wxSplitterWindow* win = XRCCTRL(*this, "window_1", wxSplitterWindow);
	win->SetSashPosition(800, true);
//	this->SetSize(1023,767);
//	this->SetSize(1024,768);
	
	m_pPanel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	m_pPanel->GetContainingSizer()->SetMinSize(1024, 768);
	m_pPanel->GetContainingSizer()->SetDimension(-1, -1, 1024, 768);
	this->GetSizer()->SetSizeHints(this);
	this->Fit();
	
	//see scene_editor_update_widgets_for_scene() for setting up a check list box of
	// scene objects
	wxPanel* sideBar = XRCCTRL(*this, "SideBar", wxPanel);
	wxBoxSizer* sideBarSizer = new wxBoxSizer(wxVERTICAL);
	
	objectList_ = new wxCheckListBox(sideBar, 416501);
	objectList_->SetSelection(wxNOT_FOUND);
	objectList_->Clear();
	
	sideBar->SetSizer(sideBarSizer);
	sideBarSizer->Add(objectList_, 1, wxEXPAND);
	
	this->Layout();
	this->Fit();
	
#define TEXTURE_ANIMATION
#ifdef TEXTURE_ANIMATION
	vector<string> sliceNames;
	sliceNames.push_back("SA1");
	sliceNames.push_back("SA2");
	sliceNames.push_back("SA3");
	sliceNames.push_back("SA4");
	sliceNames.push_back("SA5");
	sliceNames.push_back("SA6");
	sliceNames.push_back("LA1");
	sliceNames.push_back("LA2");
	sliceNames.push_back("LA3");
	
	imageSet_ = new ImageSet(sliceNames); //REFACTOR
			
#define TIME_OBJECT_CALLBACK_TEST
#ifdef TIME_OBJECT_CALLBACK_TEST
	Time_object* time_object = create_Time_object("Texture_animation_timer");
	
	Time_object_add_callback(time_object,time_callback,(void*)imageSet_);
	struct Time_keeper* time_keeper = Cmiss_command_data_get_default_time_keeper(command_data);
	Time_object_set_time_keeper(time_object, time_keeper);
//		Time_object_set_update_frequency(time_object,28);//BUG?? doesnt actually update 28 times -> only 27 
	
	Time_keeper_set_minimum(time_keeper, 0);
	Time_keeper_set_maximum(time_keeper, 1);
	
#endif		
#endif //TEXTURE_ANIMATION
}

ViewerFrame::~ViewerFrame()
{
	delete imageSet_;
}

wxPanel* ViewerFrame::getPanel()
{
	return m_pPanel;
}

void ViewerFrame::TogglePlay(wxCommandEvent& event)
{
	if (animationIsOn)
	{
		Time_keeper_stop(time_keeper);
		this->animationIsOn = false;
	}
	else
	{
		Time_keeper_play(time_keeper,TIME_KEEPER_PLAY_FORWARD);
		Time_keeper_set_play_loop(time_keeper);
		Time_keeper_set_play_every_frame(time_keeper);
		this->animationIsOn = true;
	}
	
	return;
}

void ViewerFrame::Terminate(wxCloseEvent& event)
{
	exit(0); //without this, the funny temporary window appears
}

 int ViewerFrame::add_scene_object_to_scene_check_box(struct Scene_object *scene_object, void* checklistbox)
/*******************************************************************************
LAST MODIFIED : 2 Match 2007

DESCRIPTION :
Add scene_object as checklistbox item into the box.
==============================================================================*/
{
	wxCheckListBox *checklist = static_cast<wxCheckListBox*>(checklistbox);
	char *name;
	int visible;

	ENTER(add_scene_object_to_scene_check_box);
	GET_NAME(Scene_object)(scene_object, &name);
	checklist->Append(name);
	visible =(g_VISIBLE == Scene_object_get_visibility(scene_object));
	/* default selection */
	if ( visible ==1)
	{
		 checklist->Check((checklist->GetCount()-1),1);
	}
	if (checklist->GetCount() == 1)
	{
		 checklist->SetSelection(0);
	}
	
	DEALLOCATE(name);
	LEAVE;
	return(1);
}

//test
void ViewerFrame::PopulateObjectList()
{
	//TODO move Cmgui specific code to ImageSet
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	for_each_Scene_object_in_Scene(scene,
		 add_scene_object_to_scene_check_box, (void *)objectList_);
}

void ViewerFrame::ObjectCheckListChecked(wxCommandEvent& event)
{
	int selection = event.GetInt();
//	objectList_->SetSelection(selection);
	wxString name = objectList_->GetString(selection);
	std::cout << "Check: " << name << std::endl;
	
//	//hack to test the callback works when time is manually set to 0
//	if ("heart"==name)
//	{
//		Time_keeper_request_new_time(time_keeper, 0);
//	}
	
	if(objectList_->IsChecked(selection))
	{
		imageSet_->SetVisible(name.mb_str(), true);
	}
	else
	{
		imageSet_->SetVisible(name.mb_str(), false);
	}
	
	RefreshCmguiCanvas(); //necessrary??
}

void ViewerFrame::ObjectCheckListSelected(wxCommandEvent& event)
{
	//int selection = event.GetInt();
	wxString name = objectList_->GetStringSelection();
	const ImagePlane& plane = imageSet_->GetImagePlane(name.mb_str());
	
	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();
	
//	int Scene_viewer_set_lookat_parameters_non_skew(
//		struct Scene_viewer *scene_viewer,double eyex,double eyey,double eyez,
//		double lookatx,double lookaty,double lookatz,
//		double upx,double upy,double upz)
	
	Point3D planeCenter =  0.5 * (plane.trc + plane.blc);
	Point3D eye = planeCenter + (500 * plane.normal);
	Point3D up(plane.yside);
	NORMALISE(up);
		
	Cmiss_scene_viewer_set_lookat_parameters_non_skew(
			sceneViewer, eye.x, eye.y, eye.z,
			planeCenter.x, planeCenter.y, planeCenter.z,
			up.x, up.y, up.z
			);
	
//	Cmiss_scene_viewer_view_all(sceneViewer);
}

void ViewerFrame::RefreshCmguiCanvas()
{
	Cmiss_scene_viewer_id sceneViewer = CmguiManager::getInstance().getSceneViewer();
//	Scene_viewer_redraw(sceneViewer);
	Scene_viewer_redraw_now(sceneViewer);
}

BEGIN_EVENT_TABLE(ViewerFrame, wxFrame)
	EVT_BUTTON(XRCID("button_1"),ViewerFrame::TogglePlay)
	EVT_CHECKLISTBOX(416501, ViewerFrame::ObjectCheckListChecked)
	EVT_LISTBOX(416501, ViewerFrame::ObjectCheckListSelected)
	EVT_CLOSE(ViewerFrame::Terminate)
END_EVENT_TABLE()
