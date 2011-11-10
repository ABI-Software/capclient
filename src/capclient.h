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
#include "imagebrowser.h"
#include "imagebrowserwindow.h"
#include "CAPXMLFile.h"
#include "CAPAnnotationFile.h"
#include "CAPModeller.h"
#include "CAPXMLFileHandler.h"
#include "heartmodel.h"
#include "IsoSurfaceCapture.h"
#include "utils/debug.h"

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
class CAPClient : private IImageBrowser//, public wxApp
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
	 * Destructor deletes modeller_ and ib_
	 */
	~CAPClient()
	{
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

	/**
	 * Gets a frame number for the given time.
	 *
	 * @param	time	The time.
	 *
	 * @return	The frame number for given time.
	 */
	int GetFrameNumberForTime(double time);

	/**
	 * Process the data points entered for current mode.
	 */
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

	/**
	 * Change modelling mode.
	 *
	 * @param	mode	The mode.
	 */
	void ChangeModellingMode(int mode)
	{
		modeller_->ChangeMode((CAPModeller::ModellingMode) mode);//FIX type unsafe
		gui_->UpdateModeSelectionUI(mode);
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

	/**
	 * Using the ImageBrowser class open some images from the given directory namespace.
	 * 
	 * \param imageDirname the directory name to open images from.
	 */
	void OpenImages(const std::string& imageDirname);

	/**
	 * Opens a model.
	 *
	 * @param	filename	Filename of the file.
	 */
	void OpenModel(const std::string& filename);

	/**
	 * Opens an annotation.
	 *
	 * @param	filename		Filename of the file.
	 * @param	imageDirname	Pathname of the image directory.
	 */
	void OpenAnnotation(std::string const& filename, std::string const& imageDirname);

	/**
	 * Saves a model.
	 *
	 * @param	dirname	   	Pathname of the directory.
	 * @param	userComment	The user comment.
	 */
	void SaveModel(std::string const& dirname, std::string const& userComment);

	/**
	 * Executes the export model action.
	 *
	 * @param	dirname	Pathname of the directory.
	 */
	void OnExportModel(std::string const& dirname)
	{
		IsoSurfaceCapture* iso = 0;//new IsoSurfaceCapture(imageSet_, heartModelPtr_.get(), gui_->GetCmissContext(), gui_->GetTimeKeeper());
		assert(iso);
		iso->OnExportModel(dirname);
	}

	/**
	 * Executes the export model to binary volume action.
	 *
	 * @param	dirname   	Pathname of the directory.
	 * @param	apexMargin	The apex margin.
	 * @param	baseMargin	The base margin.
	 * @param	spacing   	The spacing.
	 */
	void OnExportModelToBinaryVolume(std::string const& dirname, double apexMargin, double baseMargin, double spacing)
	{
		IsoSurfaceCapture* iso = 0; //new IsoSurfaceCapture(imageSet_, heartModelPtr_.get(), gui_->GetCmissContext(), gui_->GetTimeKeeper());
		assert(iso);
		iso->OnExportModelToBinaryVolume(dirname, apexMargin, baseMargin, spacing);
	}

	/**
	 * Adds a data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	position		   	The position.
	 * @param	time			   	The time.
	 */
	void AddDataPoint(Cmiss_node* dataPointID, Point3D const& position, double time)
	{
		modeller_->AddDataPoint(dataPointID, position, time);
	}

	/**
	 * Move data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	newPosition		   	The new position.
	 * @param	time			   	The time.
	 */
	void MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition, double time)
	{
		modeller_->MoveDataPoint(dataPointID, newPosition, time);
	}

	/**
	 * Removes the data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	time			   	The time.
	 */
	void RemoveDataPoint(Cmiss_node* dataPointID, double time) 
	{
		modeller_->RemoveDataPoint(dataPointID, time);
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

	/**
	 * Smooth along time.
	 */
	void SmoothAlongTime()
	{
		if (!modeller_)
		{
			return;//FIXME
		}
		modeller_->SmoothAlongTime();
		
		assert(heartModelPtr_);
		dbg("ED Volume(EPI) = " + toString(gui_->ComputeHeartVolume(EPICARDIUM, 0)));
		dbg("ED Volume(ENDO) = " + toString(gui_->ComputeHeartVolume(ENDOCARDIUM, 0)));
	}

	/**
	 * Sets the previous position.
	 *
	 * @param	position	The position.
	 */
	void SetPreviousPosition(const Point3D& position)
	{
		previousPosition_ = position;
	}

	/**
	 * Updates the plane position.  The plane position is updated according to
	 * the delta between the given position and the previous position.  There 
	 * is an expectation that the previous position has already been set.
	 *
	 * @param	regionName	Name of the region.
	 * @param	position  	The position.
	 */
	void UpdatePlanePosition(const std::string& regionName, const Point3D& position);
	
private:
	
	/**
	 * Initialise the window
	 */
	void Initialize();

	/**
	 * Enter initial state.
	 */
	void EnterInitState() 
	{
		gui_->EnterInitState();
		
		// Initialize input callback
		//Scene_viewer_add_input_callback(sceneViewer_, input_callback, (void*)this, 1/*add_first*/);

		// Also clean up cmgui objects such as scene, regions, materials ..etc
		cardiacAnnotationPtr_.reset(0);
		heartModelPtr_.reset(0);

		mainWindowState_ = INIT_STATE;
	}

	/**
	 * Enter images loaded state.
	 */
	void EnterImagesLoadedState();

	/**
	 * Enter model loaded state.
	 */
	void EnterModelLoadedState()
	{
		gui_->EnterModelLoadedState();
		
		gui_->SetModelVisibility(true);
		
		mainWindowState_ = MODEL_LOADED_STATE;
	}

	/**
	 * Terminates this object.
	 */
	void Terminate()
	{
		// ???????
		delete this;
	}
	
	/**
	 * Popluate the slice list in the gui.
	 */
	void PopulateSliceList();

	/**
	 * Initializes the mii.  This method only makes sense 
	 * when both the images and the model have been already 
	 * loaded.
	 */
	void InitializeMII();

	/**
	 * Updates the mii.
	 */
	void UpdateMII();

	/**
	 * Comparator for number of frames. 
	 */
	struct ComparatorForNumFrames
	{
		bool operator() (const LabelledSlice& a, const LabelledSlice& b)
		{
			return (a.GetDICOMImages().size() < b.GetDICOMImages().size());
		}
	};

	/**
	 * Initializes the model template.  If the images have not
	 * been loaded then this function will do nothing.
	 */
	void InitializeModelTemplate();

	/**
	 * Creates the modeller.
	 */
	void CreateModeller()
	{
		if (modeller_)
		{
			delete modeller_;
		}
		assert(heartModelPtr_);
		modeller_ = new CAPModeller(*heartModelPtr_); // initialise modeller and all the data points
	}

	/**
	 * Updates the states after loading model.
	 */
	void UpdateStatesAfterLoadingModel()
	{
		CreateModeller();
		gui_->UpdateModeSelectionUI(CAPModeller::APEX);
		InitializeMII(); // This turns on all MII's
	}

	/**
	 * Loads a heart model.
	 *
	 * @param	path		  	Full pathname of the file.
	 * @param	modelFilenames	The model filenames.
	 */
	void LoadHeartModel(std::string const& path, std::vector<std::string> const& modelFilenames)
	{
		assert(heartModelPtr_);
		heartModelPtr_->ReadModelFromFiles(path, modelFilenames);
		UpdateStatesAfterLoadingModel();
		EnterModelLoadedState();
	}
	
	/**
	 * Private default constructor - This class should be instantiated from the static factory method.
	 */
	CAPClient()
	: gui_(0)
	, ib_(0)
	, labelledSlices_(LabelledSlices())
	, heartModelPtr_(0)
	, modeller_(0)
	, mainWindowState_(INIT_STATE)
	, cardiacAnnotationPtr_(0)
	{
	}
	
	static CAPClient* instance_;	/**< The only instance of the CAPClient */
	CAPClientWindow* gui_;  /**< The graphical user interface */
	ImageBrowser* ib_;  /**< The image browser */
	
	LabelledSlices labelledSlices_; /**< The labelled slices */
	
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
	Point3D previousPosition_;  /**< The previous position */
};

} // namespace cap

#endif /* MAINAPP_H_ */
