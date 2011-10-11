#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <vector>
#include <map>

#include <boost/tr1/memory.hpp>

#include <wx/wxprec.h>
#include <wx/xrc/xmlres.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/frame.h>
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "ui/CAPClientWindowUI.h"

#include "cmguipanel.h"
#include "material.h"
#include "CAPMath.h"
#include "CmguiExtensions.h"
#include "textureslice.h"


namespace cap
{

class CAPClient; // Forward declare this so we can pass a pointer to the class later on.

typedef std::map< std::string, boost::shared_ptr<TextureSlice> > TextureSliceMap;

/**
 * \brief CAPClientWindow is the gui(view) for the CAPClient class.
 * This class has a handle to a cmiss_context from which it can create
 * scenes to view the data.
 * 
 * This class inherits from CAPClientWindowUI which is a generated class
 * generated from CAPClientWindowUI.xrc.  We use public inheritance here
 * so that we can directly access the widgets defined in the CAPClientWindowUI.
 * CAPClientWindowUI.xrc can be edited directly or it can be generated in turn 
 * using an application like wxformbuilder (wxformbuilder.org) 
 * [which, in this case, was used to generate this file].
 */
class CAPClientWindow : public CAPClientWindowUI
{
public:
	/**
	 * Constructor takes a wxWindow pointer to assign a parent and 
	 * a pointer to the CAPClient data class.
	 * 
	 * \param parent parent wxWidgets window.
	 * \param mainApp pointer to CAPClient data class.
	 */
	explicit CAPClientWindow(CAPClient* mainApp);
	
	/**
	 * Destructor destroys handles received from the Cmgui libraries.
	 */
	~CAPClientWindow();
	
	/**
	 * Create a texture slice.  To create a textrue slice we need to create a scene
	 * that has a surface, the label of the labelled slice determines the region name.
	 * We also need to create a material and a set of field images from the dicom images 
	 * contained in the labelled slice.  The surface then uses the material to display 
	 * one of the field images.  The position of the surface in the scene needs to be set 
	 * to the orientation as specified in the dicom images header.
	 * 
	 * \param labelledSlice the labelled slice to create the field images from.
	 */
	void CreateTextureSlice(const LabelledSlice& labelledSlice);
	
	void PlayCine();
	
	void StopCine();
	
	void UpdateModeSelectionUI(size_t mode);
	
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
	/**
	 * Set the field image as the texture for the material identified from
	 * the given name.
	 * 
	 * \param name the name of the material to set the field image to.
	 * \param fieldImage the field image to use as a texture.
	 */
	void ChangeTexture(const std::string& name, Cmiss_field_image_id fieldImage);
	
	/**
	 * Create a vector of field images from the dicom images in the labelled slice in
	 * the region determined from the label of the labelled slice.  This function will
	 * load the dicom images listed in the slice from disk.
	 * 
	 * \param labelledSlice the labelled slice to create the field images from.
	 * \returns a vector of accessed Cmiss_field_image_id.
	 */
	std::vector<Cmiss_field_image_id> CreateFieldImages(const LabelledSlice& labelledSlice);
	
	/**
	 * Create a scene for an image slice in the given region name.  A scene 
	 * consists of a surface and a material.  The material is used by the 
	 * surface to display a field image from an image slice.  This function 
	 * will add the material to the Material map which uses the 
	 * region name for a key.
	 * 
	 * \param regionName the region name to create the scene in.
	 */
	void CreateScene(const std::string& regionName);
	
	std::string PromptForUserComment();

	void ResetModeChoice();

	void UpdateModelVisibilityAccordingToUI();

	void UpdateMIIVisibilityAccordingToUI();
	
	/**
	 * Window widget event handlers.
	 */
	void Terminate(wxCloseEvent& event);
	void OnTogglePlay(wxCommandEvent& event);
	void OnObjectCheckListChecked(wxListEvent& event);
	void OnObjectCheckListSelected(wxListEvent& event);
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
	Cmiss_context_id cmissContext_; /**< handle to the context for this class. */
	CmguiPanel* cmguiPanel_; /**< handle to a cmgui panel class */
	TextureSliceMap textureSliceMap_; /**< A map of texture slices. */ 
	std::vector<Cmiss_field_image_id> fieldImages_; /**< A vector of field images. */
	Cmiss_time_keeper_id timeKeeper_; /**< time keeper */
	Cmiss_time_notifier_id timeNotifier_; /**< time notifier */
};

} // end namespace cap

#endif

