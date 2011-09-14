#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <vector>

#include <wx/wxprec.h>
#include <wx/xrc/xmlres.h>
#include <wx/notebook.h>
#include <wx/frame.h>
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "gui/CAPClientWindowUI.h"

#include "cmguipanel.h"
#include "CAPMath.h"
#include "CmguiExtensions.h"

namespace cap
{

class CAPWindow;
class CAPClient;

class CAPClientWindow : public CAPClientWindowUI
{
public:
	explicit CAPClientWindow(wxWindow* parent, CAPClient* mainApp);
	~CAPClientWindow();

	wxPanel* Get3DPanel() const
	{
		return panel_Cmgui;
	}
	
	void PlayCine();
	
	void StopCine();
	
	void UpdateModeSelectionUI(int mode);
	
	/**
	 * Set the state of the widgets to the initial state.
	 */
	void EnterInitState();

	/**
	 * Set the state of the widgets to the images loaded state.
	 */
	void EnterImagesLoadedState();

	/**
	 * Set the state of the widgets to the model loaded state.
	 */
	void EnterModelLoadedState();
	
	void PopulateSliceList(std::vector<std::string> const& sliceNames,  std::vector<bool> const& visibilities);
	
	bool IsSliceChecked(int i) const
	{
		return checkListBox_Slice->IsChecked(i);
	}
	
	void SetAnimationSliderRange(int min, int max);

	/**
	 * Get the current time from the time keeper.
	 * 
	 * \returns the current time.
	 */
	double GetCurrentTime() const;
	
	/**
	 * Get the scene viewer.
	 * 
	 * \returns the scene viewer.
	 */
	Cmiss_scene_viewer_id GetCmissSceneViewer() const;
	
	Cmiss_context_id GetCmissContext() const;
	
	
	void UpdateFrameNumber(int frameNumber);
	
	void AddDataPoint(Cmiss_node* dataPointID, Point3D const& position);
	void MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition);
	void RemoveDataPoint(Cmiss_node* dataPointID);
	void SmoothAlongTime();
	void SetTime(double time);
	void RedrawNow() const { cmguiPanel_->RedrawNow(); }
	
private:
	std::string PromptForUserComment();

	void ResetModeChoice();

	void UpdateModelVisibilityAccordingToUI();

	void UpdateMIIVisibilityAccordingToUI();
	
	/**
	 * Window widget event handlers.
	 */
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
	void OnPlaneShiftButtonPressed(wxCommandEvent& event);
	
	/**
	 * Menu event handlers.
	 */
	void OnAbout(wxCommandEvent& event);
	void OnOpenImages(wxCommandEvent& event);
	void OnOpenModel(wxCommandEvent& event);
	void OnOpenAnnotation(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnExportModel(wxCommandEvent& event);
	void OnExportModelToBinaryVolume(wxCommandEvent& event);	
	
	/**
	 * Make the connections for the widgets.
	 */
	void MakeConnections();
	
	/**
	 * On idle events are used by Cmgui to update graphics.
	 */
	void OnIdle(wxIdleEvent& event);
	
	CAPClient* mainApp_; /**< handle to the model class for this window */
	CmguiPanel* cmguiPanel_; /**< handle to a cmgui panel class */
	Cmiss_time_keeper_id timeKeeper_; /**< time keeper */
	Cmiss_time_notifier_id timeNotifier_; /**< time notifier */
};

} // end namespace cap

#endif

