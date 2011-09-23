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

#include "iimagebrowserwindow.h"
#include "CAPMath.h"

#include "Config.h"
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
#include "CAPModelLVPS4X4.h"
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
class CAPClient : private IImageBrowserWindow, public wxApp
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
	}
	
	void SetCAPClientWindow(CAPClientWindow* win)
	{
		gui_ = win;
	}

	void OnTogglePlay()
	{
		if (animationIsOn_)
		{
			StopCine();
		}
		else
		{
			PlayCine();
		}
		return;
	}
	
	void SetImageVisibility(bool visibility, std::string const& sliceName = "")
	{
		if (sliceName.length()) //REVISE
		{
			const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();

			int i = find(sliceNames.begin(),sliceNames.end(), sliceName) - sliceNames.begin();
			assert(i < sliceNames.size());
			SetImageVisibility(visibility, i);
		}
		else
		{
			for (int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
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
	
	void OnSliceSelected(std::string const& sliceName);
	
	void OnAnimationSliderEvent(double time);
	
	void OnAnimationSpeedControlEvent(double speed)
	{
		//Time_keeper_set_speed(timeKeeper_, speed);	
		Refresh3DCanvas(); // forces redraw while silder is manipulated
	}
	
	void Refresh3DCanvas()
	{
		//if (sceneViewer_) 
		//{
			//Scene_viewer_redraw_now(sceneViewer_);
		//}
	}
	
	void OnToggleHideShowAll() {}//????
	
	void OnToggleHideShowOthers() {}//?????
	
	void OnMIICheckBox(bool checked)
	{
		miiIsOn_ = checked;
		assert(heartModelPtr_);
		for (int i = 0; i < imageSet_->GetNumberOfSlices(); i++)
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
	
	int GetFrameNumberForTime(double time);
	void SetImageBrightness(double brightness)
	{
		imageSet_->SetBrightness(brightness);
		Refresh3DCanvas();
	}
	
	void SetImageContrast(double contrast)
	{
		imageSet_->SetContrast(contrast);
		Refresh3DCanvas();
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
		
		Refresh3DCanvas();
	}
	
	void ChangeModellingMode(int mode)
	{
		modeller_->ChangeMode((CAPModeller::ModellingMode) mode);//FIX type unsafe
		gui_->UpdateModeSelectionUI(mode);
		Refresh3DCanvas();
	}
	
	void LoadImages(SlicesWithImages const& slices);
	
	void LoadImagesFromXMLFile(SlicesWithImages const& slices)
	{
		LoadImages(slices);
		EnterImagesLoadedState();
	}
	
	virtual void LoadImagesFromImageBrowserWindow(const SlicesWithImages& slices, const CardiacAnnotation& anno);
	
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
		IsoSurfaceCapture* iso = new IsoSurfaceCapture(imageSet_, heartModelPtr_.get(), gui_->GetCmissContext(), timeKeeper_);
		
		iso->OnExportModel(dirname);
	}
	
	void OnExportModelToBinaryVolume(std::string const& dirname, double apexMargin, double baseMargin, double spacing)
	{
		std::cout << __func__ << "\n";
		IsoSurfaceCapture* iso = new IsoSurfaceCapture(imageSet_, heartModelPtr_.get(), gui_->GetCmissContext(), timeKeeper_);
		
		iso->OnExportModelToBinaryVolume(dirname, apexMargin, baseMargin, spacing);
	}
	
	void AddDataPoint(Cmiss_node* dataPointID, Point3D const& position, double time)
	{
		modeller_->AddDataPoint(dataPointID, position, time);
		Refresh3DCanvas(); // need to force refreshing
	}
	
	void MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition, double time)
	{
		modeller_->MoveDataPoint(dataPointID, newPosition, time);
		Refresh3DCanvas(); // need to force refreshing
	}
	
	void RemoveDataPoint(Cmiss_node* dataPointID, double time) 
	{
		modeller_->RemoveDataPoint(dataPointID, time);
		Refresh3DCanvas();
	}
	
	void SmoothAlongTime()
	{
		if (!modeller_)
		{
			return;//FIXME
		}
		modeller_->SmoothAlongTime();
		Refresh3DCanvas();
		
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
		capXMLFilePtr_.reset(0);
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
		
		StopCine();
		assert(heartModelPtr_);
		heartModelPtr_->SetModelVisibility(true);
		
		mainWindowState_ = MODEL_LOADED_STATE;
	}
	
	void PlayCine()
	{
		Cmiss_time_keeper_play(timeKeeper_, CMISS_TIME_KEEPER_PLAY_FORWARD);
		Cmiss_time_keeper_set_repeat_mode(timeKeeper_, CMISS_TIME_KEEPER_REPEAT_MODE_PLAY_LOOP);
		Cmiss_time_keeper_set_frame_mode(timeKeeper_, CMISS_TIME_KEEPER_FRAME_MODE_PLAY_REAL_TIME);
//		Time_keeper_play(timeKeeper_,TIME_KEEPER_PLAY_FORWARD);
		//Time_keeper_set_play_loop(timeKeeper_);
		//Time_keeper_set_play_every_frame(timeKeeper_);
		//Time_keeper_set_play_skip_frames(timeKeeper_);
		this->animationIsOn_ = true;
		
		gui_->PlayCine();
	}
	
	void StopCine()
	{
		Cmiss_time_keeper_stop(timeKeeper_);
		//Time_keeper_stop(timeKeeper_);
		this->animationIsOn_ = false;
		
		gui_->StopCine();
	}
	
	void Terminate()
	{
		// ???????
		delete this;
	}
	
	void PopulateSliceList()
	{	
		const std::vector<std::string>& sliceNames = imageSet_->GetSliceNames();
		std::vector<bool> visibilities;
		BOOST_FOREACH(std::string const& sliceName, sliceNames)
		{
			if (imageSet_->IsVisible(sliceName))
			{
				visibilities.push_back(true);
			}
			else
			{
				visibilities.push_back(false);
			}
		}
		gui_->PopulateSliceList(sliceNames, visibilities);
	}
	
	void InitializeMII();
	
	void UpdateMII();
	
	struct ComparatorForNumFrames
	{
		bool operator() (SliceInfo const& a, SliceInfo const& b)
		{
			return (a.GetDICOMImages().size() < b.GetDICOMImages().size());
		}
	};

	void InitializeModelTemplate(SlicesWithImages const& slices);
	
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
			const int numberOfSlices = imageSet_->GetNumberOfSlices();
			for (int i = 0; i < numberOfSlices; i++)
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
/*	CAPClient(CmguiPanel *manager)
	:
		cmguiManager_(manager),
		imageSet_(0),
		animationIsOn_(false),
		hideAll_(false),
		miiIsOn_(false),
		wireFrameIsOn_(false),
		heartModelPtr_(0),
		modeller_(0),
		capXMLFilePtr_(0),
		cardiacAnnotationPtr_(0),
		gui_(0),
		//context_(manager.GetCmissContext()),
		timeKeeper_(Cmiss_context_get_default_time_keeper(manager->GetCmissContext())),
		timeNotifier_(0)
	{}
	*/
	CAPClient()
	: gui_(0)
	, ib_(0)
	, timeKeeper_(0)
	, timeNotifier_(0)
	, imageSet_(0)
	, animationIsOn_(false)
	, hideAll_(false)
	, miiIsOn_(false)
	, wireFrameIsOn_(false)
	, heartModelPtr_(0)
	, modeller_(0)
	, mainWindowState_(INIT_STATE)
	, capXMLFilePtr_(0)
	, cardiacAnnotationPtr_(0)
	{
	}
	
	static CAPClient* instance_;
	CAPClientWindow* gui_;
	ImageBrowser* ib_;
	
	// Cmiss_context_id context_;
	Cmiss_time_keeper_id timeKeeper_;
	Cmiss_time_notifier_id timeNotifier_;
	ImageSet* imageSet_;
	
	bool animationIsOn_;
	bool hideAll_;
	bool miiIsOn_;
	bool wireFrameIsOn_;
	
	boost::scoped_ptr<CAPModelLVPS4X4> heartModelPtr_;
	
	CAPModeller* modeller_;
	
	enum CAPClientWindowState
	{
		INIT_STATE,
		IMAGES_LOADED_STATE,
		MODEL_LOADED_STATE
	};

	CAPClientWindowState mainWindowState_;
	boost::scoped_ptr<CAPXMLFile> capXMLFilePtr_;
	boost::scoped_ptr<CardiacAnnotation> cardiacAnnotationPtr_;
	
	//DECLARE_EVENT_TABLE();
};

} // namespace cap

#endif /* MAINAPP_H_ */
