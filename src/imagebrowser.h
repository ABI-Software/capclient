/*
 * imagebrowser.h
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#ifndef IMAGEBROWSER_H_
#define IMAGEBROWSER_H_

#include "math/algebra.h"
#include "dicomimage.h"
#include "utils/filesystem.h"
#include "iimagebrowser.h"
#include "imagebrowserwindow.h"
#include "zinc/sceneviewerpanel.h"
#include "io/annotationfile.h"
#include "io/imagesource.h"

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

extern "C"
{
#include <zn/cmiss_field_image.h>
}

namespace cap
{
/**
 * Slice key is described by the position in the stack and the dot product of the normal
 * and the position to determine a manner in differentiating slices in different planes.
 */
typedef std::pair<int, double> SliceKeyType;
typedef std::map<SliceKeyType, std::vector<DICOMPtr> > SliceMap; /**< A map of dicom images using image position as a key. */
typedef std::map<SliceKeyType, std::vector<Cmiss_field_image_id> > TextureMap; /**< A map of textures using image position as a key. */
typedef std::map<std::string, DICOMPtr> DICOMTable; /**< A map of dicom images using the dicom image name as a key. */
typedef std::map<std::string, Cmiss_field_image_id> TextureTable; /**< A map of textures using the texture name as a key. */
typedef std::map<std::string, bool> StudyInstanceMap; /**< A map of study instance */
typedef std::map<std::string, std::string> CaseListMap; /**< A case list map */

/**
 * The ImageBrowser class is the data class paired with ImageBrowserWindow.  This class is used for browsing dicom images.
 * Images that are labelled as short axis or long axis are, when the 'OK' button is pressed loaded into the main window
 * and made ready for use in modelling.  This class will preview images when the are selected in the image table.  Details
 * of the dicom image and annotations are also displayed.
 *
 * This class will delete itself and the ImageBrowserWindow when the ImageBrowserWindow is closed.
 */
class ImageBrowser
{
public:

	/**
	 * On image table item selected.
	 */
	void OnImageTableItemSelected(long int userDataPtr);

	/**
	 * Inform the model that the preview image has changed.
	 *
	 * \param frameNumber the index of the image required from the
	 * current image stack.
	 */
	void ChangePreviewImage(int frameNumber);

	/**
	 * Update the annotation for the image currently on display
	 * to show "Short Axis".
	 */
	void OnShortAxisButtonEvent();

	/**
	* Update the annotation for the image currently on display
	* to show "Long Axis".
	*/
	void OnLongAxisButtonEvent();

	/**
	* Update the annotation for the image currently on display
	* to show nothing.
	*/
	void OnNoneButtonEvent();

	/**
	 * On OK button clicked get all the slices marked with
	 * either "Short Axis" or "Long Axis" (but not more than
	 * 10 long axis and 30 short axis???) and return them to CAPClient.
	 */
	void OnOKButtonClicked();

	/**
	 * On cancel button clean up any accessed Cmgui handles and
	 * close the dialog.
	 */
	void OnCancelButtonClicked();

	/**
	 * Sort the image table by current mode as set in
	 * the order by radio box.
	 *
	 * \param event the int describing which order mode is set.
	 * Zero for SERIES_NUMBER and one for SERIES_NUMBER_AND_IMAGE_POSITION.
	 */
	void OnOrderByRadioBox(int event);

	void OnCaseSelected(const std::string &caseName);

	/**
	 * Set the image table with the given cardiac annotation.
	 *
	 * \param anno the cardiac annotation to populate the image table with.
	 */
	void SetAnnotation(CardiacAnnotation const& anno)
	{
		cardiacAnnotation_ = anno;
		PopulateImageTable();
	}

	/**
	 * Factory function for creating an image browser.
	 *
	 * @param	workingLocation	The working location.
	 * @param [in,out]	client 	If non-null, the client interface pointer that allows limited access to the CAPClient class.
	 */
	static void CreateImageBrowser(std::string const& workingLocation, const CardiacAnnotation& annotation, IImageBrowser *client)
	{
		if (!wxXmlInitialised_)
		{
			wxXmlInitialised_ = true;
			wxXmlInit_imagebrowserwindowui();
		}
		ImageBrowser* imageBrowser = new ImageBrowser(workingLocation, annotation, client);
		ImageBrowserWindow* frame = new ImageBrowserWindow(imageBrowser);
		frame->Show(true);
		imageBrowser->SetImageBrowserWindow(frame);
		imageBrowser->Initialize();
	}

	/**
	 * Choose image directory.
	 */
	void ChooseImageDirectory();

	/**
	 * Choose annotation file.
	 */
	void ChooseAnnotationFile();

	/**
	 * Choose archive file.
	 */
	void ChooseArchiveFile();

	/**
	 * Destructor for ImageBrowser
	 */
	~ImageBrowser();

private:
	/**
	 * Private constructor means we can control the construciton of this class, using the
	 * factory pattern.
	 *
	 * \param archiveFilename a string with the directory where the user wants images read from.
	 * \param manager a pointer to a cmgui manager from facilitating interactions with the cmgui libraries.
	 * \param client an interface pointer to the main application that allows limited access.
	 */
	ImageBrowser(std::string const& archiveFilename, const CardiacAnnotation& annotation, IImageBrowser *client);

