/*
 * ImageBrowser.h
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#ifndef IMAGEBROWSER_H_
#define IMAGEBROWSER_H_

#include "CAPMath.h"
#include "DICOMImage.h"
#include "FileSystem.h"
#include "SliceInfo.h"
#include "ImageBrowseWindowClient.h"
#include "CmguiManager.h"
#include "CAPAnnotationFile.h"

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <iostream>

struct Cmiss_texture;
typedef struct Cmiss_texture* Cmiss_texture_id;

namespace cap
{

typedef std::pair<int, double> SliceKeyType;
//	typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;
typedef std::map<SliceKeyType, std::vector<DICOMPtr> > SliceMap;
typedef std::map<SliceKeyType, std::vector<Cmiss_texture_id> > TextureMap;
typedef std::map<std::string, DICOMPtr> DICOMTable;
typedef std::map<std::string, Cmiss_texture_id> TextureTable;

template <typename ImageBrowseWindow , typename CmguiManager>
class ImageBrowser
{
public:
	
	void SetImageBrowseWindow(ImageBrowseWindow* gui)
	{
		gui_ = gui;
	}
	
	void Initialize() // Should be called after the call to SetImageBrowseWindow
	{
		assert(gui_);
		
		gui_->LoadWindowLayout();
		gui_->CreatePreviewPanel();
		
		ReadInDICOMFiles(); // This reads the dicom header and populates dicomFileTable_
		if (dicomFileTable_.empty())
		{
			std::cout << "No valid DICOM files were found here\n";
		}
		else
		{
			CreateTexturesFromDICOMFiles();
			PopulateImageTable();
		}
		
		gui_->FitWindow();
	}
	
	void UpdatePatientInfoPanel(DICOMPtr const& image)
	{
		using std::string;
		string const& name = image->GetPatientName();
		gui_->SetInfoField("PatientName", name);
		string const& id = image->GetPatientID();
		gui_->SetInfoField("PatientID", id);
		string const& scanDate = image->GetScanDate();
		gui_->SetInfoField("ScanDate", scanDate);
		string const& dob = image->GetDateOfBirth();
		gui_->SetInfoField("DateOfBirth", dob);
		string const& gender = image->GetGender();
		string const& age = image->GetAge();
		gui_->SetInfoField("GenderAndAge", gender + " " + age);
	}
	
	void SwitchSliceToDisplay(SliceMap::value_type const& slice)
	{
	//	std::cout << __func__ << '\n';
		SliceKeyType const& key = slice.first;
		std::vector<DICOMPtr> const& images = slice.second;
		assert(textureMap_.find(key) != textureMap_.end());
		sliceKeyCurrentlyOnDisplay_ = key;
		
		// Update the gui
		UpdateImageInfoPanel(images[0]); // should rename to Series Info?
		
		// Update image preview panel
		gui_->SetAnimationSliderMax(images.size());
		
		// Resize the rectangular cmgui model
		// according to the new dimensions
		size_t width = images[0]->GetImageWidth();
		size_t height = images[0]->GetImageHeight();
		gui_->ResizePreviewImage(width, height);
		
		DisplayImageByFrameNumber(0);

		double radius = std::max(width, height) / 2.0;
		gui_->FitSceneViewer(radius);
		gui_->RefreshPreviewPanel();
	}
	
	void UpdateImageInfoPanel(DICOMPtr const& dicomPtr)
	{
		using std::string;
		using boost::lexical_cast;
		
		size_t width = dicomPtr->GetImageWidth();
		size_t height = dicomPtr->GetImageHeight();
		string size = lexical_cast<string>(width) + " x " + lexical_cast<string>(height);
		gui_->SetInfoField("ImageSize", size);
		
		Point3D position = dicomPtr->GetImagePosition();
		std::stringstream ss;
		ss << std::setprecision(5) << position.x << " ";
		ss << std::setprecision(5) << position.y << " ";
		ss << std::setprecision(5) << position.z;
		string const &posStr(ss.str());
		gui_->SetInfoField("ImagePosition", posStr);
		
		std::pair<Vector3D,Vector3D> const& orientation = dicomPtr->GetImageOrientation();

//		std::cout << "ori 1: " << orientation.first.x << " " << orientation.first.y << " " << orientation.first.z << "\n";
//		std::cout << "ori 2: " << orientation.second.x << " " << orientation.second.y << " " << orientation.second.z << "\n";
	}
	
	void SortDICOMFiles()
	{	
		sliceMap_.clear();
		BOOST_FOREACH(DICOMTable::value_type const& value, dicomFileTable_)
		{
			DICOMPtr const &dicomFile = value.second;
			int seriesNum = dicomFile->GetSeriesNumber();
	//		std::cout << "Series Num = " << seriesNum << '\n';
			double distanceFromOrigin;
			if (sortingMode_ == SERIES_NUMBER)
			{
				// Don't use the position for sorting : set it to 0 for all images
				distanceFromOrigin = 0.0;
			}
			else if (sortingMode_ == SERIES_NUMBER_AND_IMAGE_POSITION)
			{
				// Use the dot product of the position and the normal vector
				// as the measure of the position
				Vector3D pos = dicomFile->GetImagePosition() - Point3D(0,0,0);
				std::pair<Vector3D,Vector3D> oris = dicomFile->GetImageOrientation();
				Vector3D normal = CrossProduct(oris.first, oris.second);
				normal.Normalise();
				distanceFromOrigin = - DotProduct(pos, normal);
			}
			
			SliceKeyType key = std::make_pair(seriesNum, distanceFromOrigin);
			SliceMap::iterator itr = sliceMap_.find(key);
			if (itr != sliceMap_.end())
			{
				itr->second.push_back(dicomFile);
			}
			else
			{
				std::vector<DICOMPtr> v(1, dicomFile);
				sliceMap_.insert(std::make_pair(key, v));
			}
		}
		
		// Sort the dicom images in a slice/series by the instance Number
		BOOST_FOREACH(SliceMap::value_type& slice, sliceMap_)
		{
			std::vector<DICOMPtr>& images = slice.second;
			// use stable_sort to preserve the order of images in case the instance number element is missing
			// (in which case the GetInstanceNumber returns -1)
			std::stable_sort(images.begin(), images.end(), 
					boost::bind(std::less<int>(),
							boost::bind(&DICOMImage::GetInstanceNumber, _1),
							boost::bind(&DICOMImage::GetInstanceNumber, _2)));
		}
	}
	
	void ConstructTextureMap()
	{
		using namespace std;
		
		textureMap_.clear();
		BOOST_FOREACH(SliceMap::value_type& value, sliceMap_)
		{	
			vector<DICOMPtr>& images = value.second;
			vector<DICOMPtr>::const_iterator itr = images.begin();
			vector<DICOMPtr>::const_iterator end = images.end();
			vector<Cmiss_texture_id> textures;
			
			for (; itr != end; ++itr)
			{
				const string& filename = (*itr)->GetFilename();	
				Cmiss_texture_id texture_id = textureTable_[filename];
				textures.push_back(texture_id);
			}
			
			textureMap_.insert(make_pair(value.first, textures));
		}
		return;
	}
	
	void PopulateImageTable()
	{	
		SortDICOMFiles();
		ConstructTextureMap();
		
		gui_->ClearImageTable();
		gui_->CreateImageTableColumns();
		
		int rowNumber = 0;
		BOOST_FOREACH(SliceMap::value_type& value, sliceMap_)
		{
			using boost::lexical_cast;
			using std::string;
			
			std::vector<DICOMPtr>& images = value.second;
			DICOMPtr image = images[0];
			int seriesNumber(image->GetSeriesNumber());
			std::string const& seriesDescription(image->GetSeriesDescription());
			std::string const& sequenceName(image->GetSequenceName());
			size_t numImages = images.size();
			long int userDataPtr = reinterpret_cast<long int>(&value);
			
			gui_->PopulateImageTableRow(rowNumber,
					seriesNumber, seriesDescription,
					sequenceName, numImages,
					userDataPtr);
			
			rowNumber++;
		}
		
		// Set the selection to the first item in the list
		gui_->SelectFirstRowInImageTable();
		
		DICOMPtr const& firstImage = sliceMap_.begin()->second[0];
		UpdatePatientInfoPanel(firstImage);
		
		if (!cardiacAnnotation_.imageAnnotations.empty())
		{
			UpdateImageTableLabelsAccordingToCardiacAnnotation();
		}
	}
	
	void ReadInDICOMFiles()
	{
		//	std::string dirname = TEST_DIR;
		// TODO unzip archive file
		// for now we assume the archive has already been unzipped
		// and archiveFilename points to the containing dir
		std::string const& dirname = archiveFilename_;
		
		FileSystem fs(dirname);
		std::vector<std::string> const& filenames = fs.getAllFileNames();
		
		std::cout << "num files = " << filenames.size() << '\n';
		numberOfDICOMFiles_ = filenames.size();
		
		gui_->CreateProgressDialog("Please wait", "Analysing DICOM headers", numberOfDICOMFiles_);
		int count = 0;
		BOOST_FOREACH(std::string const& filename, filenames)
		{
	//		std::cout << filename <<'\n';
			std::string fullpath = dirname + "/" + filename;
	//		std::cout << fullpath <<'\n';
			try
			{
				DICOMPtr dicomFile(new DICOMImage(fullpath));
				dicomFileTable_.insert(std::make_pair(fullpath, dicomFile));
			}
			catch (std::exception& e)
			{
				// This is not a DICOM file
				std::cout << "Invalid DICOM file : " << filename << '\n';
			}
			
			count++;
			if (!(count % 10))
			{
				gui_->UpdateProgressDialog(count);
			}
		}
		gui_->DestroyProgressDialog(); // REVISE interface 
	}
	
	void CreateTexturesFromDICOMFiles()
	{
		using namespace std;
		
		// load some images and display
		gui_->CreateProgressDialog("Please wait", "Loading DICOM images", numberOfDICOMFiles_);
		
		int count = 0;
		BOOST_FOREACH(DICOMTable::value_type const& value, dicomFileTable_)
		{	
			const string& filename = value.first;	
			Cmiss_texture_id texture_id = cmguiManager_.LoadCmissTexture(filename);
			textureTable_.insert(make_pair(filename, texture_id));
			
			count++;
			if (!(count % 20))
			{
				gui_->UpdateProgressDialog(count);
			}
		}
		
		gui_->DestroyProgressDialog();
		return;
	}
	
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
	
	void OnAnimationSliderEvent(int value)
	{	
//		std::cout << __func__ << " : value = " << value << '\n';
		DisplayImageByFrameNumber(value);
		
		// force redraw while silder is manipulated
		gui_->RefreshPreviewPanel();
		return;
	}
	
	void DisplayImageByFrameNumber(int frameNumber)
	{
		std::vector<Cmiss_texture_id> const& textures = textureMap_[sliceKeyCurrentlyOnDisplay_];
		frameNumberCurrentlyOnDisplay_ = frameNumber;
		gui_->DisplayImage(textures[frameNumberCurrentlyOnDisplay_]);
	}

	void OnShortAxisButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("Short Axis");
		
		// update CardiacAnnotation
//		std::vector<DICOMPtr> const& dicomPtrs = sliceMap_[sliceKeyCurrentlyOnDisplay_];
//		BOOST_FOREACH(DICOMPtr const& dicomPtr, dicomPtrs)
//		{
//			std::string const& sopiuid = dicomPtr->GetSopInstanceUID();
//			std::vector<ImageAnnotation>::iterator imageItr =
//					std::find_if(cardiacAnnotation_.imageAnnotations.begin(),
//								cardiacAnnotation_.imageAnnotations.end(),
//								boost::bind(&ImageAnnotation::sopiuid,_1) == sopiuid);
//			if (imageItr == cardiacAnnotation_.imageAnnotations.end())
//			{
//				ImageAnnotation imageAnno;
//				imageAnno.sopiuid = sopiuid;
//				cardiacAnnotation_.imageAnnotations.push_back(imageAnno);
//				imageItr = cardiacAnnotation_.imageAnnotations.end() - 1;
//			}
//			else
//			{
//				// Remove conflicting labels if present
//				std::vector<Label>::iterator labelItr =
//						imageItr->labels.begin();
//				while (labelItr != imageItr->labels.end())
//				{
//					if (labelItr->label == "Long Axis"
//						|| labelItr->label == "Horizonal Long Axis"
//						|| labelItr->label == "Vertial Long Axis"
//						|| labelItr->label == "Cine Loop")
//					{
////						std::cout << "Erasing label : " << labelItr->label << '\n';
//						imageItr->labels.erase(labelItr);
//					}
//					else
//					{
//						++labelItr;
//					}
//				}
//			}
////			std::cout << "Done erasing label\n";
//			Label cine_loop = {"RID:10928", "Series", "Cine Loop"};
//			imageItr->labels.push_back(cine_loop);
//			Label short_axis = {"RID:10577", "Slice", "Short Axis"};
//			imageItr->labels.push_back(short_axis);
//		}
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

	void OnNoneButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("");
		
		// update CardiacAnnotation
		std::vector<std::string> labelsToRemove;
		labelsToRemove.push_back("Short Axis");
		labelsToRemove.push_back("Long Axis");
		labelsToRemove.push_back("Horizonal Long Axis");
		labelsToRemove.push_back("Vertial Long Axis");
		labelsToRemove.push_back("Cine Loop");
		
		std::vector<Label> labelsToAdd;
		
		UpdateAnnotationOnImageCurrentlyOnDisplay(labelsToRemove, labelsToAdd);
	}

	void OnOKButtonEvent()
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
		
		client_.LoadImagesFromImageBrowseWindow(slices);
		gui_->Close();
	}

	void OnCancelButtonEvent()
	{
		//TODO Cleanup textures - REVISE design
		// Should probably use reference counted smart pointer for Cmiss_texture
		// Since the ownership is shared between ImageSlice and this (ImageBrowseWindow)
		
		BOOST_FOREACH(TextureTable::value_type& value, textureTable_)
		{
			Cmiss_texture_id tex = value.second;
			cmguiManager_.DestroyTexture(tex);
		}
//		Close();
	}

	void OnOrderByRadioBox(int event)
	{
	//	std::cout << __func__ << " event.GetInt() = " << event.GetInt() << '\n';
		if (event == 0)
		{
			sortingMode_ = SERIES_NUMBER;
		}
		else if (event == 1)
		{
			sortingMode_ = SERIES_NUMBER_AND_IMAGE_POSITION;
		}
		else
		{
			throw std::logic_error("Invalid sorting mode");
		}
		
		PopulateImageTable();
	}
	
	void SetAnnotation(CardiacAnnotation const& anno)
	{
		cardiacAnnotation_ = anno;
		PopulateImageTable();
	}
	
	void ShowImageAnnotationCurrentlyOnDisplay()
	{
		std::string const& sopiuid = 
				sliceMap_[sliceKeyCurrentlyOnDisplay_].at(frameNumberCurrentlyOnDisplay_)->GetSopInstanceUID();
		
		std::vector<ImageAnnotation>::const_iterator itr =
				std::find_if(cardiacAnnotation_.imageAnnotations.begin(),
						cardiacAnnotation_.imageAnnotations.end(),
						boost::bind(&ImageAnnotation::sopiuid,_1) == sopiuid);
		
		
		std::cout << "Image Annotation for sop: " << sopiuid << '\n';
		if (itr == cardiacAnnotation_.imageAnnotations.end())
		{
			std::cout << "No annotation\n";
			return;
		}
		
		BOOST_FOREACH(Label const& label, itr->labels)
		{
			std::cout << label.label << "\n";
		}
		BOOST_FOREACH(ROI const& roi, itr->rOIs)
		{
			std::cout << "ROI:\n";
			BOOST_FOREACH(Label const& label, roi.labels)
			{
				std::cout << label.label << "\n";
			}
		}
	}
	
	static
	ImageBrowser<ImageBrowseWindow, CmguiManager>*
	CreateImageBrowser(std::string const& archiveFilename, CmguiManager const& manager, ImageBrowseWindowClient& client)
	{
		ImageBrowser<ImageBrowseWindow, CmguiManager>* ib = new ImageBrowser<ImageBrowseWindow, CmguiManager>(archiveFilename, manager, client);
		ImageBrowseWindow* frame = new ImageBrowseWindow(*ib, manager);
		frame->Show(true);
		ib->SetImageBrowseWindow(frame);
		ib->Initialize();
		
		return ib;
	}
	
private:
	
	// Private constructor means this class must be created from the factory method
	ImageBrowser(std::string const& archiveFilename, CmguiManager const& manager, ImageBrowseWindowClient& client)
	:
		archiveFilename_(archiveFilename),
		cmguiManager_(manager),
		client_(client),
		sortingMode_(SERIES_NUMBER),
		frameNumberCurrentlyOnDisplay_(0)
	{
	}
	
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
	
	// Pointer to the gui layer. (wxWidgets)
	// Note this class does not own the gui_ object 
	// (gui's lifecycle is managed by wxWidets framework)
	ImageBrowseWindow* gui_; 
	
	enum SortingMode
	{
		SERIES_NUMBER,
		SERIES_NUMBER_AND_IMAGE_POSITION
	};
	
	SortingMode sortingMode_;
	
	SliceMap sliceMap_;
	TextureMap textureMap_; // this could be merged with sliceMap_
	SliceKeyType sliceKeyCurrentlyOnDisplay_;
	int frameNumberCurrentlyOnDisplay_;
	
	DICOMTable dicomFileTable_; // unsorted list of all dicom files
	TextureTable textureTable_; // unsorted list of all textures
	
	std::string archiveFilename_;
	size_t numberOfDICOMFiles_;
	
	CmguiManager const& cmguiManager_;
	ImageBrowseWindowClient& client_;
	CardiacAnnotation cardiacAnnotation_;
	
	const static std::string aa;
};


} //namespace cap

#endif /* IMAGEBROWSER_H_ */
