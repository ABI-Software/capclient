
#include <wx/wxprec.h>
#include <wx/listctrl.h>
#ifndef WX_PRECOMP
# include <wx/wx.h>
#endif
#include <wx/xrc/xmlres.h>

extern "C"
{
#include <api/cmiss_region.h>
#include <api/cmiss_field.h>
#include <api/cmiss_field_image.h>
}

#include "labelledslice.h"
#include "imagebrowser.h"
#include "labelledtexture.h"
#include "utils/debug.h"

namespace cap
{

bool ImageBrowser::wxXmlInitialised_ = false;

ImageBrowser::ImageBrowser(std::string const& archiveFilename, IImageBrowser *client)
	: gui_(0)
	, sortingMode_(SERIES_NUMBER)
	, client_(client)
	, frameNumberCurrentlyOnDisplay_(0)
	, archiveFilename_(archiveFilename)
{
}

ImageBrowser::~ImageBrowser()
{
	dbg("ImageBrowser::~ImageBrowser()");
	BOOST_FOREACH(TextureTable::value_type& value, textureTable_)
	{
		Cmiss_field_image_id tex = value.second;
		Cmiss_field_image_destroy(&tex); /** TODO: fix undefined reference */
	}
}

void ImageBrowser::UpdatePatientInfoPanel(DICOMPtr const& image)
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

void ImageBrowser::SwitchSliceToDisplay(SliceMap::value_type const& slice)
{
	//	std::cout << __func__ << '\n';
	const SliceKeyType& key = slice.first;
	const std::vector<DICOMPtr>& images = slice.second;
	assert(textureMap_.find(key) != textureMap_.end());
	sliceKeyCurrentlyOnDisplay_ = key;
	
	// Update the gui
	UpdateSeriesInfoPanel(images[0]);
	
	// Update image preview panel
	gui_->SetAnimationSliderMax(images.size());
	ChangePreviewImage(0);
}

void ImageBrowser::Initialize()
{
	assert(gui_);
	
	ReadInDICOMFiles();
	if (dicomFileTable_.empty())
	{
		std::cout << "No valid DICOM files were found here" << std::endl;
	}
	else
	{
		SortDICOMFiles();
		CreateTexturesFromDICOMFiles();
		PopulateImageTable();
	}
}

void ImageBrowser::ReadInDICOMFiles()
{
	//	std::string dirname = TEST_DIR;
	// TODO unzip archive file
	// for now we assume the archive has already been unzipped
	// and archiveFilename points to the containing dir
	std::string const& dirname = archiveFilename_;
	std::vector<std::string> const& filenames = FileSystem::GetAllFileNames(dirname);
	
	gui_->CreateProgressDialog("Please wait", "Analysing DICOM headers", filenames.size());
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
		catch (std::exception&)
		{
			// This is not a DICOM file
			std::cout << "Invalid DICOM file : " << filename << std::endl;
		}
		
		count++;
		if (!(count % 10))
		{
			gui_->UpdateProgressDialog(count);
		}
	}
	gui_->DestroyProgressDialog(); // REVISE interface 
}

size_t ImageBrowser::GetSliceMapImageCount() const
{
	size_t count = 0;
	BOOST_FOREACH(const SliceMap::value_type& slice, sliceMap_)
	{
		count += slice.second.size();
	}
	
	return count;
}

void ImageBrowser::CreateTexturesFromDICOMFiles()
{
	using namespace std;
	
	// load some images and display
	gui_->CreateProgressDialog("Please wait", "Loading DICOM images", GetSliceMapImageCount());
	
	textureTable_.clear();
	textureMap_.clear();
	int count = 0;
	BOOST_FOREACH(SliceMap::value_type& slice, sliceMap_)
	{
		std::vector<Cmiss_field_image_id> image_field_stack;
		BOOST_FOREACH(DICOMPtr const& dicomPtr, slice.second)
		{
			Cmiss_field_image_id image_field = gui_->CreateFieldImage(dicomPtr);
			image_field_stack.push_back(image_field);
			textureTable_.insert(make_pair(dicomPtr->GetFilename(), image_field));
			count++;
			if (!(count % 5))
			{
				gui_->UpdateProgressDialog(count);
			}
		}
		textureMap_.insert(make_pair(slice.first, image_field_stack));
	}
	
	gui_->DestroyProgressDialog();
	return;
}

