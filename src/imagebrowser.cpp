
#include "imagebrowser.h"
#include "labelledslice.h"
#include "utils/debug.h"
#include "utils/misc.h"
#include "logmsg.h"

#include <wx/wxprec.h>
#include <wx/listctrl.h>
#ifndef WX_PRECOMP
# include <wx/wx.h>
#endif
#include <wx/xrc/xmlres.h>

extern "C"
{
#include <zn/cmiss_region.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_image.h>
}

#include <boost/bind.hpp>

namespace cap
{

bool ImageBrowser::wxXmlInitialised_ = false;

ImageBrowser::ImageBrowser(std::string const& workingLocation, const CardiacAnnotation& annotation, IImageBrowser *client)
	: gui_(0)
	, previousWorkingLocation_(workingLocation)
	, sortingMode_(SERIES_NUMBER)
	, client_(client)
	, frameNumberCurrentlyOnDisplay_(0)
	, textureTable_()
	, cardiacAnnotation_(annotation)
{
}

ImageBrowser::~ImageBrowser()
{
	ClearTextureMap();
	ClearTextureTable();
	dicomFileTable_.clear();
	client_ = 0;
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

void ImageBrowser::ChooseImageDirectory()
{
	std::string path = previousWorkingLocation_;
	if (path.empty() || !IsDirectory(path))
		path = wxGetCwd();
	
	const wxString& dirname = wxDirSelector(wxT("Choose the folder that contains the images to load"), path);
	if (!dirname.empty())
	{
		std::string restorePathOnFail = previousWorkingLocation_;
		previousWorkingLocation_ = dirname.mb_str();
		LOG_MSG(LOGINFORMATION) << "Loading images from '" << previousWorkingLocation_ << "'";
		if (!LoadImages())
			previousWorkingLocation_ = restorePathOnFail;
	}
}

void ImageBrowser::ChooseArchiveFile()
{
	dbg("ImageBrowser::ChooseArchiveFile()");
}

void ImageBrowser::ChooseAnnotationFile()
{
	if (previousWorkingLocation_.length() == 0)
		previousWorkingLocation_ = wxGetCwd();

	wxString defaultFilename = wxT("");
	wxString defaultExtension = wxT("xml");
	wxString wildcard = wxT("");
	int flags = wxOPEN;
	
	wxString filename = wxFileSelector(wxT("Choose an annotation file to open"),
			wxT(previousWorkingLocation_.c_str()), defaultFilename, defaultExtension, wildcard, flags);
	if (!filename.empty())
	{
		// work with the file
		LOG_MSG(LOGINFORMATION) << "Opening Annotation '" << filename <<"'";
		AnnotationFile annotationFile(filename.mb_str());
		annotationFile.ReadFile();
		if (annotationFile.GetCardiacAnnotation().imageAnnotations.size() > 0)
		{
			SetAnnotation(annotationFile.GetCardiacAnnotation());
			previousWorkingLocation_ = GetPath(filename.mb_str());
		}
		else
		{
			LOG_MSG(LOGWARNING) << "Invalid Annotation File - no image annotations found";
		}
	}
}

bool ImageBrowser::LoadImages()
{
	bool success = false;
	CreateTexturesFromDICOMFiles();
	if (dicomFileTable_.empty())
	{
		LOG_MSG(LOGWARNING) << "No valid DICOM files were found at this location '" << previousWorkingLocation_ << "'";
	}
	else
	{
		SortDICOMFiles();
		PopulateImageTable();
		success = true;
	}

	return success;
}

void ImageBrowser::Initialize()
{
	assert(gui_);
	if (!previousWorkingLocation_.empty())
	{
		LoadImages();
	}
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

void ImageBrowser::ClearTextureMap()
{
	TextureMap::iterator it = textureMap_.begin();
	for(; it != textureMap_.end(); ++it)
	{
		std::vector<Cmiss_field_image_id>::iterator it2 = it->second.begin();
		for (;it2 != it->second.end(); ++it2)
		{
			Cmiss_field_image_destroy(&(*it2));
		}
	}
	textureMap_.clear();
}

void ImageBrowser::ClearTextureTable()
{
	BOOST_FOREACH(TextureTable::value_type& value, textureTable_)
	{
		Cmiss_field_image_id tex = value.second;
		Cmiss_field_image_destroy(&tex);
	}
	textureTable_.clear();
}

void ImageBrowser::CreateTexturesFromDICOMFiles()
{
	using namespace std;
	
	// load some images and display
	const std::string& dirname = previousWorkingLocation_;
	std::vector<std::string> const& filenames = GetAllFileNamesRecursive(dirname);
	gui_->CreateProgressDialog("Please wait", "Loading DICOM images", filenames.size());
	
	std::map<std::string, bool> caseMap;
	std::vector<std::string> caseList;
	dicomFileTable_.clear();
	ClearTextureTable();
	caseListMap_.clear();
	std::vector<std::string>::const_iterator cit = filenames.begin();
	for(int count = 0; cit != filenames.end(); ++cit, count++)
	{
		std::string filename = *cit;
		// Skip known non-dicom image extensions
		if (EndsWith(filename, ".exnode") ||
			EndsWith(filename, ".exelem") ||
			EndsWith(filename, ".xml"))
		{
			continue;
		}
		std::string fullpath = dirname + "/" + filename;
		// Returns an accessed image field
		Cmiss_field_image_id image_field = gui_->CreateFieldImage(fullpath);
		if (image_field != 0)
		{
			DICOMPtr dicomFile(new DICOMImage(fullpath));
			if (dicomFile->Analyze(image_field))
			{
				std::map<std::string, bool>::const_iterator cit = caseMap.find(dicomFile->GetStudyInstanceUID());
				if (cit == caseMap.end())
				{
					caseMap[dicomFile->GetStudyInstanceUID()] = true;
					std::string caseString = "Case " + ToString(caseMap.size()) + " " + dicomFile->GetScanDate();
					caseList.push_back(caseString);
					caseListMap_[caseString] = dicomFile->GetStudyInstanceUID();
				}

				textureTable_.insert(std::make_pair(fullpath, image_field));
				dicomFileTable_.insert(std::make_pair(fullpath, dicomFile));
			}
			else
			{
				Cmiss_field_image_destroy(&image_field);
			}
		}

		if (!(count % 5))
		{
			gui_->UpdateProgressDialog(count);
		}
	}

	gui_->UpdateProgressDialog(filenames.size()-1);
	gui_->DestroyProgressDialog();
	gui_->SetCaseList(caseList);
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
	
	if (!sliceMap_.empty())
	{
		DICOMPtr const& firstImage = sliceMap_.begin()->second[0];
		UpdatePatientInfoPanel(firstImage);
		UpdateSeriesInfoPanel(firstImage);
	}
	
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
	ClearTextureMap();
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
			// The returned field does not increase the access count for the image field
			Cmiss_field_id temp_field = Cmiss_field_image_base_cast(field_image);
			// Add an access for the texture map
			Cmiss_field_access(temp_field);
		}
		textureMap_.insert(make_pair(slice.first, field_images));
	}
}

void ImageBrowser::ChangeImageAnnotation(int image_index)
{
	gui_->ClearAnnotationTable();
	gui_->CreateAnnotationTableColumns();
	
	std::string const& sopiuid = sliceMap_[sliceKeyCurrentlyOnDisplay_].at(image_index)->GetSopInstanceUID();
	
	std::vector<ImageAnnotation>::const_iterator itr =
		std::find_if(cardiacAnnotation_.imageAnnotations.begin(),
			cardiacAnnotation_.imageAnnotations.end(),
			boost::bind(&ImageAnnotation::sopiuid,_1) == sopiuid);
	
	
	if (itr != cardiacAnnotation_.imageAnnotations.end())
	{
		int rowNumber = 0;
		BOOST_FOREACH(Label const& label, itr->labels)
		{
			gui_->PopulateAnnotationTableRow(rowNumber++, label.label, label.rid, label.scope);
		}
		
		BOOST_FOREACH(ROI const& roi, itr->rOIs)
		{
			dbg("ROI:");
			BOOST_FOREACH(Label const& label, roi.labels)
			{
				dbg("    " + label.label);
			}
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
	std::string filename = GetFileName(dicomImages.at(frameNumber)->GetFilename());
	gui_->SetAnnotationString("File name : " + filename);
	double radius = std::max(width, height) / 2.0;
	gui_->FitSceneViewer(radius);
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
	if (gui_)
	{
		gui_->Destroy();
		delete gui_;
		gui_ = 0;
	}

	delete this;
}

void ImageBrowser::OnOKButtonClicked()
{
	LabelledSlices labelledSlices;
	
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
		LabelledSlice labelledSlice(sliceName, sliceMap_[key]);
		labelledSlices.push_back(labelledSlice);
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
	
	std::sort(labelledSlices.begin(), labelledSlices.end(), LabelledSortOrder());

	ClearTextureMap();
	ClearTextureTable();

	gui_->Destroy();
	delete gui_;
	gui_ = 0;
	
	client_->ResetModel();
	client_->LoadLabelledImages(labelledSlices);
	client_->LoadCardiacAnnotations(cardiacAnnotation_);
	client_->SetImageLocation(previousWorkingLocation_);

	delete this;
}

void ImageBrowser::OnNoneButtonEvent()
{
	// update CardiacAnnotation
	std::vector<std::string> labelsToRemove;
	labelsToRemove.push_back("Short Axis");
	labelsToRemove.push_back("Long Axis");
	labelsToRemove.push_back("Horizonal Long Axis");
	labelsToRemove.push_back("Vertical Long Axis");
	labelsToRemove.push_back("Cine Loop");
	
	std::vector<Label> labelsToAdd;
	
	UpdateAnnotationOnImageCurrentlyOnDisplay(labelsToRemove, labelsToAdd);

	gui_->PutLabelOnSelectedSlice("");
}

void ImageBrowser::OnLongAxisButtonEvent()
{
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

	gui_->PutLabelOnSelectedSlice("Long Axis");
}

void ImageBrowser::OnShortAxisButtonEvent()
{
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
	
	gui_->PutLabelOnSelectedSlice("Short Axis");
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
		std::map<std::string, long int>::const_iterator itr = uidToSliceKeyMap.find(sopiuid);
		if (itr != uidToSliceKeyMap.end())
		{
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
					LOG_MSG(LOGDEBUG) << "uid: " << sopiuid << ", label = " << imageLabel;
					std::map<long int, std::string>::const_iterator itr = slicePtrToLabelMap.find(sliceKeyPtr);
					
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
		else
		{
			LOG_MSG(LOGWARNING) << "Can't find the image with sopiuid: " << sopiuid;
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