/*
 * ImageBrowseWindow.cpp
 *
 *  Created on: Jun 17, 2010
 *      Author: jchu014
 */
#include "ImageBrowseWindow.h"

#include "Config.h"
#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "DICOMImage.h"
#include "CAPMaterial.h"
#include "ImageBrowseWindowClient.h"

#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/dir.h>
#include <wx/progdlg.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
//#include <boost/lambda/lambda.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <set>

extern "C"
{
#include "finite_element/finite_element.h"
#include "graphics/scene_viewer.h"
#include "graphics/scene.h"
}

namespace
{

int FitSceneViewer(Cmiss_scene_viewer_id scene_viewer, double radius)
{
	double centre_x, centre_y, centre_z, clip_factor, //radius,
		size_x, size_y, size_z, width_factor;
	int return_code;

	if (scene_viewer)
	{	
		Scene_get_graphics_range(Scene_viewer_get_scene(scene_viewer),
			&centre_x,&centre_y,&centre_z,&size_x,&size_y,&size_z);
//		radius = 0.3*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);
		
		/* enlarge radius to keep image within edge of window */
		/*???RC width_factor should be read in from defaults file */
		width_factor = 1.05;
		radius *= width_factor;
			
		/*???RC clip_factor should be read in from defaults file: */
		clip_factor = 10.0;		
		return_code = Scene_viewer_set_view_simple(scene_viewer, centre_x, centre_y,
			centre_z, radius, 40, clip_factor*radius);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_view_all.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} 

std::vector<std::string> EnumerateAllFiles(const std::string& dirname)
{
	using std::vector;
	using std::string;
	
	wxString wxDirname(dirname.c_str());
	wxDir dir(wxDirname);

	if ( !dir.IsOpened() )
	{
		// deal with the error here - wxDir would already log an error message
		// explaining the exact reason of the failure
		return vector<string>();
	}

	puts("Enumerating files in current directory:");

	vector<string> filenames;
	wxString filename;

	bool cont = dir.GetFirst(&filename, "", wxDIR_FILES);
	while ( cont )
	{
		printf("%s\n", filename.c_str());
		filenames.push_back(filename.c_str());
		cont = dir.GetNext(&filename);
	}
	return filenames;
}

//const char* TEST_DIR = "./temp/XMLZipTest";
static long const LABEL_COLUMN_INDEX = 4;

} // end anonymous namespace

namespace cap
{

std::string const ImageBrowseWindow::IMAGE_PREVIEW = std::string("ImagePreview");

static int dummy_input_callback(struct Scene_viewer *scene_viewer,
		struct Graphics_buffer_input *input, void *viewer_frame_void)
{
	return 0; // returning false means don't call the other input handlers;
}

ImageBrowseWindow::ImageBrowseWindow(std::string const& archiveFilename, CmguiManager const& manager, ImageBrowseWindowClient& client)
:
	archiveFilename_(archiveFilename),
	texturesCurrentlyOnDisplay_(0),
	cmguiManager_(manager),
	client_(client),
	sortingMode_(SERIES_NUMBER)
{	
	LoadWindowLayout();
	CreatePreviewPanel();
	
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
	
	FitWindow();
}

ImageBrowseWindow::ImageBrowseWindow(SlicesWithImages const& slicesWithImages, CmguiManager const& manager, ImageBrowseWindowClient& client)
:
	texturesCurrentlyOnDisplay_(0),
	cmguiManager_(manager),
	client_(client),
	sortingMode_(SERIES_NUMBER)
{	
	LoadWindowLayout();
	CreatePreviewPanel();
	
	std::set<int> setOfSeriesNumbers;
	BOOST_FOREACH(SliceInfo const& sliceInfo, slicesWithImages)
	{
		size_t numberOfFrames = sliceInfo.GetDICOMImages().size();
		assert(numberOfFrames);
		for (size_t i = 0; i < numberOfFrames; ++i)
		{
			// Need to assign unigue id for each image (here we use the file name)
			// this id is used to map a DICOMFile object to its corresponding Cmiss_texture
			DICOMPtr const& dicomPtr = sliceInfo.GetDICOMImages().at(i);
			std::string const& filename = dicomPtr->GetFilename();
			dicomFileTable_.insert(std::make_pair(filename, dicomPtr));
			Cmiss_texture_id texture = sliceInfo.GetTextures().at(i);
			textureTable_.insert(std::make_pair(filename, texture));
		}
		
		// Check if sortingMode_ is SERIES_NUMBER or SERIES_NUMBER_AND_IMAGE_POSITION
		// by looking at whether images from any two slices share the same series number
		DICOMPtr const& dicomPtr = sliceInfo.GetDICOMImages().at(0);
		int seriesNumber = dicomPtr->GetSeriesNumber();
		if (setOfSeriesNumbers.count(seriesNumber))
		{
			sortingMode_ = SERIES_NUMBER_AND_IMAGE_POSITION;
		}
		setOfSeriesNumbers.insert(seriesNumber);
	}
	
	PopulateImageTable();
	// Now populate the Image Table with the correct labels
	// Since the sliceMap may not be in the same sort order as slicesWithImages,
	// We need to map the two 
	// TODO: revise the data structures used so this mapping is not needed?
	long index = 0;
	BOOST_FOREACH(SliceMap::value_type const& value, sliceMap_)
	{
		// find the matching slice in the SlicesWithImages
		// Not that this is O(n^2) operation but should be ok as the number of slices are usually small (20~40)
		std::string const& filename1 = value.second.at(0)->GetSopInstanceUID();
		BOOST_FOREACH(SliceInfo const& sliceInfo, slicesWithImages)
		{
			std::string const& filename2 = sliceInfo.GetDICOMImages().at(0)->GetSopInstanceUID();
			if (filename1 == filename2) // matching slices
			{
				std::string const& label = sliceInfo.GetLabel();
				std::string longLabel;
				if (label.find("LA") == 0)
				{
					longLabel = "Long Axis";
				}
				else if (label.find("SA") == 0)
				{
					longLabel = "Short Axis";
				}
				imageTable_->SetItem(index, LABEL_COLUMN_INDEX, longLabel.c_str());
				index ++;
			}
		}
	}
	
	FitWindow();
}

ImageBrowseWindow::ImageBrowseWindow(DICOMTable const& dicomFileTable, TextureTable const& textureTable, CmguiManager const& manager, ImageBrowseWindowClient& client)
:
		dicomFileTable_(dicomFileTable),
		textureTable_(textureTable),
		cmguiManager_(manager),
		client_(client)
{	
}

void ImageBrowseWindow::LoadWindowLayout()
{
	wxXmlResource::Get()->Load("ImageBrowseWindow.xrc");
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ImageBrowseWindow"));
	Show(true); // gtk crashes without this
	imageTable_ = XRCCTRL(*this, "ImageTable", wxListCtrl);
}

void ImageBrowseWindow::CreatePreviewPanel()
{
	wxPanel* panel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	sceneViewer_ = cmguiManager_.CreateSceneViewer(panel, IMAGE_PREVIEW);
	Cmiss_scene_viewer_add_input_callback(sceneViewer_,
			dummy_input_callback, (void*)this, 1/*add_first*/);

	LoadImagePlaneModel();
}

void ImageBrowseWindow::FitWindow()
{
	// fit the window to meet the size requirements of its children
	this->Fit();
	
	// This stops the window from getting too long in height 
	// when there are many items in the Image Table
	this->SetSize(-1, 768);
	this->Centre();
	
	// Hack for working around bug ID: 3053989
	// see https://sourceforge.net/tracker/?func=detail&aid=3053989&group_id=237340&atid=1103258 for more info
#ifdef WIN32	
	this->Maximize(true);
#endif
}

void ImageBrowseWindow::CreateImageTableColumns()
{	
	long columnIndex = 0;
	imageTable_->InsertColumn(columnIndex++, _("Series #"), wxLIST_FORMAT_CENTRE, 75);
	imageTable_->InsertColumn(columnIndex++, _("Series Description"), wxLIST_FORMAT_CENTRE, -1);
	imageTable_->InsertColumn(columnIndex++, _("Sequence Name"), wxLIST_FORMAT_CENTRE, 120);
//	imageTable_->InsertColumn(columnIndex++, _("Series Time"));
	imageTable_->InsertColumn(columnIndex++, _("Images"), wxLIST_FORMAT_CENTRE, 75);
	imageTable_->InsertColumn(columnIndex, _("Label"), wxLIST_FORMAT_CENTRE, 85);
}

ImageBrowseWindow::~ImageBrowseWindow()
{
	Cmiss_scene_viewer_destroy(&sceneViewer_);
	Cmiss_context_execute_command(cmguiManager_.GetCmissContext(),
			("gfx destroy scene " + IMAGE_PREVIEW).c_str());
	//TODO : destroy textures??
}

void ImageBrowseWindow::CreateProgressDialog(std::string const& title, std::string const& message, int max)
{
	progressDialogPtr_.reset(new wxProgressDialog(_(title.c_str()), message.c_str(), max, this));
}

void ImageBrowseWindow::UpdateProgressDialog(int count)
{
	progressDialogPtr_->Update(count);
}

void ImageBrowseWindow::DestroyProgressDialog()
{
	progressDialogPtr_.reset(0);
}

void ImageBrowseWindow::ReadInDICOMFiles()
{
	//	std::string dirname = TEST_DIR;
	// TODO unzip archive file
	// for now we assume the archive has already been unzipped
	// and archiveFilename points to the containing dir
	std::string const& dirname = archiveFilename_;
	std::vector<std::string> const& filenames = EnumerateAllFiles(dirname);
	
	std::cout << "num files = " << filenames.size() << '\n';
	numberOfDICOMFiles_ = filenames.size();
	
//	wxProgressDialog progressDlg(_("Please wait"), _("Analysing DICOM headers"),
//		numberOfDICOMFiles_, this);
	CreateProgressDialog("Please wait", "Analysing DICOM headers", numberOfDICOMFiles_);
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
//			progressDlg.Update(count);
			UpdateProgressDialog(count);
		}
	}
	DestroyProgressDialog(); // REVISE interface 
}

