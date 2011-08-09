#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include "wx/wxprec.h"
#include "wx/xrc/xmlres.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "MainApp.h"

namespace cap
{

class CmguiManager;

class MainWindow : public wxFrame
{
public:
	explicit MainWindow(MainApp<MainWindow, CmguiManager>& mainApp);
	~MainWindow();

	wxPanel* Get3DPanel() const
	{
		return m_pPanel;
	}
	
	void SetTime(double time, int frameNumber);
	
	void PlayCine();
	
	void StopCine();
	
	void UpdateModeSelectionUI(int mode);
	
	void EnterInitState();

	void EnterImagesLoadedState();

	void EnterModelLoadedState();
	
	void PopulateSliceList(std::vector<std::string> const& sliceNames,  std::vector<bool> const& visibilities);
	
	bool IsSliceChecked(int i) const
	{
		return objectList_->IsChecked(i);
	}
	
	void SetAnimationSliderRange(int min, int max);
	
private:
	template <typename Widget>
	Widget* GetWidgetByName(std::string const& _name)
	{
		long int id = wxXmlResource::GetXRCID(wxString(_name.c_str(),wxConvUTF8));
		return wxStaticCast((*this).FindWindow(id), Widget);
	}
	
	std::string PromptForUserComment();

	void UpdateFrameNumber(int frameNumber);
	
	void ResetModeChoice();

	void UpdateModelVisibilityAccordingToUI();

	void UpdateMIIVisibilityAccordingToUI();
	
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
	void OnOpenAnnotation(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnExportModel(wxCommandEvent& event);
	void OnExportModelToBinaryVolume(wxCommandEvent& event);
	
	void OnPlaneShiftButtonPressed(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE();
	
	wxCheckListBox* objectList_;
	wxPanel* m_pPanel;

	MainApp<MainWindow, CmguiManager>& mainApp_;
};

} // end namespace cap

#endif

