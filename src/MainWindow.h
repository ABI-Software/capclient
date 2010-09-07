#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

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
#include "ImageBrowseWindowClient.h"

#include <boost/scoped_ptr.hpp>

struct Cmiss_node;
struct Cmiss_time_keeper;

namespace cap
{

class ImageSet;
class CmguiManager;
class CAPXMLFile;

class MainWindow : public wxFrame, public ImageBrowseWindowClient
{
public:
	explicit MainWindow(CmguiManager const& cmguiManager);
	~MainWindow();

//	wxPanel* getPanel() const;
	
	Cmiss_scene_viewer_id GetCmissSceneViewer() const
	{
		return sceneViewer_;
	}
	
	void SetTime(double time);
	
	void RefreshCmguiCanvas();
	
	//void AddDataPoint(DataPoint* dataPoint);
	
	void StopCine();
	
	void AddDataPoint(Cmiss_node*, const cap::Point3D&);
	
	void MoveDataPoint(Cmiss_node*, const cap::Point3D&);
	
	void RemoveDataPoint(Cmiss_node* dataPointID);
	
	void SmoothAlongTime();
	
//	void InitialiseModel();
	
	double GetCurrentTime() const;
	
	void LoadImages(SlicesWithImages const& slices);
	
	void LoadImagesFromXMLFile(SlicesWithImages const& slices);

	virtual void LoadImagesFromImageBrowseWindow(SlicesWithImages const& slices);
	
private:	
	//private utility functions
	void RenderMII(const std::string& sliceName);
	
	void SetImageVisibility(bool visibility, int index);
	
	void SetImageVisibility(bool visibility, const std::string& name = std::string());
	
	void InitialiseVolumeGraph();
	
	void InitialiseMII();
	
	void UpdateMII();
	
	void LoadImages();
	
	void RenderIsoSurfaces();
	
	void LoadHeartModel(std::string const& dirOnly, std::string const& prefix);
	
	void LoadHeartModel(std::string const& path, std::vector<std::string> const& modelFilenames);
	
	void UpdateStatesAfterLoadingModel();
	
	void PopulateObjectList();
	
	template <typename Widget>
	Widget* GetWidgetByName(std::string const& name);

	void EnterInitState();

	void EnterImagesLoadedState();

	void EnterModelLoadedState();
	
	std::string PromptForUserComment();
	
	void PlayCine();

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
	
	CmguiManager const& cmguiManager_;
	Cmiss_context_id context_;
	Cmiss_time_keeper* timeKeeper_;
	ImageSet* imageSet_;
	
	bool animationIsOn_;
	bool hideAll_;
	
	boost::scoped_ptr<CAPModelLVPS4X4> heartModelPtr_;
	
	CAPModeller* modeller_;
	
	Cmiss_scene_viewer_id sceneViewer_;

	enum MainWindowState
	{
		INIT_STATE,
		IMAGES_LOADED_STATE,
		MODEL_LOADED_STATE
	};

	MainWindowState mainWindowState_;
	boost::scoped_ptr<CAPXMLFile> capXMLFilePtr_;
};

} // end namespace cap

#endif

