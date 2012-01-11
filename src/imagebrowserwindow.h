/*
 * imagebrowserwindow.h
 *
 *  Created on: Jun 16, 2010
 *      Author: jchu014
 */

#ifndef IMAGEBROWSEWINDOW_H_
#define IMAGEBROWSEWINDOW_H_

#include "cmgui/sceneviewerpanel.h"

#include <boost/scoped_ptr.hpp>
#include <boost/tr1/memory.hpp>
#include <string>
#include <vector>
#include <map>

#include <wx/wxprec.h>
#include <wx/xrc/xmlres.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

extern "C"
{
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_scene_viewer.h>
}

#include "ui/ImageBrowserWindowUI.h"

class wxListCtrl;
class wxListEvent;
class wxProgressDialog;

namespace cap
{

class DICOMImage;
class Material;
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
	 * Destructor destroys any cmiss handles.
	 */
	virtual ~ImageBrowserWindow();
	
	/**
	 * Set the range of the image stack slider.  All image stacks
	 * start from one and stop at max.
	 * 
	 * \param max the maximum number for the slider to represent.
	 */
	void SetAnimationSliderMax(size_t max);

	/**
	 * Sets an information field.
	 *
	 * @param	fieldName	Name of the field.
	 * @param	data	 	The data.
	 */
	void SetInfoField(std::string const& fieldName, std::string const& data);

	/**
	 * Populate image table row.
	 *
	 * @param	rowNumber		 	The row number.
	 * @param	seriesNumber	 	The series number.
	 * @param	seriesDescription	Information describing the series.
	 * @param	sequenceName	 	Name of the sequence.
	 * @param	numImages		 	Number of images.
	 * @param	userDataPtr		 	The user data pointer.
	 */
	void PopulateImageTableRow(int rowNumber,
			int seriesNumber, std::string const& seriesDescription,
			std::string const& sequenceName, size_t numImages,
			long int const& userDataPtr);

	/**
	 * Select first row in image table.
	 */
	void SelectFirstRowInImageTable();

	/**
	 * Clears the image table.
	 */
	void ClearImageTable();

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

	/**
	 * Creates a message box.
	 *
	 * @param	message	The message.
	 * @param	caption	The caption.
	 */
	void CreateMessageBox(std::string const& message, std::string const& caption);

	/**
	 * Gets the list of labels from image table.
	 *
	 * @return	The list of labels from image table.
	 */
	std::vector<std::pair<std::string, long int> > GetListOfLabelsFromImageTable() const;

	/**
	 * Create a field image from the given filename.  The name of the texture is 'tex_' + the file
	 * name.  The image field returned must be destroyed by the receiver.
	 *
	 * @param	filename	Filename of the file.
	 *
	 * @return	An accessed field image id.
	 */
	Cmiss_field_image_id CreateFieldImage(const std::string& filename);
	
	/**
	 * Resize the texture frame in the preview panel to the supplied
	 * width and height.
	 * 
	 * \param width the width to set the texture frame to.
	 * \param height the height to set the texture frame to.
	 */
	void ResizePreviewImage(double width, double height);
	
	/**
	 * Change the current image in the preview panel to the supplied image.  This
	 * image will be set into the Material to be used as a texture.
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
	 * View all
	 */
	void ViewAll() const { cmguiPanel_->ViewAll(); }

	/**
	 * Sets an image table row label.
	 *
	 * @param	index	Zero-based index of the.
	 * @param	label	The label.
	 */
	void SetImageTableRowLabel(long int index, std::string const& label);
	// Sets the label to the row in the image table with the matching user data

	/**
	 * Sets an image table row label by user data.
	 *
	 * @param	userData	Information describing the user.
	 * @param	label   	The label.
	 */
	void SetImageTableRowLabelByUserData(long int userData, std::string const& label);

//	void ShowImageAnnotation();

	/**
	 * Creates the image table columns.
	 */
	void CreateImageTableColumns();

	/**
	 * Puts a label on selected slice.
	 *
	 * @param	label	The label.
	 */
	void PutLabelOnSelectedSlice(std::string const& label);

	/**
	 * Clears the annotation table.
	 */
	void ClearAnnotationTable();

	/**
	 * Creates the annotation table columns.
	 */
	void CreateAnnotationTableColumns();

	/**
	 * Populate annotation table row.
	 *
	 * @param	rowNumber	The row number.
	 * @param	label	 	The label.
	 * @param	rid		 	The rid.
	 * @param	scope	 	The scope.
	 */
	void PopulateAnnotationTableRow(int rowNumber, std::string const& label, std::string const& rid, std::string const& scope);

	/**
	 * Sets an annotation string.
	 *
	 * @param	text	The text.
	 */
	void SetAnnotationString(std::string text);

	/**
	 * Sets the image location for the image location widget.
	 *
	 * @param	dir	The dir.
	 */
	void SetImageLocation(const std::string& dir);

	/**
	 * Gets the image location.
	 *
	 * @return	The image location.
	 */
	std::string GetImageLocation() const;

private:

	/**
	 * Query if this object has at least one image labelled.
	 *
	 * @return	true if at least one image is labelled, false if not.
	 */
	bool IsAtLeastOneImageLabelled() const;

	/**
	 * Gets a cell contents string.
	 *
	 * @param	row_number	The row number.
	 * @param	column	  	The column.
	 *
	 * @return	The cell contents string.
	 */
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
	void OnImageTableItemSelected(wxListEvent& event); /**< event handler. */
	void OnCloseImageBrowserWindow(wxCloseEvent& event); /**< event handler. */
	void OnPreviewSelectionSliderEvent(wxCommandEvent& event); /**< event handler. */
	void OnBrightnessSliderEvent(wxCommandEvent& event); /**< event handler. */
	void OnContrastSliderEvent(wxCommandEvent& event); /**< event handler. */
	void OnShortAxisButtonEvent(wxCommandEvent& event); /**< event handler. */
	void OnLongAxisButtonEvent(wxCommandEvent& event); /**< event handler. */
	void OnNoneButtonEvent(wxCommandEvent& event); /**< event handler. */
	void OnOKButtonClicked(wxCommandEvent& event); /**< event handler. */
	void OnCancelButtonClicked(wxCommandEvent& event); /**< event handler. */
	void OnOrderByRadioBox(wxCommandEvent& event); /**< event handler. */
	void OnCaseSelected(wxCommandEvent& event); /**< event handler. */
	void OnChooseDirectory(wxCommandEvent& event); /**< event handler. */
	void OnChooseArchive(wxCommandEvent& event); /**< event handler. */
	
	ImageBrowser *browser_; /**< the data for this window. */
	Cmiss_context_id cmissContext_; /**< handle to the context for this class. */
	SceneViewerPanel *cmguiPanel_; /**< the cmgui panel where the preview scene is. */
	
	Material* material_; /**< material. */
	
	static const std::string IMAGE_PREVIEW; /**< Image preview string. */
	
	boost::scoped_ptr<wxProgressDialog> progressDialogPtr_; /**< Progess dialog pointer. */
	
};

} // end namespace cap
#endif /* IMAGEBROWSEWINDOW_H_ */