void ImageBrowser::PopulateImageTable()
{
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
	UpdateSeriesInfoPanel(firstImage);
	
	if (!cardiacAnnotation_.imageAnnotations.empty())
	{
		UpdateImageTableLabelsAccordingToCardiacAnnotation();
	}
}

void ImageBrowser::SortDICOMFiles()
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
	textureMap_.clear();
	BOOST_FOREACH(SliceMap::value_type& slice, sliceMap_)
	{
		std::vector<DICOMPtr>& images = slice.second;
		// use stable_sort to preserve the order of images in case the instance number element is missing
		// (in which case the GetInstanceNumber returns -1)
		std::stable_sort(images.begin(), images.end(), 
						 boost::bind(std::less<int>(),
									 boost::bind(&DICOMImage::GetInstanceNumber, _1),
									 boost::bind(&DICOMImage::GetInstanceNumber, _2)));
		
		std::vector<DICOMPtr>::const_iterator itr = images.begin();
		std::vector<Cmiss_field_image_id> field_images;
		for( ; itr != images.end(); ++itr)
		{
			const std::string& filename = (*itr)->GetFilename();
			Cmiss_field_image_id field_image = textureTable_[filename];
			field_images.push_back(field_image);
		}
		textureMap_.insert(make_pair(slice.first, field_images));
	}
}