	/**
	 * Initialize the browser data and preview window.  Requires a valid ImageBrowserWindow
	 * to already be set.
	 */
	void Initialize();

	/**
	 * Loads the images from the directory specified in previousWorkingLocation_.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool LoadImages(const std::string& pathfile);

	/**
	 * Set the image browser window pointer to give this model
	 * class access to the gui(view).
	 *
	 * \param gui an ImageBrowserWindow pointer
	 */
	void SetImageBrowserWindow(ImageBrowserWindow* gui)
	{
		gui_ = gui;
	}

	/**
	 * Change the preview panel to display the given slice.
	 *
	 * \param slice a const reference to a pair of a slice key and a dicom image vector.
	 */
	void SwitchSliceToDisplay(SliceMap::value_type const& slice);

	/**
	 * Sets the annotation table from information stored in cardiacAnnotation_
	 * using the current slice key and the frame number supplied to select
	 * which annotation to display.
	 *
	 * \param image_index the index of the image to display.
	 */
	void ChangeImageAnnotation(int image_index);

	/**
	 * Update the patient infromation panel from the given dicom
	 * image.
	 *
	 * \param image a const reference shared pointer to a dicom image
	 */
	void UpdatePatientInfoPanel(DICOMPtr const& image);

	/**
	 * Update the series information panel.
	 *
	 * \param dicomPtr a const reference shared pointer to dicom image
	 */
	void UpdateSeriesInfoPanel(DICOMPtr const& dicomPtr);

	/**
	 * Sort the DICOM files by series number or series number and image position.
	 * The files sorted are taken from the dicomFileTable_ and stored in sliceMap_.
	 * This function uses stable sort to preserve the order of the images, in case
	 * the instance number element is missing
	 *
	 * For sorting by series number the distance from origin is set to zero for all
	 * images.  For sorting by series number and image position the distance from
	 * origin is determined by calculating the dot product of the position and the
	 * normal vector.
	 */
	void SortDICOMFiles();

	/**
	 * Construct the texture map.
	 */
	void ConstructTextureMap();

	/**
	 * Populate the image table with information currently
	 * stored in the slice map.
	 */
	void PopulateImageTable();

	/**
	 * Create textures from the dicom files found, stores the output in
	 * the slice map.
	 */
	void CreateTexturesFromDICOMFiles(const std::string &path);

	/**
	 * Create textures from the given archive filename.
	 *
	 * @param filename	The filename to use.
	 */
	void CreateTexturesFromArchive(const std::string& filename);

	/**
	 * Create Image and add to tables textureTable_ and dicomFileTable with
	 * the image source filename as the key.
	 *
	 * @param imageSource	The image source.
	 */
	void AddImageToTable(const cap::ImageSource &imageSource);

	/**
	 * Calculate the number of images stored in the slice map.
	 *
	 * \returns the total number of images
	 */
	size_t GetSliceMapImageCount() const;

	/**
	 * Update the image table labels according to cardiac annotation.
	 */
	void UpdateImageTableLabelsAccordingToCardiacAnnotation();

	/**
	 * Update the annotation for the image currently on display.
	 *
	 * \param labelsToRemove the vector of labels to remove from the annotation.
	 * \param labelsToAdd the vector of labels to add to the annotation.
	 */
	void UpdateAnnotationOnImageCurrentlyOnDisplay(const std::vector<std::string>& labelsToRemove, const std::vector<Label>& labelsToAdd);

	/**
	 * Sorting mode enumeration
	 */
	enum SortingMode
	{
		SERIES_NUMBER,
		SERIES_NUMBER_AND_IMAGE_POSITION
	};

	/**
	 * Destroys the field image handles in the textrue table and then clears the table.
	 */
	void ClearTextureTable();

	/**
	 * Destroys the field image handles in the textrue map and then clears the map.
	 */
	void ClearTextureMap();

	/**
	 * Static variable to be set when wxXmlInit_ImageBrowserWindowUI
	 * is called for the first time so that we don't call it again.
	 */
	static bool wxXmlInitialised_;

	/**
	 * Pointer to the gui(view) layer. (wxWidgets)
	 * Note this class does not own the gui_ object
	 * (gui's lifecycle is managed by wxWidets framework)
	 */
	ImageBrowserWindow* gui_;

	std::string previousWorkingLocation_;   /**< The previous working location */
	SortingMode sortingMode_; /**< class state variable to track the current sorting mode. */
	std::string caseName_; /**< class state variable to track the current case */

	IImageBrowser *client_; /**< Pointer to CAPClient using an inteface class to restrict access. */

	CaseListMap caseListMap_; /**< Map of cases to sopiuid */
	StudyInstanceMap studyInstanceMap_; /**< Map of study instance uid */
	SliceMap sliceMap_; /**< A map of dicom images using image position for a key. */
	TextureMap textureMap_; /**< A map of textures using image position for a key. */

	SliceKeyType sliceKeyCurrentlyOnDisplay_; /**< class state variable to track image currently on display. */
	int frameNumberCurrentlyOnDisplay_; /**< class state variable to track frame currently on display. */

	DICOMTable dicomFileTable_; /**< unsorted list of all dicom files. */
	TextureTable textureTable_; /**< unsorted list of all textures. */

	CardiacAnnotation cardiacAnnotation_; /**< Annotation of something TODO: more... */
};

} //namespace cap

#endif /* IMAGEBROWSER_H_ */
