/*
 * capclient.h
 *
 *  Created on: Mar 9, 2011
 *      Author: jchu014
 */

#ifndef MAINAPP_H_
#define MAINAPP_H_


#include "iimagebrowser.h"
#include "math/algebra.h"

#include "capclientconfig.h"
#include "capclientwindow.h"
#include "SliceInfo.h"
#include "dicomimage.h"
#include "ImageSetBuilder.h"
#include "ImageSet.h"
#include "imagebrowser.h"
#include "imagebrowserwindow.h"
#include "CAPXMLFile.h"
#include "CAPAnnotationFile.h"
#include "model/modeller.h"
#include "CAPXMLFileHandler.h"
#include "model/heart.h"
#include "IsoSurfaceCapture.h"
#include "utils/misc.h"
#include "utils/debug.h"

extern "C"
{
#include <zn/cmiss_region.h>
#include <zn/cmiss_time_keeper.h>
#include <zn/cmiss_time.h>
#include <zn/cmiss_field_module.h>
}

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

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
class CAPClient : private IImageBrowser, public IModeller//, public wxApp
{
public:
	
	/**
	 * Here we create the main app, the constructor, copy constructor and 
	 * assignment operator are private.  This enables us to control the creation
	 * of the CAPClient object.
	 */
	static CAPClient* GetInstance()
	{
		if (instance_ == 0)
		{
			instance_ = new CAPClient();
		}
		return instance_;
	}
	
	/**
	 * Destructor deletes modeller_
	 */
	~CAPClient()
	{
		dbg("CAPClient::~CAPClient()");
		delete modeller_;
		gui_->Destroy();
		delete gui_;
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
	 * Gets the number of heart model frames.
	 *
	 * @return	The number of heart model frames.
	 */
	int GetNumberOfHeartModelFrames() const;

	/**
	 * Process the data points entered for current mode.
	 *
	 * @return	true if it succeeds, false if it fails.
	 */
	bool ProcessDataPointsEnteredForCurrentMode()
	{
		bool success = false;
		if (modeller_->OnAccept())
		{
			ModellingEnum mode = modeller_->GetCurrentMode();
			//== std::cout << "Current Mode = " << ModeStrings[mode] << '\n'; 
			gui_->UpdateModeSelectionUI(mode);
			if (mode == GUIDEPOINT)
			{
				if (!gui_->IsInitialisedHeartModel())
				{
					InitializeHeartModelTemplate();
					modeller_->AlignModel();
					modeller_->UpdateTimeVaryingModel();
					SmoothAlongTime();
					//EnterModelLoadedState();
				}
				UpdateMII();
			}
			success = true;
		}

		return success;
	}

	/**
	 * Change modelling mode.
	 *
	 * @param	mode	The mode.
	 */
	void ChangeModellingMode(ModellingEnum mode)
	{
		modeller_->ChangeMode(mode);
		gui_->UpdateModeSelectionUI(mode);
	}

	/**
	 * Sets a template to patient transformation.  This function implements the pure virtual
	 * function from the IModeller interface.
	 *
	 * @param	m	The transform.
	 */
	void SetHeartModelTransformation(const gtMatrix& m);

	/**
	 * Sets the heart model focal length.
	 *
	 * @param	focalLength	focal length for prolate spheriodal coordinate system.
	 */
	void SetHeartModelFocalLength(double focalLength);

	/**
	 * Sets the heart model mu from the base plane at the given time.
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
	 * Calculates the heart model xi for the given position and time and returns the element id of
	 * the nearest element to the given point.
	 *
	 * @param	position  	The position.
	 * @param	time	  	The time.
	 * @param [in,out]	xi	The xi.
	 *
	 * @return	The element id, -1 on failure.
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
	 * Using the ImageBrowser class open some images.
	 */
	void OpenImages();

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
	 * Adds a modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void AddModellingPoint(Cmiss_region_id region, int node_id, Point3D const& position, double time)
	{
		modeller_->AddModellingPoint(region, node_id, position, time);
	}

	/**
	 * Move modelling point.
	 *
	 * @param	region  	The region.
	 * @param	node_id 	Identifier for the node.
	 * @param	position	The position.
	 * @param	time		The time.
	 */
	void MoveModellingPoint(Cmiss_region_id region, int node_id, Point3D const& position, double time)
	{
		modeller_->MoveModellingPoint(region, node_id, position, time);
	}

	/**
	 * Move data point.
	 *
	 * @param [in,out]	dataPointID	If non-null, identifier for the data point.
	 * @param	newPosition		   	The new position.
	 * @param	time			   	The time.
	 */
	//void MoveDataPoint(Cmiss_node* dataPointID, Point3D const& newPosition, double time)
	//{
	//	modeller_->MoveDataPoint(dataPointID, newPosition, time);
	//}

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
		
		dbg("ED Volume(EPI) = " + ToString(gui_->ComputeHeartVolume(EPICARDIUM)));
		dbg("ED Volume(ENDO) = " + ToString(gui_->ComputeHeartVolume(ENDOCARDIUM)));
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

	/**
	 * Sets an image location.
	 *
	 * @param	location	The location.
	 */
	void SetImageLocation(const std::string& location)
	{
		previousImageLocation_ = location;
	}

	void ResetModel()
	{
		RemoveModeller();
		gui_->RemoveHeartModel();
		modeller_ = new Modeller(this); // initialise modeller and all the data points
	}

private:
	
	/**
	 * Initialise the window
	 */
	void Initialize();

	/**
	 * Deletes the modeller.
	 */
	void RemoveModeller()
	{
		if (modeller_)
		{
			delete modeller_;
			gui_->UpdateModeSelectionUI(APEX);
			modeller_ = 0;
		}
	}

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
	 * Initializes the heart model from the template.  If no images have 
	 * been loaded then this function will do nothing.
	 */
	void InitializeHeartModelTemplate();

	/**
	 * Private default constructor - This class should be instantiated from the static factory method.
	 */
	CAPClient()
		: gui_(0)
		, labelledSlices_(LabelledSlices())
		, modeller_(0)
		, cardiacAnnotationPtr_(0)
		, previousImageLocation_("")
	{
	}
	
	static CAPClient* instance_;	/**< The only instance of the CAPClient */
	CAPClientWindow* gui_;  /**< The graphical user interface */
	
	LabelledSlices labelledSlices_; /**< The labelled slices */
	
	Modeller* modeller_; /**< The modeller */

	boost::scoped_ptr<CardiacAnnotation> cardiacAnnotationPtr_; /**< The cardiac annotation pointer */

	Point3D previousPosition_;  /**< The previous position */
	std::string previousImageLocation_; /**< The previous image location */
};

} // namespace cap

#endif /* MAINAPP_H_ */