void ImageBrowser::ChangeImageAnnotation(int image_index)
{
	gui_->ClearAnnotationTable();
	gui_->CreateAnnotationTableColumns();
	
	std::string const& sopiuid = 
	sliceMap_[sliceKeyCurrentlyOnDisplay_].at(image_index)->GetSopInstanceUID();
	
	std::vector<ImageAnnotation>::const_iterator itr =
	std::find_if(cardiacAnnotation_.imageAnnotations.begin(),
				 cardiacAnnotation_.imageAnnotations.end(),
				 boost::bind(&ImageAnnotation::sopiuid,_1) == sopiuid);
	
	
	//std::cout << "Image Annotation for sop: " << sopiuid << '\n';
	if (itr == cardiacAnnotation_.imageAnnotations.end())
	{
		//std::cout << "No annotation\n";
		return;
	}
	
	int rowNumber = 0;
	BOOST_FOREACH(Label const& label, itr->labels)
	{
		//std::cout << label.label << "\n";
		gui_->PopulateAnnotationTableRow(rowNumber++, label.label, label.rid, label.scope);
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

void ImageBrowser::UpdateSeriesInfoPanel(DICOMPtr const& dicomPtr)
{
	using std::string;
	using boost::lexical_cast;
	
	size_t width = dicomPtr->GetImageWidthPx();
	size_t height = dicomPtr->GetImageHeightPx();
	string size = lexical_cast<string>(width) + " x " + lexical_cast<string>(height);
	gui_->SetInfoField("ImageSize", size);
	
	Point3D position = dicomPtr->GetImagePosition();
	std::stringstream ss;
	ss << std::setprecision(5) << position.x << " ";
	ss << std::setprecision(5) << position.y << " ";
	ss << std::setprecision(5) << position.z;
	string const &posStr(ss.str());
	gui_->SetInfoField("ImagePosition", posStr);
	
	// std::pair<Vector3D,Vector3D> const& orientation = dicomPtr->GetImageOrientation(); /**< Was this intended to be used somewhere? */
}

void ImageBrowser::ChangePreviewImage(int frameNumber)
{
	std::vector<DICOMPtr> dicomImages = sliceMap_[sliceKeyCurrentlyOnDisplay_];
	std::vector<Cmiss_field_image_id> images = textureMap_[sliceKeyCurrentlyOnDisplay_];
	gui_->ChangePreviewImage(images.at(frameNumber));
	double width = dicomImages.at(frameNumber)->GetImageWidthMm();
	double height = dicomImages.at(frameNumber)->GetImageHeightMm();
	gui_->ResizePreviewImage(width, height); // Resize the preview element 
	
	ChangeImageAnnotation(frameNumber);
	
	gui_->ViewAll();
	gui_->SetAnnotationString(dicomImages.at(frameNumber)->GetFilename());
	double radius = std::max(width, height) / 2.0;
	gui_->FitSceneViewer(radius);
	gui_->RefreshPreviewPanel();
}

void ImageBrowser::OnOrderByRadioBox(int event)
{
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
	SortDICOMFiles();
	PopulateImageTable();
}

void ImageBrowser::OnCancelButtonClicked()
{
	//TODO Cleanup textures - REVISE design
	// Should probably use reference counted smart pointer for Cmiss_texture
	// Since the ownership is shared between ImageSlice and this (ImageBrowserWindow)
	
	BOOST_FOREACH(TextureTable::value_type& value, textureTable_)
	{
		Cmiss_field_image_id tex = value.second;
		Cmiss_field_image_destroy(&tex);
	}
}

void ImageBrowser::OnOKButtonClicked()
{
	// construct the data structure of type SlicesWithImages to pass to the main window
	SlicesWithImages slices;
	LabelledSlices labelledSlices;
	//std::vector<LabelledTexture> labelledTextures;
	
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
		LabelledSlice labelledSlice(sliceName, sliceMap_[key]);
		//LabelledTexture labelledTexture(sliceName, textureMap_[key]);
		labelledSlices.push_back(labelledSlice);
		//labelledTextures.push_back(labelledTexture);
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
	std::sort(labelledSlices.begin(), labelledSlices.end(), LabelledSortOrder());
	//std::sort(labelledTextures.begin(), labelledTextures.end(), LabelledSortOrder());
	
	std::cout << __func__ << " : slices.size() = " << slices.size() <<  '\n';
	if (slices.empty())
	{
		std::cout << "Empty image set.\n";
		return;
	}
	
	//--client_->LoadImagesFromImageBrowserWindow(slices, cardiacAnnotation_);
	client_->LoadLabelledImages(labelledSlices);
	client_->LoadCardiacAnnotations(cardiacAnnotation_);
	gui_->Hide();
}

void ImageBrowser::OnNoneButtonEvent()
{
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

void ImageBrowser::OnLongAxisButtonEvent()
{
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

void ImageBrowser::OnShortAxisButtonEvent()
{
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

void ImageBrowser::OnImageTableItemSelected(long int userDataPtr)
{
	SliceMap::value_type* const sliceValuePtr = 
	reinterpret_cast<SliceMap::value_type* const>(userDataPtr);
	//	std::cout << "Series Num = " << (*sliceValuePtr).first.first << '\n';
	//	std::cout << "Distance to origin = " << (*sliceValuePtr).first.second << '\n';
	//	std::cout << "Image filename = " << (*sliceValuePtr).second[0]->GetFilename() << '\n';
	
	// Display the images from the selected row.
	SwitchSliceToDisplay(*sliceValuePtr);
}

void ImageBrowser::UpdateImageTableLabelsAccordingToCardiacAnnotation()
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

void ImageBrowser::UpdateAnnotationOnImageCurrentlyOnDisplay(std::vector<std::string> const& labelsToRemove, std::vector<Label> const& labelsToAdd)
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
			std::vector<Label>::iterator labelItr = imageItr->labels.begin();
			while (labelItr != imageItr->labels.end())
			{
				bool erasePerformed = false;
				BOOST_FOREACH(std::string const& labelToRemove, labelsToRemove)
				{
					if (labelItr != imageItr->labels.end() && labelItr->label == labelToRemove)
					{
						 // Does this invalidate the iterator? yes but it returns an iterator 
						 // to the next item.  But the next item could be the end so check for that.
						labelItr = imageItr->labels.erase(labelItr);
						erasePerformed = true;
					}
				}
				if (!erasePerformed)
				{
					++labelItr;
				}
			}
		}
		
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

}