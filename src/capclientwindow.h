#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>

#include <wx/wxprec.h>
#include <wx/xrc/xmlres.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/frame.h>
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

extern "C"
{
#include <zn/cmiss_context.h>
#include <zn/cmiss_field_image.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_node.h>
}

#include "ui/capclientwindowui.h"

#include "standardheartdefinitions.h"
#include "math/algebra.h"
#include "model/modellingpoint.h"
#include "io/modelfile.h"

class wxProgressDialog;

namespace cap
{

class CAPClient; // Forward declare this so we can pass a pointer to the class later on.
class TextureSlice;
class LabelledSlice;
class SceneViewerPanel;
class HeartModel;
struct Plane;

/**
 * Defines an alias representing the texture slice map.
 */
typedef std::map< std::string, TextureSlice* > TextureSliceMap;

/**
 * Defines an alias representing the cmiss field graphic pair.
 */
typedef std::pair<Cmiss_field_id, Cmiss_graphic_id> CmissFieldGraphicPair;

/**
 * Defines an alias representing the status text strings field map.
 */
typedef std::map<std::string, CmissFieldGraphicPair> StatusTextStringsFieldMap;

/**
 * Defines an alias representing the mii field graphic map.
 */
typedef std::map<std::string, std::pair<Cmiss_graphic_id, Cmiss_graphic_id> > MIIGraphicMap;

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

	/**
	 * Clears the texture slices.  This requires the deletion of the fields and
	 * regions that the texture slices are created in.
	 */
	void RemoveTextureSlices();

	/**
	 * Remove image contours.
	 */
	void RemoveImageContours();

	void AddImageContours(const std::string& label, const std::vector<ModelFile::Contour>& contour, int frame);
	void MoveImageContours(const std::string& regionName, const Vector3D& diff);

	/**
	 * Play cine.
	 */
	void PlayCine();

	/**
	 * Stop cine.
	 */
	void StopCine();

	/**
	 * Updates the mode selection user interface described by mode.
	 *
	 * @param	mode	The mode.
	 */
	void UpdateModeSelectionUI(ModellingEnum mode);

	/**
	 * Executes the accept action.  Callback API for cmgui to activate the
	 * OnAccept function.  Pressing the 'a' key in the Cmgui scene viewer panel
	 * will simulate the pressing of the accept button on the modeller panel.
	 */
	void OnAccept();

	/**
	 * Sets the intial position for calculating plane shifting movement.
	 * This function is part of the PlaneShiftingInterface.
	 *
	 * @param	pos	The pos in the global coordinate frame (in pixels).
	 */
	void SetInitialPosition(const Point3D& pos);

	/**
	 * Updates the position.  This function is part of the PlaneShiftingInterface.
	 *
	 * @param	pos	The pos in the global coordinate frame (in pixels).
	 */
	void UpdatePosition(const Point3D& pos);

	/**
	 * Sets an end position.  This function is part of the PlaneShiftingInterface.
	 *
	 * @param	pos	The pos in the global coordinate frame (in pixels).
	 */
	void SetEndPosition(const Point3D& pos);

	void SetZKeyDown(bool down);

	/**
	 * Reposition image plane.
	 *
	 * @param	regionName	Name of the region.
	 * @param	plane	  	The plane.
	 */
	void RepositionImagePlane(const std::string& regionName, const ImagePlane* plane);

	/**
	 * Ends the current modelling mode.
	 */
	void EndCurrentModellingMode();

	/**
	 * Initializes the mii for the given slice.
	 *
	 * @param	sliceName	The name of the slice.
	 */
	void InitializeMII(const std::string& sliceName);

	/**
	 * Updates the mii.
	 *
	 * @param	sliceName	The name of the slice.
	 * @param	plane	 	The plane.
	 * @param	iso_value	The iso value.
	 */
	void UpdateMII(const std::string& sliceName, const Vector3D& plane, double iso_value);

	/**
	 * Sets a mii visibility.
	 *
	 * @param	visible	true to visibility.
	 */
	void SetMIIVisibility(bool visible);

	/**
	 * Sets a mii visibility.
	 *
	 * @param	name   	The name.
	 * @param	visible	true to visibile.
	 */
	void SetMIIVisibility(const std::string& name, bool visible);