void ImageBrowseWindow::SortDICOMFiles()
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

void ImageBrowseWindow::PopulateImageTableRow(int rowNumber,
		int seriesNumber, std::string const& seriesDescription,
		std::string const& sequenceName, size_t numImages,
		long int const& userDataPtr)
{
	using namespace std;
	using namespace boost;
	long itemIndex = imageTable_->InsertItem(rowNumber, lexical_cast<string>(seriesNumber).c_str());
	long columnIndex = 1;
	imageTable_->SetItem(itemIndex, columnIndex++, seriesDescription.c_str()); 
	imageTable_->SetItem(itemIndex, columnIndex++, sequenceName.c_str());
//		double triggerTime = image->GetTriggerTime();
//		std::string seriesTime = triggerTime < 0 ? "" : lexical_cast<string>(image->GetTriggerTime());// fix
//		imageTable_->SetItem(itemIndex, columnIndex++, seriesTime.c_str());
	imageTable_->SetItem(itemIndex, columnIndex++, lexical_cast<string>(numImages).c_str());
	
	imageTable_->SetItemData(itemIndex, userDataPtr); // Check !! is this safe??!!
}

void ImageBrowseWindow::SelectFirstRowInImageTable()
{
	imageTable_->SetItemState(0 , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
	imageTable_->SetItemState(0 , wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

void ImageBrowseWindow::ClearImageTable()
{
	imageTable_->ClearAll();
}

void ImageBrowseWindow::PopulateImageTable()
{	
	SortDICOMFiles();
	ConstructTextureMap();
	
	ClearImageTable();
	CreateImageTableColumns();
	
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
		
		PopulateImageTableRow(rowNumber,
				seriesNumber, seriesDescription,
				sequenceName, numImages,
				userDataPtr);
		
		rowNumber++;
	}
	
	// Set the selection to the first item in the list
	SelectFirstRowInImageTable();
	
//	imageTable_->SetScrollPos(wxVERTICAL, 0); // This does not work on wxListCtrl!
	
	DICOMPtr const& firstImage = sliceMap_.begin()->second[0];
	UpdatePatientInfoPanel(firstImage);
}

void ImageBrowseWindow::CreateTexturesFromDICOMFiles()
{
	using namespace std;
	
	// load some images and display
	wxProgressDialog progressDlg(_("Please wait"), _("Loading DICOM images"),
		numberOfDICOMFiles_, this);
	
	int count = 0;
	BOOST_FOREACH(DICOMTable::value_type const& value, dicomFileTable_)
	{	
		const string& filename = value.first;	
		Cmiss_texture_id texture_id = cmguiManager_.LoadCmissTexture(filename);
		textureTable_.insert(make_pair(filename, texture_id));
		
		count++;
		if (!(count % 20))
		{
			progressDlg.Update(count);
		}
	}
	return;
}

void ImageBrowseWindow::ConstructTextureMap()
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

void ImageBrowseWindow::SetAnimationSliderMax(size_t max)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSlider", wxSlider);
	assert(slider);	
	if (max == 1)
	{
		// special case where a slice contains only 1 image
		slider->Enable(false);
		slider->SetMin(1);
		slider->SetMax(2); // gtk requires max > min
	}
	else
	{
		slider->Enable(true);
		slider->SetMin(1);
		slider->SetMax(max);
		slider->SetValue(1);
	}
}

void ImageBrowseWindow::SwitchSliceToDisplay(SliceMap::value_type const& slice)
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
	SetAnimationSliderMax(images.size());
	
	// Resize the rectangular cmgui model
	// according to the new dimensions
	size_t width = images[0]->GetImageWidth();
	size_t height = images[0]->GetImageHeight();
	ResizePreviewImage(width, height);
	
	DisplayImage(textures[0]);

	double radius = std::max(width, height) / 2.0;
	FitSceneViewer(sceneViewer_ ,radius);
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
}

