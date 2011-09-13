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

#include "gui/MainWindowUI.h"

//#include "MainApp.h"

namespace cap
{

class CmguiManager;
class CAPWindow;
class MainApp;

class MainWindow : public MainWindowUI
{
public:
	explicit MainWindow(wxWindow* parent, MainApp* mainApp);
	~MainWindow();

	wxPanel* Get3DPanel() const
	{
		return panel_Cmgui;
	}
	
	void SetTime(double time, int frameNumber);
	
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
	
private:
	std::string PromptForUserComment();

	void UpdateFrameNumber(int frameNumber);
	
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
	
	MainApp* mainApp_; /**< handle to the model class for this window */
};

} // end namespace cap

#endif