	/**
	 * Sets the heart model visibility.
	 *
	 * @param	visible	true to visibility.
	 */
	void SetModelVisibility(bool visible);

	/**
	 * Populate the slice list with the visibilities given.  The association
	 * of slice to visibility is done purely through position in the vector.
	 *
	 * @param	sliceNames  	List of names of the slices.
	 * @param	visibilities	The visibilities of the associted slices.
	 */
	void PopulateSliceList(std::vector<std::string> const& sliceNames,  std::vector<bool> const& visibilities);

	/**
	 * Sets an animation slider range.
	 *
	 * @param	min	The minimum.
	 * @param	max	The maximum.
	 */
	void SetAnimationSliderRange(int min, int max);

	/**
	 * Get the current time from the time keeper.
	 *
	 * \returns the current time.
	 */
	double GetCurrentTime() const;

	/**
	 * Gets the time keeper.
	 *
	 * @return	The time keeper.
	 */
	Cmiss_time_keeper_id GetTimeKeeper() const { return timeKeeper_; }

	/**
	 * Creates the heart model.  If the heart model exists it will be deleted and a new one created.
	 */
	void CreateHeartModel();

	/**
	 * Removes the heart model.  This function also removes the Model/Image intersection graphics as
	 * well.
	 */
	void RemoveHeartModel();

	/**
	 * Query if this objects heart model is initialised .
	 *
	 * @return	true if heart model is initialised, false if not.
	 */
	bool IsInitialisedHeartModel() const
	{
		return (heartModel_ != 0);
	}

	/**
	 * Sets the heart transform.  This function applies a transformation
	 * from a prolate spheriodal coordinate field named 'coordinates' to
	 * a rectangular cartesian field named 'coordinates_patient_rc' that has
	 * been transformed using the given transformation matrix.
	 *
	 * It is expected that the prolate spheriodal field 'coordinates'
	 * is already defined.
	 *
	 * @param	transform	The transform.
	 */
	void SetHeartModelTransformation(const gtMatrix& transform);

	/**
	 * Calculates the heart model xi.  This function finds the nearest element of the heart model
	 * and returns the xi coordinates of the element at which the given point is nearest to.
	 *
	 * @param	position  	The position.
	 * @param	time	  	The time.
	 * @param [in,out]	xi	The xi.
	 *
	 * @return	The element id of the nearest element, returns -1 if no element was found.
	 */
	int ComputeHeartModelXi(const Point3D& position, double time, Point3D& xi) const;

	/**
	 * Converts a nodes rc position into a heart model prolate spheriodal coordinate.
	 *
	 * @param	node_id	   	The node identifier.
	 * @param	region_name	Name of the region.
	 *
	 * @return	The position in a prolate shperiodal coordinate system.
	 */
	Point3D ConvertToHeartModelProlateSpheriodalCoordinate(int node_id, const std::string& region_name) const;

	/**
	 * Sets the heart prolate spheriod focal length.
	 *
	 * @param	focalLength	focal length for the prolate spheriod coordinate system.
	 */
	void SetHeartModelFocalLength(double focalLength);

	/**
	 * Sets a heart model mu from base plane at time.
	 *
	 * @param	plane	The plane.
	 * @param	time 	The time.
	 */
	void SetHeartModelMuFromBasePlaneAtTime(const Plane& plane, double time);

