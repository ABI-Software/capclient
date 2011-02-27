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

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>

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

template <typename ImageBrowseWindow>
class ImageBrowser
{
public:
	void SetImageBrowseWindow(ImageBrowseWindow* gui)
	{
		gui_ = gui;
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
		std::vector<Cmiss_texture_id> const& textures = textureMap_[key];
		texturesCurrentlyOnDisplay_ = &textures;
		
		// Update the gui
		UpdateImageInfoPanel(images[0]); // should rename to Series Info?
		
		// Update image preview panel
		gui_->SetAnimationSliderMax(images.size());
		
		// Resize the rectangular cmgui model
		// according to the new dimensions
		size_t width = images[0]->GetImageWidth();
		size_t height = images[0]->GetImageHeight();
		gui_->ResizePreviewImage(width, height);
		
		gui_->DisplayImage(textures[0]);

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
	
//	void CreateTexturesFromDICOMFiles()
//	{
//		using namespace std;
//		
//		// load some images and display
////		ProgressDialog progressDlg("Please wait", "Loading DICOM images",
////			numberOfDICOMFiles_, this);
//		gui_->CreateProgressDialog("Please wait", "Loading DICOM images", numberOfDICOMFiles_);
//		
//		int count = 0;
//		BOOST_FOREACH(DICOMTable::value_type const& value, dicomFileTable_)
//		{	
//			const string& filename = value.first;	
////			Cmiss_texture_id texture_id = cmguiManager_.LoadCmissTexture(filename);
//			textureTable_.insert(make_pair(filename, texture_id));
//			
//			count++;
//			if (!(count % 20))
//			{
////				progressDlg.Update(count);
//				gui_->UpdateProgressDialog(count);
//			}
//		}
//		return;
//	}
	
	void OnAnimationSliderEvent(int value)
	{
		int textureIndex = value - 1; // tex index is 0 based while slider value is 1 based
		
		gui_->DisplayImage((*texturesCurrentlyOnDisplay_)[value-1]);
		
		// force redraw while silder is manipulated
		gui_->RefreshPreviewPanel();
		return;
	}

	void OnShortAxisButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("Short Axis");
		
		// TODO update CardiacAnnotation
	}

	void OnLongAxisButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("Long Axis");
		
		// TODO update CardiacAnnotation
	}

	void OnNoneButtonEvent()
	{
	//	std::cout << __func__ << '\n';
		gui_->PutLabelOnSelectedSlice("");
		
		// TODO update CardiacAnnotation
	}

//	void ImageBrowseWindow::OnOKButtonEvent(wxCommandEvent& event)
//	{
//		std::cout << __func__ << '\n';
//		// construct the data structure of type SlicesWithImages to pass to the main window
//		SlicesWithImages slices;
//		int shortAxisCount = 1;
//		int longAxisCount = 1;
//		long index = imageTable_->GetItemCount() - 1;
//		while (index >= 0) // iterate from the bottom of the list ( to be compatible with CIM's setup)
//		{
//			std::string label = GetCellContentsString(index, LABEL_COLUMN_INDEX);
//			std::cout << "index = " << index << ", label = " << label << '\n';
//			if (label.length())
//			{
//				long ptr = imageTable_->GetItemData(index);
//				SliceMap::value_type* const sliceValuePtr = reinterpret_cast<SliceMap::value_type* const>(ptr);
//				SliceKeyType const& key = sliceValuePtr->first;
//				std::string sliceName;
//				if (label == "Short Axis")
//				{
//					sliceName = "SA" + boost::lexical_cast<std::string>(shortAxisCount++);
//				}
//				else // (label == "Long Axis"
//				{
//					sliceName = "LA" + boost::lexical_cast<std::string>(longAxisCount++);
//				}
//				SliceInfo sliceInfo(sliceName, sliceMap_[key], textureMap_[key]);
//				slices.push_back(sliceInfo);
//			}
//			index--;
//		}
//		
//		if (longAxisCount >= 10)
//		{
//			std::cout << "TOO MANY LONG AXES\n";
//			wxMessageBox("Too many long axes slices", "Invalid selection",
//					wxOK, this);
//			return;
//		}
//		if (shortAxisCount >= 30)
//		{
//			std::cout << "TOO MANY SHORT AXES\n";
//			wxMessageBox("Too many short axes slices", "Invalid selection",
//					wxOK, this);
//			return;
//		}
//		
//		std::sort(slices.begin(), slices.end(), SliceInfoSortOrder());
//
//		std::cout << __func__ << " : slices.size() = " << slices.size() <<  '\n';
//		if (slices.empty())
//		{
//			std::cout << "Empty image set.\n";
//			return;
//		}
//		
//		client_.LoadImagesFromImageBrowseWindow(slices);
//		Close();
//	}

//	void ImageBrowseWindow::OnCancelButtonEvent(wxCommandEvent& event)
//	{
//		//TODO Cleanup textures - REVISE design
//		// Should probably use reference counted smart pointer for Cmiss_texture
//		// Since the ownership is shared between ImageSlice and this (ImageBrowseWindow)
//		
//		BOOST_FOREACH(TextureTable::value_type& value, textureTable_)
//		{
//			Cmiss_texture_id tex = value.second;
//			DESTROY(Texture)(&tex);
//		}
//		Close();
//	}

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
	
private:
	
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
	std::vector<Cmiss_texture_id> const* texturesCurrentlyOnDisplay_;
	
	DICOMTable dicomFileTable_; // unsorted list of all dicom files
	TextureTable textureTable_; // unsorted list of all textures
	
	std::string archiveFilename_;
	size_t numberOfDICOMFiles_;
};

} //namespace cap

#endif /* IMAGEBROWSER_H_ */
