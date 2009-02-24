#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "DICOMImage.h"

struct Cmiss_command_data;
struct Time_keeper;

extern "C"
{
#include "graphics/scene_viewer.h"
#include "graphics/scene.h"
}

class ViewerFrame : public wxFrame
{
public:
	ViewerFrame(Cmiss_command_data* command_data_);
	ViewerFrame(const wxChar* title, int xpos, int ypos, int width, int height);
	~ViewerFrame();
	
	wxPanel* getPanel();
	
	//test
	void PopulateObjectList();
	
private:
	wxCheckListBox* objectList_;
	wxPanel* m_pPanel;
	
	Cmiss_command_data* command_data;
	Time_keeper* time_keeper;
	ImageSet* imageSet_;
	
	bool animationIsOn;
	
	//private utility functions
	void RefreshCmguiCanvas();
	static int add_scene_object_to_scene_check_box(struct Scene_object *scene_object, void* checklistbox);
	
	//Event handlers
	void Terminate(wxCloseEvent& event);
	void TogglePlay(wxCommandEvent& event);
	void ObjectCheckListChecked(wxCommandEvent& event);
	void ObjectCheckListSelected(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE();
};

#endif
