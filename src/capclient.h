/*
 * capclient.h
 *
 *  Created on: Mar 9, 2011
 *      Author: jchu014
 */

#ifndef MAINAPP_H_
#define MAINAPP_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

extern "C"
{
#include <api/cmiss_region.h>
#include <api/cmiss_time_keeper.h>
#include <api/cmiss_time.h>
#include <api/cmiss_field_module.h>
}

#include "iimagebrowser.h"
#include "CAPMath.h"

#include "capclientconfig.h"
#include "capclientwindow.h"
#include "SliceInfo.h"
#include "DICOMImage.h"
#include "ImageSetBuilder.h"
#include "ImageSet.h"
#include "CmguiExtensions.h"
#include "imagebrowser.h"
#include "imagebrowserwindow.h"
#include "CAPXMLFile.h"
#include "CAPAnnotationFile.h"
#include "CAPModeller.h"
#include "CAPXMLFileHandler.h"
#include "HeartModel.h"
#include "IsoSurfaceCapture.h"

//class Cmiss_node;
class wxPanel;
		
namespace cap
{
class CAPClientWindow;

/**
 * \brief CAPCLient is the main data class for the application.
 * The gui(view) class for CAPClient is CAPClientWindow.  This class is follows 
 * the singleton pattern so that we can only have one CAPCLient.
 */
class CAPClient : private IImageBrowser, public wxApp
{
public:
	
	/**
	 * Here we create the main app, the constructor, copy constructor and 
	 * assignment operator are private.  This enables us to control the creation
	 * of the CAPClient object.
	 */
	static CAPClient* CreateCAPClient()
	{
		if (instance_ == 0)
		{
			instance_ = new CAPClient();
		}
		return instance_;
	}
	
	/**
	 * Destructor deletes imageSet_ and modeller_
	 */
	~CAPClient()
	{
		delete imageSet_;
		delete modeller_;
		if (ib_)
			delete ib_;
	}
	
	/**
	 * I would like to make this function private and replace it with a public
	 * function that returns the wxFrame.
	 */
	void SetCAPClientWindow(CAPClientWindow* win)
	{
		gui_ = win;
	}