void ImageBrowseWindow::ResizePreviewImage(int width, int height)
{
	Cmiss_context_id cmissContext_ = cmguiManager_.GetCmissContext();
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_region* region = Cmiss_region_find_subregion_at_path(root_region, IMAGE_PREVIEW.c_str());
	assert(region);
	
	Cmiss_node* node = Cmiss_region_get_node(region, "1");
	FE_node_set_position_cartesian(node, 0, 0.0, 0.0, 0.0);	
	node = Cmiss_region_get_node(region, "2");
	FE_node_set_position_cartesian(node, 0, width, 0.0, 0.0);
	node = Cmiss_region_get_node(region, "3");
	FE_node_set_position_cartesian(node, 0, 0.0, height, 0.0);
	node = Cmiss_region_get_node(region, "4");
	FE_node_set_position_cartesian(node, 0, width, height, 0.0);

	Cmiss_region_destroy(&region);
	Cmiss_region_destroy(&root_region);
}

void ImageBrowseWindow::LoadImagePlaneModel()
{	
	cmguiManager_.ReadRectangularModelFiles(IMAGE_PREVIEW, IMAGE_PREVIEW);	
	material_ = cmguiManager_.CreateCAPMaterial(IMAGE_PREVIEW);
	cmguiManager_.AssignMaterialToObject(sceneViewer_, material_->GetCmissMaterial(), IMAGE_PREVIEW);
	
	return;
}

