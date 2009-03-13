#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//#include "DICOMImage.h"
#include "ImageSet.h"
#include "CAPModelLVPS4X4.h"

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
	
	void SetTime(double time);
	
	void RefreshCmguiCanvas();
private:
	wxCheckListBox* objectList_;
	wxPanel* m_pPanel;
	
	Cmiss_command_data* command_data;
	Time_keeper* timeKeeper_;
	ImageSet* imageSet_;
	
	bool animationIsOn_;
	bool hideAll_;
	
	CAPModelLVPS4X4 heartModel_;
	
	//private utility functions
	void RenderMII(const std::string& sliceName);
	
	static int add_scene_object_to_scene_check_box(struct Scene_object *scene_object, void* checklistbox);
	
	//Event handlers
	void Terminate(wxCloseEvent& event);
	void TogglePlay(wxCommandEvent& event);
	void ObjectCheckListChecked(wxCommandEvent& event);
	void ObjectCheckListSelected(wxCommandEvent& event);
	void ToggleHideShowAll(wxCommandEvent& event);
	void ToggleHideShowOthers(wxCommandEvent& event);
	void OnAnimationSliderEvent(wxCommandEvent& event);
	void OnAnimationSpeedControlEvent(wxCommandEvent& event);
	void OnMIICheckBox(wxCommandEvent& event);
	void OnWireframeCheckBox(wxCommandEvent& event);
	DECLARE_EVENT_TABLE();
};

#endif
