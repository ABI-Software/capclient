#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

extern "C" {
#include "api/cmiss_context.h"
}

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "CAPModelLVPS4X4.h"
#include "CAPModeller.h"

struct Cmiss_node;
struct Cmiss_time_keeper;

class ImageSet;

class ViewerFrame : public wxFrame
{
public:
	ViewerFrame(Cmiss_context_id command_data_);
	~ViewerFrame();

	wxPanel* getPanel();
	
	void PopulateObjectList();
	
	void SetTime(double time);
	
	void RefreshCmguiCanvas();
	
	//void AddDataPoint(DataPoint* dataPoint);
	
	void AddDataPoint(Cmiss_node*, const Point3D&);
	
	void MoveDataPoint(Cmiss_node*, const Point3D&);
	
	void RemoveDataPoint(Cmiss_node* dataPointID);
	
	void SmoothAlongTime();
	
	void InitialiseModel();
	
	float GetCurrentTime() const;
	
private:	
	//private utility functions
	void RenderMII(const std::string& sliceName);
	
	void SetImageVisibility(bool visibility, int index);
	
	void SetImageVisibility(bool visibility, const std::string& name = std::string());
	
	static int add_scene_object_to_scene_check_box(struct Scene_object *scene_object, void* checklistbox);
	
	void InitialiseVolumeGraph();
	
	void InitialiseMII();
	
	void UpdateMII();
	
	void LoadImages();
	
	void RenderIsoSurfaces();
	
	//Event handlers
	void Terminate(wxCloseEvent& event);
	void OnTogglePlay(wxCommandEvent& event);
	void OnObjectCheckListChecked(wxCommandEvent& event);
	void OnObjectCheckListSelected(wxCommandEvent& event);
	void OnToggleHideShowAll(wxCommandEvent& event);
	void OnToggleHideShowOthers(wxCommandEvent& event);
	void OnAnimationSliderEvent(wxCommandEvent& event);
	void OnAnimationSpeedControlEvent(wxCommandEvent& event);
	void OnMIICheckBox(wxCommandEvent& event);
	void OnWireframeCheckBox(wxCommandEvent& event);
	void OnBrightnessSliderEvent(wxCommandEvent& event);
	void OnContrastSliderEvent(wxCommandEvent& event);

	void OnAcceptButtonPressed(wxCommandEvent& event);
	void OnModellingModeChanged(wxCommandEvent& event);
	
	void OnAbout(wxCommandEvent& event);
	void OnOpenImages(wxCommandEvent& event);
	void OnOpenModel(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnExportModel(wxCommandEvent& event);
	
	void OnPlaneShiftButtonPressed(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE();
	
	wxCheckListBox* objectList_;
	wxPanel* m_pPanel;
	
	Cmiss_context_id context_;
	Cmiss_time_keeper* timeKeeper_;
	ImageSet* imageSet_;
	
	bool animationIsOn_;
	bool hideAll_;
	
	CAPModelLVPS4X4 heartModel_;
	
//	std::vector<DataPoint*> dataPoints_;
	CAPModeller* modeller_;
	
	Cmiss_scene_viewer_id sceneViewer_;
};

#endif
