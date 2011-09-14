/*
 * imagebrowserwindow.h
 *
 *  Created on: Jun 16, 2010
 *      Author: jchu014
 */

#ifndef IMAGEBROWSEWINDOW_H_
#define IMAGEBROWSEWINDOW_H_

#include <boost/scoped_ptr.hpp>
#include <boost/tr1/memory.hpp>
#include <string>
#include <vector>
#include <map>

#include <wx/wxprec.h>
#include <wx/notebook.h>
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

extern "C"
{
#include <api/cmiss_field_module.h>
#include <api/cmiss_scene_viewer.h>
}

#include "gui/ImageBrowserWindowUI.h"
#include "SliceInfo.h"
#include "cmguipanel.h"

class wxListCtrl;
class wxListEvent;
class wxProgressDialog;

namespace cap
{

class DICOMImage;
class CmguiPanel;
class CAPMaterial;
class ImageBrowser;

/**
 * The ImageBrowserWindow class is the view for a BrowserWindow.  The UI part of the 
 * class is converted from xrc at compilation time and inherited from so that the 
 * gui elements can be used directly in this derived class.
 */
class ImageBrowserWindow : public ImageBrowserWindowUI
{
public:
	/**
	 * Construct an ImageBrowserWindow as the view to the ImageBrowser data.
	 * 
	 * \param browser the data class for this window.
	 */
	ImageBrowserWindow(ImageBrowser *browser);
	
	/**
	 * Destructor
	 */
	virtual ~ImageBrowserWindow();
	
	/**
	 * Set the range of the image stack slider.  All image stacks
	 * start from one and stop at max.
	 * 
	 * \param max the maximum number for the slider to represent.
	 */
	void SetAnimationSliderMax(size_t max);
	
	void SetInfoField(std::string const& fieldName, std::string const& data);
	void PopulateImageTableRow(int rowNumber,
			int seriesNumber, std::string const& seriesDescription,
			std::string const& sequenceName, size_t numImages,
			long int const& userDataPtr);
	void SelectFirstRowInImageTable();
	void ClearImageTable();
	
	void CreateProgressDialog(std::string const& title, std::string const& message, int max);
	void UpdateProgressDialog(int count);
	void DestroyProgressDialog();
	
	void CreateMessageBox(std::string const& message, std::string const& caption);
	
	std::vector<std::pair<std::string, long int> > GetListOfLabelsFromImageTable() const;
	
	/**
	 * Create a field image from the given dicom pointer.  The name of the
	 * texture is the next unique name in the series 'tex_<number>'.  The image field
	 * returned must be destroyed by the receiver.
	 * 
	 * \param dicom the dicom pointer to create the field image from.
	 * \returns an accessed cmiss_field_image.
	 */
	Cmiss_field_image_id CreateFieldImage(DICOMPtr dicom);
	
	/**
	 * Resize the texture frame in the preview panel to the supplied
	 * width and height.
	 * 
	 * \param width the width in pixels to set the texture frame to.
	 * \param height the height in pixels to set the texture frame to.
	 */
	void ResizePreviewImage(int width, int height);
	
	/**
	 * Change the current image in the preview panel to the supplied image.  This
	 * image will be set into the CAPMaterial to be used as a texture.
	 * 
	 * \param image the field image to view in the preview panel.
	 */
	void ChangePreviewImage(Cmiss_field_image_id image);
	
	/**
	 * Adjust the scene viewing volume to the given radius.  This brings the scene
	 * slightly closer than the predefined view all function.
	 * 
	 * \param radius the radius to use for calculating the viewing volume.
	 */
	void FitSceneViewer(double radius);
	
	/**
	 * Force a redraw.  This is used, for instance, at times when 
	 * the slider is animating.
	 */
	void RefreshPreviewPanel();
	
	/**
	 * View all
	 */
	void ViewAll() const { cmguiPanel_->ViewAll(); }
	
	void SetImageTableRowLabel(long int index, std::string const& label);
	// Sets the label to the row in the image table with the matching user data
	void SetImageTableRowLabelByUserData(long int userData, std::string const& label);

//	void ShowImageAnnotation();
	
	void CreateImageTableColumns();
	void PutLabelOnSelectedSlice(std::string const& label);
	
	void ClearAnnotationTable();
	void CreateAnnotationTableColumns();
	void PopulateAnnotationTableRow(int rowNumber, std::string const& label, std::string const& rid, std::string const& scope);
	
private:
	std::string GetCellContentsString( long row_number, int column ) const;
	
	/**
	 * Create the preview scene.  Creates the image plane element and the 
	 * image texture surface.  Also the scene tumble rate is set to zero
	 * to force a 2D view.
	 */
	void CreatePreviewScene();
	
	/**
	 * Make the connections from the gui widgets to event handlers.
	 */
	void MakeConnections();
	
	/**
	 * Event handlers for the window.
	 */
	void OnImageTableItemSelected(wxListEvent& event);
	void OnCloseImageBrowserWindow(wxCloseEvent& event);
	void OnPreviewSelectionSliderEvent(wxCommandEvent& event);
	void OnBrightnessSliderEvent(wxCommandEvent& event);
	void OnContrastSliderEvent(wxCommandEvent& event);
	void OnShortAxisButtonEvent(wxCommandEvent& event);
	void OnLongAxisButtonEvent(wxCommandEvent& event);
	void OnNoneButtonEvent(wxCommandEvent& event);
	void OnOKButtonClicked(wxCommandEvent& event);
	void OnCancelButtonClicked(wxCommandEvent& event);
	void OnOrderByRadioBox(wxCommandEvent& event);
	void OnCaseSelected(wxCommandEvent& event);
	
	ImageBrowser *browser_; /**< the data for this window. */
	CmguiPanel *cmguiPanel_; /**< the cmgui panel where the preview scene is */
	
	std::tr1::shared_ptr<CAPMaterial> capmaterial_;
	
	static const std::string IMAGE_PREVIEW;
	
	boost::scoped_ptr<wxProgressDialog> progressDialogPtr_;
	
};

} // end namespace cap
#endif /* IMAGEBROWSEWINDOW_H_ */