void ImageBrowseWindow::DisplayImage(Cmiss_texture_id tex)
{	
	if (material_)
	{
		material_->ChangeTexture(tex);
	}
	else
	{
		std::cout << "Error: cant find material\n";
	}

	return;
}

std::string ImageBrowseWindow::GetCellContentsString( long row_number, int column ) const
{
	wxListItem     row_info;  
	wxString       cell_contents_string;
	
	// Set what row it is (m_itemId is a member of the regular wxListCtrl class)
	row_info.m_itemId = row_number;
	// Set what column of that row we want to query for information.
	row_info.m_col = column;
	// Set text mask
	row_info.m_mask = wxLIST_MASK_TEXT;
	
	// Get the info and store it in row_info variable.   
	imageTable_->GetItem( row_info );
	
	// Extract the text out that cell
	cell_contents_string = row_info.m_text; 
	
	return cell_contents_string.c_str();
}

void ImageBrowseWindow::SetInfoField(std::string const& fieldName, std::string const& data)
{
	wxStaticText* st = XRCCTRL(*this, fieldName.c_str(), wxStaticText);
	st->SetLabel(data.c_str());
}

void ImageBrowseWindow::UpdatePatientInfoPanel(DICOMPtr const& image)
{
	using std::string;
	string const& name = image->GetPatientName();
	SetInfoField("PatientName", name);
	string const& id = image->GetPatientID();
	SetInfoField("PatientID", id);
	string const& scanDate = image->GetScanDate();
	SetInfoField("ScanDate", scanDate);
	string const& dob = image->GetDateOfBirth();
	SetInfoField("DateOfBirth", dob);
	string const& gender = image->GetGender();
	string const& age = image->GetAge();
	SetInfoField("GenderAndAge", gender + " " + age);
	
	//FIX temporory this should move to a separate function
	wxChoice* choice = XRCCTRL(*this, "m_choice1", wxChoice);
	choice->Clear();
	choice->Append((id + " " + scanDate).c_str());
	choice->SetSelection(0);
}

void ImageBrowseWindow::OnImageTableItemSelected(wxListEvent& event)
{	
	SliceMap::value_type* const sliceValuePtr = reinterpret_cast<SliceMap::value_type* const>(event.GetItem().GetData());
//	std::cout << "Series Num = " << (*sliceValuePtr).first.first << '\n';
//	std::cout << "Distance to origin = " << (*sliceValuePtr).first.second << '\n';
//	std::cout << "Image filename = " << (*sliceValuePtr).second[0]->GetFilename() << '\n';
		
	// Display the images from the selected row.
	SwitchSliceToDisplay(*sliceValuePtr);
}

