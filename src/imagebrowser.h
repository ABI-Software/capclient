/*
 * imagebrowser.h
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#ifndef IMAGEBROWSER_H_
#define IMAGEBROWSER_H_

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
#include <api/cmiss_field_image.h>
}

#include "CAPMath.h"
#include "DICOMImage.h"
#include "FileSystem.h"
#include "SliceInfo.h"
#include "iimagebrowserwindow.h"
#include "imagebrowserwindow.h"
#include "cmguipanel.h"
#include "CAPAnnotationFile.h"

namespace cap
{

typedef std::pair<int, double> SliceKeyType;
//	typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;
typedef std::map<SliceKeyType, std::vector<DICOMPtr> > SliceMap;
typedef std::map<SliceKeyType, std::vector<Cmiss_field_image_id> > TextureMap;
typedef std::map<std::string, DICOMPtr> DICOMTable;
typedef std::map<std::string, Cmiss_field_image_id> TextureTable;

class ImageBrowser
{
public:
	
	void OnImageTableItemSelected(long int userDataPtr)
	{
		SliceMap::value_type* const sliceValuePtr = 
				reinterpret_cast<SliceMap::value_type* const>(userDataPtr);
		//	std::cout << "Series Num = " << (*sliceValuePtr).first.first << '\n';
		//	std::cout << "Distance to origin = " << (*sliceValuePtr).first.second << '\n';
		//	std::cout << "Image filename = " << (*sliceValuePtr).second[0]->GetFilename() << '\n';
				
		// Display the images from the selected row.
		SwitchSliceToDisplay(*sliceValuePtr);
	}
	
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
	void OnShortAxisButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("Short Axis");
		
		// update CardiacAnnotation
		std::vector<std::string> labelsToRemove;
		labelsToRemove.push_back("Short Axis");
		labelsToRemove.push_back("Long Axis");
		labelsToRemove.push_back("Horizonal Long Axis");
		labelsToRemove.push_back("Vertial Long Axis");
		labelsToRemove.push_back("Cine Loop");
		
		std::vector<Label> labelsToAdd;
		Label cine_loop = {"RID:10928", "Series", "Cine Loop"};
		Label short_axis = {"RID:10577", "Slice", "Short Axis"};
		labelsToAdd.push_back(cine_loop);
		labelsToAdd.push_back(short_axis);
		
		UpdateAnnotationOnImageCurrentlyOnDisplay(labelsToRemove, labelsToAdd);
	}

	/**
	* Update the annotation for the image currently on display
	* to show "Long Axis".
	*/
	void OnLongAxisButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("Long Axis");
		
		// update CardiacAnnotation
		std::vector<std::string> labelsToRemove;
		labelsToRemove.push_back("Short Axis");
		labelsToRemove.push_back("Long Axis");
		labelsToRemove.push_back("Cine Loop");
		
		std::vector<Label> labelsToAdd;
		Label cine_loop = {"RID:10928", "Series", "Cine Loop"};
		Label short_axis = {"RID:10571", "Slice", "Long Axis"};
		labelsToAdd.push_back(cine_loop);
		labelsToAdd.push_back(short_axis);
		
		UpdateAnnotationOnImageCurrentlyOnDisplay(labelsToRemove, labelsToAdd);
	}

	/**
	* Update the annotation for the image currently on display
	* to show nothing.
	*/
	void OnNoneButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("");
		
		// update CardiacAnnotation
		std::vector<std::string> labelsToRemove;
		labelsToRemove.push_back("Short Axis");
		labelsToRemove.push_back("Long Axis");
		labelsToRemove.push_back("Horizonal Long Axis");
		labelsToRemove.push_back("Vertical Long Axis");
		labelsToRemove.push_back("Cine Loop");
		
		std::vector<Label> labelsToAdd;
		
		UpdateAnnotationOnImageCurrentlyOnDisplay(labelsToRemove, labelsToAdd);
	}

	void OnOKButtonClicked()
	{
		std::cout << __func__ << '\n';
		// construct the data structure of type SlicesWithImages to pass to the main window
		SlicesWithImages slices;
		
		std::vector<std::pair<std::string, long int> > labels = gui_->GetListOfLabelsFromImageTable();
		
		int shortAxisCount = 1;
		int longAxisCount = 1;
		
		typedef std::pair<std::string, long int> LabelPair;
		BOOST_FOREACH(LabelPair const& labelPair, labels)
		{
			SliceMap::value_type* const sliceValuePtr = reinterpret_cast<SliceMap::value_type* const>(labelPair.second);
			SliceKeyType const& key = sliceValuePtr->first;
			std::string sliceName;
			std::string const& label = labelPair.first;
			if (label == "Short Axis")
			{
				sliceName = "SA" + boost::lexical_cast<std::string>(shortAxisCount++);
			}
			else if (label == "Long Axis")
			{
				sliceName = "LA" + boost::lexical_cast<std::string>(longAxisCount++);
			}
			else
			{
				throw std::logic_error("Invalid label : " + label);
			}
			SliceInfo sliceInfo(sliceName, sliceMap_[key], textureMap_[key]);
			slices.push_back(sliceInfo);
		}
		
		if (longAxisCount >= 10)
		{
			std::cout << "TOO MANY LONG AXES\n";
			gui_->CreateMessageBox("Too many long axes slices", "Invalid selection");
			return;
		}
		if (shortAxisCount >= 30)
		{
			std::cout << "TOO MANY SHORT AXES\n";
			gui_->CreateMessageBox("Too many short axes slices", "Invalid selection");
			return;
		}
		
		std::sort(slices.begin(), slices.end(), SliceInfoSortOrder());

		std::cout << __func__ << " : slices.size() = " << slices.size() <<  '\n';
		if (slices.empty())
		{
			std::cout << "Empty image set.\n";
			return;
		}
		
		client_->LoadImagesFromImageBrowserWindow(slices, cardiacAnnotation_);
		gui_->Close();
	}

	void OnCancelButtonClicked()
	{
		//TODO Cleanup textures - REVISE design
		// Should probably use reference counted smart pointer for Cmiss_texture
		// Since the ownership is shared between ImageSlice and this (ImageBrowserWindow)
		
		BOOST_FOREACH(TextureTable::value_type& value, textureTable_)
		{
			Cmiss_field_image_id tex = value.second;
			Cmiss_field_image_destroy(&tex);
		}
//		Close();
	}

	/**
	 * Sort the image table by current mode as set in 
	 * the order by radio box.
	 * 
	 * \param event the int describing which order mode is set. 
	 * Zero for SERIES_NUMBER and one for SERIES_NUMBER_AND_IMAGE_POSITION.
	 */
	void OnOrderByRadioBox(int event);
	
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
	 * Pass through functioin to allow downstream to control the visibility of the 
	 * window.
	 */
	void ShowWindow() const
	{
		gui_->Show(true);
	}
	
	/**
	 * Factory function for creating an image browser.
	 * 
	 * \param archiveFilename a string with the directory where the user wants images read from.
	 * \param client an interface pointer to the main application that allows limited access.
	 */
	static ImageBrowser* CreateImageBrowser(std::string const& archiveFilename, IImageBrowserWindow *client)
	{
		if (instance_ == 0)
		{
			wxXmlInit_ImageBrowserWindowUI();
			instance_ = new ImageBrowser(archiveFilename, client);
			ImageBrowserWindow* frame = new ImageBrowserWindow(instance_);
			frame->Show(true);
			instance_->SetImageBrowserWindow(frame);
			instance_->Initialize();
		}
		
		return instance_;
	}
	
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
	ImageBrowser(std::string const& archiveFilename, IImageBrowserWindow *client);
	
	/**
	 * Initialize the browser data and preview window.  Requires a valid ImageBrowserWindow
	 * to already be set.
	 */
	void Initialize();
	
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
	 * Reads the DICOM header of the files found in the given directory
	 * and populates dicomFileTable_.
	 */
	void ReadInDICOMFiles();
	
	/**
	 * Create textures from the dicom files found, stores the output in
	 * the slice map.
	 */
	void CreateTexturesFromDICOMFiles();
	
	/**
	 * Calculate the number of images stored in the slice map.
	 * 
	 * \returns the total number of images
	 */
	size_t GetSliceMapImageCount() const;
	
	void UpdateImageTableLabelsAccordingToCardiacAnnotation()
	{
		std::map<std::string, long int> uidToSliceKeyMap;
		BOOST_FOREACH(SliceMap::value_type const& value, sliceMap_)
		{
			BOOST_FOREACH(DICOMPtr const& dicomPtr, value.second)
			{
				uidToSliceKeyMap.insert(
						std::make_pair(dicomPtr->GetSopInstanceUID(),
						reinterpret_cast<long int>(&value)));
			}
		}
		
		std::map<long int, std::string> slicePtrToLabelMap;
		BOOST_FOREACH(ImageAnnotation const& imageAnno, cardiacAnnotation_.imageAnnotations)
		{
			std::string const& sopiuid = imageAnno.sopiuid;
			std::map<std::string, long int>::const_iterator itr = 
				uidToSliceKeyMap.find(sopiuid);
			if (itr == uidToSliceKeyMap.end())
			{
				std::cout << "Can't find the sopiuid: " << sopiuid << '\n';
			}
			long int sliceKeyPtr = itr->second;
			
			std::vector<Label>::const_iterator labelItr = 
					std::find_if(imageAnno.labels.begin(),
							imageAnno.labels.end(),
							boost::bind(&Label::label, _1) == "Cine Loop");
			if (labelItr == imageAnno.labels.end())
			{
				continue;
			}
			BOOST_FOREACH(Label const& annoLabel, imageAnno.labels)
			{
				std::string const& imageLabel = annoLabel.label;
				if (imageLabel == "Short Axis" || imageLabel == "Long Axis")
				{	
					std::cout << "uid: " << sopiuid << ", label = " << imageLabel << '\n';
					std::map<long int, std::string>::const_iterator itr = 
							slicePtrToLabelMap.find(sliceKeyPtr);
					
					std::string sliceLabel;
					if (itr != slicePtrToLabelMap.end() && itr->second != imageLabel)
					{
						//This means there is an image with a conflicting label in the
						//same slice.
						//This usually means the SortingMode is not properly set.
						//Changing the SortingMode should get rid of this problem.
						sliceLabel = "?";
					}
					else
					{
						sliceLabel = imageLabel;
					}
					
					slicePtrToLabelMap[sliceKeyPtr] = sliceLabel;
					gui_->SetImageTableRowLabelByUserData(sliceKeyPtr, sliceLabel);
				}
			}
		}
	}
	
	void UpdateAnnotationOnImageCurrentlyOnDisplay(std::vector<std::string> const& labelsToRemove, std::vector<Label> const& labelsToAdd)
	{
		std::vector<DICOMPtr> const& dicomPtrs = sliceMap_[sliceKeyCurrentlyOnDisplay_];
		BOOST_FOREACH(DICOMPtr const& dicomPtr, dicomPtrs)
		{
			std::string const& sopiuid = dicomPtr->GetSopInstanceUID();
			std::vector<ImageAnnotation>::iterator imageItr =
					std::find_if(cardiacAnnotation_.imageAnnotations.begin(),
								cardiacAnnotation_.imageAnnotations.end(),
								boost::bind(&ImageAnnotation::sopiuid,_1) == sopiuid);
			if (imageItr == cardiacAnnotation_.imageAnnotations.end())
			{
				// No annotation for the image exists
				ImageAnnotation imageAnno;
				imageAnno.sopiuid = sopiuid;
				cardiacAnnotation_.imageAnnotations.push_back(imageAnno);
				imageItr = cardiacAnnotation_.imageAnnotations.end() - 1;
			}
			else
			{
				// Remove conflicting labels if present
				std::vector<Label>::iterator labelItr =
						imageItr->labels.begin();
				while (labelItr != imageItr->labels.end())
				{
					bool erasePerformed = false;
					BOOST_FOREACH(std::string const& labelToRemove, labelsToRemove)
					{
						if (labelItr->label == labelToRemove)
						{
//							std::cout << "Erasing label : " << labelItr->label << '\n';
							imageItr->labels.erase(labelItr);
							erasePerformed = true;
						}
					}
					if (!erasePerformed)
					{
						++labelItr;
					}
				}
			}
//			std::cout << "Done erasing label\n";
			
			BOOST_FOREACH(Label const& label, labelsToAdd)
			{
				imageItr->labels.push_back(label);
			}
			
			// Remove ImageAnnotation if the update left it with no labels and no roi's
			if (imageItr->labels.empty() && imageItr->rOIs.empty())
			{
				cardiacAnnotation_.imageAnnotations.erase(imageItr);
			}
		}
	}
	
	/**
	 * Sorting mode enumeration
	 */
	enum SortingMode
	{
		SERIES_NUMBER,
		SERIES_NUMBER_AND_IMAGE_POSITION
	};
	
	/**
	 * Static instance of this pointer for the singleton pattern
	 */
	static ImageBrowser* instance_;
	
	/**
	 * Pointer to the gui(view) layer. (wxWidgets)
	 * Note this class does not own the gui_ object 
	 * (gui's lifecycle is managed by wxWidets framework)
	 */
	ImageBrowserWindow* gui_; 
	
	SortingMode sortingMode_;
	
	IImageBrowserWindow *client_;
	
	SliceMap sliceMap_;
	TextureMap textureMap_; // this could be merged with sliceMap_
	
	SliceKeyType sliceKeyCurrentlyOnDisplay_;
	int frameNumberCurrentlyOnDisplay_;
	
	DICOMTable dicomFileTable_; /**< unsorted list of all dicom files */
	TextureTable textureTable_; /**< unsorted list of all textures */
	
	std::string archiveFilename_;
	
	CardiacAnnotation cardiacAnnotation_;
};

} //namespace cap

#endif /* IMAGEBROWSER_H_ */