	/**
	 * Sets the heart model lambda parameters at the given time.
	 *
	 * @param	lambdaParams	The lambda parameters.
	 * @param	time			The time.
	 */
	void SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time);

	/**
	 * Loads the template heart model.
	 *
	 * @param	numberOfModelFrames	Number of model frames.
	 */
	void LoadTemplateHeartModel(unsigned int numberOfModelFrames);

	/**
	 * Reset the heart nodes to their intial location.
	 *
	 * @param numberOfModelFrames	Number of model frames.
	 */
	void ResetHeartNodes(unsigned int numberOfModelFrames);

	/**
	 * Calculates the heart volume at the given time.  If no time is given then the heart volume is
	 * calculated for the current time.
	 *
	 * @param	surface	The surface (EPI or ENDO).
	 *
	 * @return	The calculated heart volume.
	 */
	double ComputeHeartVolume(HeartSurfaceEnum surface, double time = -1.0) const;

	/**
	 * Loads a heart model from the list of exnode files.  Each exnode file is listed with it's full
	 * path.
	 *
	 * @param	fullExelemFileName 	Full path file name of the exelem file.
	 * @param	fullExnodeFileNames	List of names of the exnode files.
	 */
	void LoadHeartModel(std::string fullExelemFileName, std::vector<std::string> fullExnodeFileNames);

	void WriteHeartModel(std::string dirname, unsigned int numberOfModelFrames);

	/**
	 * Adds currently selected node.  If no node is currently selected then do nothing.
	 */
	void AddCurrentlySelectedNode();

	/**
	 * Move currently selected node.  If no node is currently selected then do nothing.
	 */
	void MoveCurrentlySelectedNode();

	/**
	 * Delete currently selected node.  If no node is currently selected then do nothing.
	 */
	void DeleteCurrentlySelectedNode();

	/**
	 * Set the currently selected node with the label of the planes it lies upon.
	 * If no node is currently selected then do nothing.
	 */
	void AttachCurrentlySelectedNode();

	/**
	 * Start modelling action.  When starting the modelling action some things need
	 * to be done.
	 *	1. Create a region for the current modelling mode if one doesn't already exist.
	 *	2. Set the node tool up to work with the current modelling mode.
	 *	3. Record the current state of the cine.
	 */
	void StartModellingAction();

	/**
	 * End modelling action.  When ending the modelling action we reinstate the previous
	 * cine mode.
	 */
	void EndModellingAction();

	/**
	 * Test to see if modelling is currently active.
	 *
	 * \returns true if modelling is active, false otherwise.
	 */
	bool IsModellingActive() const { return modellingActive_; }

	/**
	 * Updates the frame number described by frameNumber.
	 *
	 * @param	frameNumber	The frame number.
	 */
	void UpdateFrameNumber(int frameNumber);

	/**
	 * Sets a time.
	 *
	 * @param	time	The time.
	 */
	void SetTime(double time);

	/**
	 * Process the modelling point details.  This function will sort the
	 * modelling points into modelling order then process the details and
	 * create the modelling points in the context and set them on the
	 * modeller as well.  To be used when loading in models/modelling points.
	 *
	 * @param	modellingPoints	The modelling points.
	 */
	void ProcessModellingPointDetails(ModellingPointDetails modellingPoints);

	/**
	 * Creates the progress dialog.
	 *
	 * @param	title  	The title.
	 * @param	message	The message.
	 * @param	max	   	The maximum.
	 */
	void CreateProgressDialog(std::string const& title, std::string const& message, int max);

	/**
	 * Updates the progress dialog described by count.
	 *
	 * @param	count	Number of.
	 */
	void UpdateProgressDialog(int count);

	/**
	 * Destroys the progress dialog.
	 */
	void DestroyProgressDialog();