void ImageBrowseWindow::UpdateImageInfoPanel(DICOMPtr const& dicomPtr)
{
	using std::string;
	using boost::lexical_cast;
	
	size_t width = dicomPtr->GetImageWidth();
	size_t height = dicomPtr->GetImageHeight();
	string size = lexical_cast<string>(width) + " x " + lexical_cast<string>(height);
	SetInfoField("ImageSize", size);
	
	Point3D position = dicomPtr->GetImagePosition();
	std::stringstream ss;
	ss << std::setprecision(5) << position.x << " ";
	ss << std::setprecision(5) << position.y << " ";
	ss << std::setprecision(5) << position.z;
	string const &posStr(ss.str());
	SetInfoField("ImagePosition", posStr);
	
	std::pair<Vector3D,Vector3D> const& orientation = dicomPtr->GetImageOrientation();

	std::cout << "ori 1: " << orientation.first.x << " " << orientation.first.y << " " << orientation.first.z << "\n";
	std::cout << "ori 2: " << orientation.second.x << " " << orientation.second.y << " " << orientation.second.z << "\n";
}

void ImageBrowseWindow::OnPlayToggleButtonPressed(wxCommandEvent& event)
{
}

void ImageBrowseWindow::OnAnimationSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
	int textureIndex = value - 1; // tex index is 0 based while slider value is 1 based
	
	DisplayImage((*texturesCurrentlyOnDisplay_)[value-1]);
//	Time_keeper_request_new_time(timeKeeper_, time);
	
	// force redraw while silder is manipulated
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
	return;
}

void ImageBrowseWindow::OnAnimationSpeedControlEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "AnimationSpeedControl", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
	
	double speed = (double)(value - min) / (double)(max - min) * 2.0;
//	Time_keeper_set_speed(timeKeeper_, speed);
//	
//	RefreshCmguiCanvas(); // forces redraw while silder is manipulated
	return;
}

void ImageBrowseWindow::OnBrightnessSliderEvent(wxCommandEvent& event)
{
	int value = event.GetInt();
//	std::cout << __func__ << "- event.GetInt() = " << value << "\n";
	float brightness = static_cast<float>(value)/100.0;
	material_->SetBrightness(brightness);
	
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
}

void ImageBrowseWindow::OnContrastSliderEvent(wxCommandEvent& event)
{
	wxSlider* slider = XRCCTRL(*this, "ContrastSlider", wxSlider);
	int value = slider->GetValue();
	int min = slider->GetMin();
	int max = slider->GetMax();
//	std::cout << __func__ << "- slide.GetMin() = " << min << "\n";
//	std::cout << __func__ << "- sliderGetMax() = " << max << "\n";
	
	float contrast = (float)(value - min) / (float)(max - min);
	assert(material_);
	material_->SetContrast(contrast);
	
	Cmiss_scene_viewer_redraw_now(sceneViewer_);
}