	void SetImageVisibility(bool visibility, std::string const& sliceName = "")
	{
		if (sliceName.length()) //REVISE
		{
			const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();

			unsigned int i = find(sliceNames.begin(),sliceNames.end(), sliceName) - sliceNames.begin();
			assert(i < sliceNames.size());
			SetImageVisibility(visibility, i);
		}
		else
		{
			for (unsigned int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
			{
				SetImageVisibility(visibility, i);
			}
		}
	}
	
	void SetImageVisibility(bool visibility, int index)
	{
		if (miiIsOn_)	
			heartModelPtr_->SetMIIVisibility(visibility, index);
		imageSet_->SetVisible(visibility, index);
	}
	
	void OnMIICheckBox(bool checked)
	{
		miiIsOn_ = checked;
		assert(heartModelPtr_);
		for (unsigned int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
		{
			if (gui_->IsSliceChecked(i))
			{
				heartModelPtr_->SetMIIVisibility(checked,i);
			}
		}
	}
	
	void OnWireframeCheckBox(bool checked)
	{
		assert(heartModelPtr_);
		heartModelPtr_->SetModelVisibility(checked);
	}

	/**
	 * Gets a frame number for the given time.
	 *
	 * @param	time	The time.
	 *
	 * @return	The frame number for given time.
	 */
	int GetFrameNumberForTime(double time);

	void SetImageBrightness(double brightness)
	{
		imageSet_->SetBrightness(brightness);
	}
	
	void SetImageContrast(double contrast)
	{
		imageSet_->SetContrast(contrast);
	}
	
	void ProcessDataPointsEnteredForCurrentMode()
	{
		if (modeller_->OnAccept())
		{
			CAPModeller::ModellingMode mode = modeller_->GetCurrentMode();
			//== std::cout << "Current Mode = " << ModeStrings[mode] << '\n'; 
			gui_->UpdateModeSelectionUI(mode);
			if (mode == CAPModeller::GUIDEPOINT)
			{
				UpdateMII();
				EnterModelLoadedState();
			}
		}
	}
	
	void ChangeModellingMode(int mode)
	{
		modeller_->ChangeMode((CAPModeller::ModellingMode) mode);//FIX type unsafe
		gui_->UpdateModeSelectionUI(mode);
	}
	
	void LoadImages(SlicesWithImages const& slices);
	
	void LoadImagesFromXMLFile(SlicesWithImages const& slices)
	{
		LoadImages(slices);
		EnterImagesLoadedState();
	}
	
	virtual void LoadImagesFromImageBrowserWindow(const SlicesWithImages& slices, const CardiacAnnotation& anno);
	
	/**
	 * Implement pure virtual function from IImageBrowser interface 
	 * so that labelledSlices, labelledTextures and cardiac annotations 
	 * can be passed to this class.
	 */
	void LoadLabelledImages(const std::vector<LabelledSlice>& labelledSlices);

	/**
	 * Loads cardiac annotations.
	 *
	 * @param	anno	The cardiac annotations to load.
	 */
	void LoadCardiacAnnotations(const CardiacAnnotation& anno);

	void SetModelVisibility(bool visibility) {}
	
	void SetMIIVisibility(bool visibility, int sliceNum) {}

	/**
	 * Using the ImageBrowser class open some images from the given directory namespace.
	 * 
	 * \param imageDirname the directory name to open images from.
	 */
	void OpenImages(const std::string& imageDirname);
	
	void OpenModel(std::string const& filename);
	
	void OpenAnnotation(std::string const& filename, std::string const& imageDirname);
	
	void SaveModel(std::string const& dirname, std::string const& userComment);
	
	void StartPlaneShift()
	{
//		Cmiss_scene_viewer_remove_input_callback(sceneViewer_,
//						input_callback, (void*)this);
//		Cmiss_scene_viewer_add_input_callback(sceneViewer_,
//						input_callback_image_shifting, (void*)this, 1/*add_first*/);
	}
	
	void FinishPlaneShift()
	{
//		Cmiss_scene_viewer_remove_input_callback(sceneViewer_,
//						input_callback_image_shifting, (void*)this);
//		Cmiss_scene_viewer_add_input_callback(sceneViewer_,
//						input_callback, (void*)this, 1/*add_first*/);
		
		imageSet_->SetShiftedImagePosition();
	}
	
	void OnExportModel(std::string const& dirname)
	{
		std::cout << __func__ << "\n";
		IsoSurfaceCapture* iso = new IsoSurfaceCapture(imageSet_, heartModelPtr_.get(), gui_->GetCmissContext(), gui_->GetTimeKeeper());
		
		iso->OnExportModel(dirname);
	}
	
	void OnExportModelToBinaryVolume(std::string const& dirname, double apexMargin, double baseMargin, double spacing)
	{
		std::cout << __func__ << "\n";
		IsoSurfaceCapture* iso = new IsoSurfaceCapture(imageSet_, heartModelPtr_.get(), gui_->GetCmissContext(), gui_->GetTimeKeeper());
		
		iso->OnExportModelToBinaryVolume(dirname, apexMargin, baseMargin, spacing);
	}
	
	void AddDataPoint(Cmiss_node* dataPointID, Point3D const& position, double time)
	{
		modeller_->AddDataPoint(dataPointID, position, time);
	}
	
	void MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition, double time)
	{
		modeller_->MoveDataPoint(dataPointID, newPosition, time);
	}

	/**
	 * Gets the image plane for the image stack with the given label.
	 * Throws an exception if the label is not found, but this should not
	 * happen because the possible input values are dervied from this list
	 * initially.
	 *
	 * @param	label	The label of the image stack.
	 *
	 * @return	The image plane.
	 */
	const ImagePlane& GetImagePlane(const std::string& label);

	/**
	 * Gets the minimum number of frames.  This is the slice with the
	 * minimum number of images.
	 *
	 * @return	The number of frames.
	 */
	unsigned int GetMinimumNumberOfFrames() const;

	void RemoveDataPoint(Cmiss_node* dataPointID, double time) 
	{
		modeller_->RemoveDataPoint(dataPointID, time);
	}
	
	void SmoothAlongTime()
	{
		if (!modeller_)
		{
			return;//FIXME
		}
		modeller_->SmoothAlongTime();
		
		assert(heartModelPtr_);
		std::cout << "ED Volume(EPI) = " << heartModelPtr_->ComputeVolume(EPICARDIUM, 0) << '\n';
		std::cout << "ED Volume(ENDO) = " << heartModelPtr_->ComputeVolume(ENDOCARDIUM, 0) << '\n';
	}
	
private:
	
	/**
	 * Initialise the window
	 */
	void Initialize();
	
	void EnterInitState() 
	{
		gui_->EnterInitState();
		
		// Initialize input callback
		//Scene_viewer_add_input_callback(sceneViewer_, input_callback, (void*)this, 1/*add_first*/);

		// Also clean up cmgui objects such as scene, regions, materials ..etc
		cardiacAnnotationPtr_.reset(0);
		if(imageSet_)
		{
			delete imageSet_;
			imageSet_ = 0;
		}
		heartModelPtr_.reset(0);

		mainWindowState_ = INIT_STATE;
	}
	
	void EnterImagesLoadedState();
	
	void EnterModelLoadedState()
	{
		gui_->EnterModelLoadedState();
		
		assert(heartModelPtr_);
		heartModelPtr_->SetModelVisibility(true);
		
		mainWindowState_ = MODEL_LOADED_STATE;
	}
	
	void Terminate()
	{
		// ???????
		delete this;
	}
	
// 	void PopulateSliceList()
// 	{	
// 		const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
// 		std::vector<bool> visibilities;
// 		BOOST_FOREACH(std::string const& sliceName, sliceNames)
// 		{
// 			if (imageSet_->IsVisible(sliceName))
// 			{
// 				visibilities.push_back(true);
// 			}
// 			else
// 			{
// 				visibilities.push_back(false);
// 			}
// 		}
// 		gui_->PopulateSliceList(sliceNames, visibilities);
// 	}
	
	/**
	 * Popluate the slice list in the gui.
	 */
	void PopulateSliceList();
	
	void InitializeMII();
	
	void UpdateMII();
	
	struct ComparatorForNumFrames
	{
		bool operator() (const LabelledSlice& a, const LabelledSlice& b)
		{
			return (a.GetDICOMImages().size() < b.GetDICOMImages().size());
		}
	};

	void InitializeModelTemplate(const LabelledSlices& slices);
	
	void CreateModeller()
	{
		if (modeller_)
		{
			delete modeller_;
		}
		assert(heartModelPtr_);
		modeller_ = new CAPModeller(*heartModelPtr_); // initialise modeller and all the data points
	}
	
	void UpdateStatesAfterLoadingModel()
	{
		CreateModeller();
		gui_->UpdateModeSelectionUI(CAPModeller::APEX);
		
		UpdateModelVisibilityAccordingToUI();
		
		InitializeMII(); // This turns on all MII's
		UpdateMIIVisibilityAccordingToUI();
	}
	
	void LoadTemplateHeartModel(std::string const dirname, std::string const& path)
	{
		// This function is used to load the unfitted generic heart model.
		// Currently this is necessary. It might be possible to eliminate the use of generic model
		// in the future by creating the related cmgui nodes, fields and element completely thru cmgui api
		assert(heartModelPtr_);
		heartModelPtr_->ReadModelFromFiles(dirname, path);
		UpdateStatesAfterLoadingModel();
	}

	void UpdateModelVisibilityAccordingToUI()
	{
		assert(heartModelPtr_);
		heartModelPtr_->SetModelVisibility(wireFrameIsOn_);
	}
	
	void UpdateMIIVisibilityAccordingToUI()
	{
		// Update the visibility of each mii according to the ui status
		// ( = mii checkbox and the slice list)
		if (miiIsOn_)
		{
			for (unsigned int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
			{
				std::cout << "slice num = " << i << ", isChecked = " << gui_->IsSliceChecked(i) << '\n';
				if (!gui_->IsSliceChecked(i))
				{
					heartModelPtr_->SetMIIVisibility(false,i);
				}
			}
		}
		else
		{
			heartModelPtr_->SetMIIVisibility(false);
		}
	}
	
	void LoadHeartModel(std::string const& path, std::vector<std::string> const& modelFilenames)
	{
		assert(heartModelPtr_);
		heartModelPtr_->ReadModelFromFiles(path, modelFilenames);
		UpdateStatesAfterLoadingModel();
		EnterModelLoadedState();
	}
	
	// Private Constructor - This class should be instantiated from the static factory method
	CAPClient()
	: gui_(0)
	, ib_(0)
	, imageSet_(0)
	, labelledSlices_(LabelledSlices())
	, hideAll_(false)
	, miiIsOn_(false)
	, wireFrameIsOn_(false)
	, heartModelPtr_(0)
	, modeller_(0)
	, mainWindowState_(INIT_STATE)
	, cardiacAnnotationPtr_(0)
	{
	}
	
	static CAPClient* instance_;	/**< The instance */
	CAPClientWindow* gui_;  /**< The graphical user interface */
	ImageBrowser* ib_;  /**< The ib */
	
	ImageSet* imageSet_;	/**< Set the image belongs to */
	LabelledSlices labelledSlices_; /**< The labelled slices */
	
	bool hideAll_;  /**< true to hide, false to show all */
	bool miiIsOn_;  /**< true if mii is on */
	bool wireFrameIsOn_;	/**< true if wire frame is on */
	
	boost::scoped_ptr<HeartModel> heartModelPtr_;  /**< The heart model pointer */
	
	CAPModeller* modeller_; /**< The modeller */

	/**
	 * Values that represent CAPClientWindowState. 
	 */
	enum CAPClientWindowState
	{
		INIT_STATE,
		IMAGES_LOADED_STATE,
		MODEL_LOADED_STATE
	};

	CAPClientWindowState mainWindowState_;  /**< State of the main window */
	boost::scoped_ptr<CardiacAnnotation> cardiacAnnotationPtr_; /**< The cardiac annotation pointer */
	
};

} // namespace cap

#endif /* MAINAPP_H_ */