private:

	/**
	 * Updates the user interface.
	 */
	void UpdateUI();

	/**
	 * Tests to see if any nodes are selected in the current mode.
	 *
	 * @param currentMode   currentMode the current mode to check for selection in.
	 * @return true if a node is selected, false otherwise
	 */
	bool IsNodeSelected(ModellingEnum currentMode) const;

	/**
	 * Clear the selection of nodes for the current mode.
	 * @param currentMode   currentMode the current mode to clear the selection in.
	 */
	void ClearSelection(ModellingEnum currentMode);

	/**
	 * Loads the hermite heart elements.  If the exelem file name
	 * is not given then the default elements for the heart model
	 * will be read in.
	 *
	 * @param	exelemFileName	(optional) filename of the exelem file.
	 */
	void LoadHermiteHeartElements(std::string exelemFileName = "");

	/**
	 * Set the field image as the texture for the material identified from
	 * the given name.
	 *
	 * \param name the name of the material to set the field image to.
	 * \param fieldImage the field image to use as a texture.
	 */
	void ChangeTexture(const std::string& name, Cmiss_field_image_id fieldImage);

	/**
	 * Change all the texture slices to display the correct texture for the given time.
	 *
	 * @param	time	The time.
	 */
	void ChangeAllTextures(double time);

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

	/**
	 * Creates the CAPClient icon in the cmgui context.
	 */
	void CreateCAPIconInContext() const;

	/**
	 * Show the UserCommentDlg to get a comment from the user for saving the model.
	 *
	 * @return true if the user saved the model, false otherwise.
	 */
	bool GetUserComment();

	/**
	 * Sets the 'text' of the field named in 'mode'.  The mode string must be a key of the
	 * statusTextStringsFieldMap_.
	 *
	 * @param	mode	The mode.
	 * @param	text	The text.
	 */
	void SetStatusTextString(std::string mode, std::string text) const;

	/**
	 * Sets the status text visibility.  The mode string must be a key of the
	 * statusTextStringsFieldMap_.
	 *
	 * @param	mode   	The mode.
	 * @param	visible	true to show, false to hide.
	 */
	void SetStatusTextVisibility(std::string mode, bool visible) const;

	/**
	 * Query if the MII named 'sliceName' is visible.
	 *
	 * @param	sliceName	Name of the slice.
	 *
	 * @return	true if mii visible, false if not.
	 */
	bool IsMIIVisible(const std::string& sliceName);

	/**
	 * Resets the mode choice.  Removes all but the APEX mode from the mode selection widget.
	 */
	void ResetModeChoice();

	/**
	 * Create the materials used by Cmgui for colouring visual entities.
	 */
	void CreateMaterials();

	/**
	 * Create the spectrum for setting the type dependent colour of the guide points
	 */
	void CreateSpectrum();

	/**
	 * Creates the fonts used by Cmgui.
	 */
	void CreateFonts();

	/**
	 * Removes the mii graphics.
	 */
	void RemoveMIIGraphics();

	/**
	 * Removes the status text strings.
	 */
	void RemoveStatusTextStrings();

	/**
	 * Sets the modelling callbacks.
	 */
	void SetModellingCallbacks();

	/**
	 * Removes the modelling callbacks.
	 */
	void RemoveModellingCallbacks();

	/**
	 * Sets the plane shifting callbacks.
	 */
	void SetPlaneShiftingCallbacks();

	/**
	 * Removes the plane shifting callbacks.
	 */
	void RemovePlaneShiftingCallbacks();

	/**
	 * Creates a modelling point.  This function creates a modelling point in the current context
	 * with the given values.  This is to facilitate the creation of nodes in the correct region and
	 * acting as a manual node tool for creating and defining nodes/modelling points.  The created
	 * modelling point is added to the corresponding modelling mode for it's type.
	 *
	 * @param	type		The type of modelling point to create.
	 * @param	position	The position.
	 * @param	time		The time.
	 * @return	the node id of the created node.
	 */
	int CreateModellingPoint(ModellingEnum type, const Point3D& position, double time);

	/**
	 * Creates the status text strings field renditions.  In this function
	 * we create the rendition element for the status text string fields.  A point
	 * glyph of type none is created for the status text field to use.  The point glyph
	 * is set in normalised window fit left coordinates.  The names of the fields are
	 *  #.	"modellingmode"
	 *  #.	"heartvolumeepi"
	 *  #.	"heartvolumeendo"
	 *
	 * This function populates the statusTextStringsFieldMap_ with the above strings for
	 * keys and it's Cmiss_field_id as it's value.
	 */
	void CreateStatusTextStringsFieldRenditions();

	/*
	 * Window widget event handlers.
	 */
	void Terminate(wxCloseEvent& event); /**< Widget event handler */
	void OnTogglePlay(wxCommandEvent& event); /**< Widget event handler */
	void OnObjectCheckListChecked(wxListEvent& event); /**< Widget event handler */
	void OnObjectCheckListSelected(wxListEvent& event); /**< Widget event handler */
	void OnToggleHideShowAll(wxCommandEvent& event); /**< Widget event handler */
	void OnToggleHideShowOthers(wxCommandEvent& event); /**< Widget event handler */
	void OnAnimationSliderEvent(wxCommandEvent& event); /**< Widget event handler */
	void OnAnimationSpeedControlEvent(wxCommandEvent& event); /**< Widget event handler */
	void OnMIICheckBox(wxCommandEvent& event); /**< Widget event handler */
	void OnWireframeCheckBox(wxCommandEvent& event); /**< Widget event handler */
	void OnBrightnessSliderEvent(wxCommandEvent& event); /**< Widget event handler */
	void OnContrastSliderEvent(wxCommandEvent& event); /**< Widget event handler */
	void OnAcceptClicked(wxCommandEvent& event); /**< Widget event handler */
	void OnModellingModeChanged(wxCommandEvent& event); /**< Widget event handler */
	void OnTogglePlaneShift(wxCommandEvent& event); /**< Widget event handler */
	void OnToggleModelling(wxCommandEvent& event); /**< Widget event handler */
	void OnModelDisplayModeChanged(wxCommandEvent& event); /**< Widget event handler */
	void OnDeleteModellingPointClicked(wxCommandEvent& event); /**< Widget event handler */

	/*
	 * Menu event handlers.
	 */
	void OnAbout(wxCommandEvent& event); /**< Menu event handler */
	void OnNewModel(wxCommandEvent& event); /**< Menu event handler */
	void OnOpenModel(wxCommandEvent& event); /**< Menu event handler */
	void OnOpenImageBrowser(wxCommandEvent& event); /**< Menu event handler */
	void OnCloseModel(wxCommandEvent& event); /**< Menu event handler */
	void OnSave(wxCommandEvent& event); /**< Menu event handler */
	void OnQuit(wxCommandEvent& event); /**< Menu event handler */
	void OnExportModel(wxCommandEvent& event); /**< Menu event handler */
	void OnExportHeartVolumes(wxCommandEvent& event); /**< Menu event handler */
	void OnExportToCmgui(wxCommandEvent& event); /**< Menu event handler */
	void OnExportModelToBinaryVolume(wxCommandEvent& event); /**< Menu event handler */
	void OnViewAll(wxCommandEvent& event); /**< Menu event handler */
	void OnViewStatusText(wxCommandEvent& event); /**< Menu event handler */
	void OnViewModellingPointLabels(wxCommandEvent& event); /**< Menu event handler */
	void OnViewLog(wxCommandEvent& event); /**< Menu event handler */

	/**
	 * On idle events are used by Cmgui to update graphics.
	 *
	 * @param [in,out]	event	The event.
	 */
	void OnIdle(wxIdleEvent& event);

	/**
	 * Executes the close window action.
	 *
	 * @param [in,out]	event	The event.
	 */
	void OnCloseWindow(wxCloseEvent& event);

	/**
	 * Make the connections for the widgets.
	 */
	void MakeConnections();

	CAPClient* mainApp_; /**< handle to the model class for this window */
	Cmiss_context_id cmissContext_; /**< handle to the context for this class. */
	SceneViewerPanel* cmguiPanel_; /**< handle to a cmgui panel class */
	TextureSliceMap textureSliceMap_; /**< A map of texture slices. */
	std::vector<Cmiss_field_image_id> fieldImages_; /**< A vector of field images. */
	Cmiss_graphic_id heart_epi_surface_; /**< The heart epicardium surface */
	Cmiss_graphic_id heart_endo_surface_;	/**< The heart endocardium surface */
	MIIGraphicMap miiMap_; /**< The mii map */
	HeartModel* heartModel_;  /**< The heart model pointer */
	Cmiss_time_keeper_id timeKeeper_; /**< time keeper */
	Cmiss_time_notifier_id timeNotifier_; /**< time notifier */

	std::string previousWorkingLocation_; /**< The previous working location */
	bool initialised_xmlUserCommentDialog_; /**< true if initialised user comment dialog xml resource */
	bool modellingActive_;  /**< true if modelling active, that is adding, moving or deleting a modelling point, false otherwise */
	bool zKeyDown_; /**< true if the z key is down, false otherwise */
	int sliceCheckedEventId_; /**< The event id of the slice list checked event.  For working around a selection bug in wxWidgets. Set to -1 if invalid. */
	StatusTextStringsFieldMap statusTextStringsFieldMap_;   /**< The status text strings field map */

	wxProgressDialog *progressDialog_; /**< Progess dialog pointer. */
};

} // end namespace cap

#endif

