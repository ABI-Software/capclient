#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

//#include "DICOMImage.h"
#include "ImageSet.h"
#include "CAPModelLVPS4X4.h"
#include "DataPoint.h"
#include "CAPModeller.h"

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
	
	void PopulateObjectList();
	
	void SetTime(double time);
	
	void RefreshCmguiCanvas();
	
	//void AddDataPoint(DataPoint* dataPoint);
	
	void AddDataPoint(Cmiss_node_id, const DataPoint&);
	
	void MoveDataPoint(Cmiss_node_id, const Point3D&);
	
	void RemoveDataPoint(Cmiss_node_id dataPointID);
	
	void SmoothAlongTime();
	
	void InitialiseModel();
	
	float GetCurrentTime() const;
	
private:
	wxCheckListBox* objectList_;
	wxPanel* m_pPanel;
	
	Cmiss_command_data* command_data;
	Time_keeper* timeKeeper_;
	ImageSet* imageSet_;
	
	bool animationIsOn_;
	bool hideAll_;
	
	CAPModelLVPS4X4 heartModel_;
	
//	std::vector<DataPoint*> dataPoints_;
	CAPModeller modeller_;
	
	//private utility functions
	void RenderMII(const std::string& sliceName);
	
	void SetImageVisibility(bool visibility, int index);
	
	void SetImageVisibility(bool visibility, const std::string& name = std::string());
	
	static int add_scene_object_to_scene_check_box(struct Scene_object *scene_object, void* checklistbox);
	
	void InitialiseVolumeGraph();
	
	void InitialiseMII();
	
	void LoadImages();
	
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
	void OnBrightnessSliderEvent(wxCommandEvent& event);
	void OnContrastSliderEvent(wxCommandEvent& event);
	
	
	void OnAcceptButtonPressed(wxCommandEvent& event);
	void OnModellingModeChanged(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE();
};

#endif