void ImageBrowseWindow::PutLabelOnSelectedSlice(std::string const& label)
{
	std::cout << __func__ << '\n';
	long index = imageTable_->GetNextItem(-1,
						wxLIST_NEXT_ALL,
						wxLIST_STATE_SELECTED);
	if (index != -1)
	{
		imageTable_->SetItem(index, LABEL_COLUMN_INDEX, label.c_str());
//		std::cout <<  "label = " << GetCellContentsString(index, LABEL_COLUMN_INDEX) << '\n';
		if (index < imageTable_->GetItemCount() - 1)
		{
			imageTable_->SetItemState(index  , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			imageTable_->SetItemState(index+1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
		else //HACK to refresh the table
		{
			imageTable_->SetItemState(index , 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED);
			imageTable_->SetItemState(index , wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		}
	}
}

void ImageBrowseWindow::OnShortAxisButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	PutLabelOnSelectedSlice("Short Axis");
}

void ImageBrowseWindow::OnLongAxisButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	PutLabelOnSelectedSlice("Long Axis");
}

void ImageBrowseWindow::OnNoneButtonEvent(wxCommandEvent& event)
{
//	std::cout << __func__ << '\n';
	PutLabelOnSelectedSlice("");
}

std::vector<std::pair<std::string, long int> > ImageBrowseWindow::GetListOfLabelsFromImageTable() const
{
	std::vector<std::pair<std::string, long int> > labels;
	
	long index = imageTable_->GetItemCount() - 1;
	while (index >= 0) // iterate from the bottom of the list ( to be compatible with CIM's setup)
	{
		std::string label = GetCellContentsString(index, LABEL_COLUMN_INDEX);
		std::cout << "index = " << index << ", label = " << label << '\n';		
		if (label.length())
		{
			long ptr = imageTable_->GetItemData(index);
			labels.push_back(std::make_pair(label, ptr));
		}
		index--;
	}
	
	return labels;
}

void ImageBrowseWindow::CreateMessageBox(std::string const& message, std::string const& caption)
{
	wxMessageBox(message.c_str(), caption.c_str(), wxOK, this);
}

void ImageBrowseWindow::OnOKButtonEvent(wxCommandEvent& event)
{
	std::cout << __func__ << '\n';
	// construct the data structure of type SlicesWithImages to pass to the main window
	SlicesWithImages slices;
	
	std::vector<std::pair<std::string, long int> > labels = GetListOfLabelsFromImageTable();
	
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
		CreateMessageBox("Too many long axes slices", "Invalid selection");
		return;
	}
	if (shortAxisCount >= 30)
	{
		std::cout << "TOO MANY SHORT AXES\n";
		CreateMessageBox("Too many short axes slices", "Invalid selection");
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
	Close();
}

void ImageBrowseWindow::OnCancelButtonEvent(wxCommandEvent& event)
{
	//TODO Cleanup textures - REVISE design
	// Should probably use reference counted smart pointer for Cmiss_texture
	// Since the ownership is shared between ImageSlice and this (ImageBrowseWindow)
	
	BOOST_FOREACH(TextureTable::value_type& value, textureTable_)
	{
		Cmiss_texture_id tex = value.second;
		DESTROY(Texture)(&tex);
	}
	Close();
}

void ImageBrowseWindow::OnOrderByRadioBox(wxCommandEvent& event)
{
//	std::cout << __func__ << " event.GetInt() = " << event.GetInt() << '\n';
	if (event.GetInt() == 0)
	{
		sortingMode_ = SERIES_NUMBER;
	}
	else if (event.GetInt() == 1)
	{
		sortingMode_ = SERIES_NUMBER_AND_IMAGE_POSITION;
	}
	else
	{
		throw std::logic_error("Invalid sorting mode");
	}
	
	PopulateImageTable();
}

void ImageBrowseWindow::ImageBrowseWindow::OnCloseImageBrowseWindow(wxCloseEvent& event)
{
	// TODO DO clean up!!
	Destroy();
//	exit(0);
}

BEGIN_EVENT_TABLE(ImageBrowseWindow, wxFrame)
	EVT_LIST_ITEM_SELECTED(XRCID("ImageTable"), ImageBrowseWindow::OnImageTableItemSelected)
	EVT_SLIDER(XRCID("AnimationSlider"),ImageBrowseWindow::OnAnimationSliderEvent)
	EVT_BUTTON(XRCID("PlayButton"),ImageBrowseWindow::OnPlayToggleButtonPressed) 
	EVT_SLIDER(XRCID("AnimationSpeedControl"),ImageBrowseWindow::OnAnimationSpeedControlEvent)
	EVT_SLIDER(XRCID("BrightnessSlider"),ImageBrowseWindow::OnBrightnessSliderEvent)
	EVT_SLIDER(XRCID("ContrastSlider"),ImageBrowseWindow::OnContrastSliderEvent)
	EVT_BUTTON(XRCID("ShortAxisButton"), ImageBrowseWindow::OnShortAxisButtonEvent)
	EVT_BUTTON(XRCID("LongAxisButton"), ImageBrowseWindow::OnLongAxisButtonEvent)
	EVT_BUTTON(XRCID("NoneButton"), ImageBrowseWindow::OnNoneButtonEvent)
	EVT_BUTTON(XRCID("wxID_OK"), ImageBrowseWindow::OnOKButtonEvent)
	EVT_BUTTON(XRCID("wxID_CANCEL"), ImageBrowseWindow::OnCancelButtonEvent)
	EVT_RADIOBOX(XRCID("OrderByRadioBox"), ImageBrowseWindow::OnOrderByRadioBox)
	EVT_CLOSE(ImageBrowseWindow::OnCloseImageBrowseWindow)
END_EVENT_TABLE()

} // end namespace cap
