/*
 * ImageBrowseWindow.h
 *
 *  Created on: Jun 16, 2010
 *      Author: jchu014
 */

#ifndef IMAGEBROWSEWINDOW_H_
#define IMAGEBROWSEWINDOW_H_

#include "SliceInfo.h"

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/tr1/memory.hpp>
#include <string>
#include <vector>
#include <map>

struct Cmiss_scene_viewer;
typedef struct Cmiss_scene_viewer *Cmiss_scene_viewer_id;
struct Cmiss_texture;
typedef struct Cmiss_texture* Cmiss_texture_id;
//struct Graphical_material;

class wxListCtrl;
class wxListEvent;
class wxProgressDialog;

namespace cap
{

class DICOMImage;
class CmguiManager;
class CAPMaterial;
class ImageBrowseWindowClient;

class ImageBrowseWindow : public wxFrame
{
public:
	typedef std::map<std::string, DICOMPtr> DICOMTable;
	typedef std::map<std::string, Cmiss_texture_id> TextureTable;
	
	ImageBrowseWindow(std::string const& archiveFilename, CmguiManager const& manager ,ImageBrowseWindowClient&);
	ImageBrowseWindow(SlicesWithImages const& slicesWithImages, CmguiManager const& manager, ImageBrowseWindowClient&);
	ImageBrowseWindow(DICOMTable const& dicomFileTable, TextureTable const& textureTable, CmguiManager const& manager, ImageBrowseWindowClient&);
	virtual ~ImageBrowseWindow();
	
	void SetInfoField(std::string const& fieldName, std::string const& data);
	void SetAnimationSliderMax(size_t max);
	void PopulateImageTableRow(int rowNumber,
			int seriesNumber, std::string const& seriesDescription,
			std::string const& sequenceName, size_t numImages,
			long int const& userDataPtr);
	void SelectFirstRowInImageTable();
	void ClearImageTable();
	
	void CreateProgressDialog(std::string const& title, std::string const& message, int max);
	void UpdateProgressDialog(int count);
	void DestroyProgressDialog();
	
	void CreateMessageBox(std::string const& message, std::string const& caption);
	
	std::vector<std::pair<std::string, long int> > GetListOfLabelsFromImageTable() const;
			
private:
	typedef std::pair<int, double> SliceKeyType;
//	typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;
	typedef std::map<SliceKeyType, std::vector<DICOMPtr> > SliceMap;
	typedef std::map<SliceKeyType, std::vector<Cmiss_texture_id> > TextureMap;
	
	void LoadWindowLayout();
	void CreatePreviewPanel();
	void FitWindow();
	
	std::string GetCellContentsString( long row_number, int column ) const;
	void CreateImageTableColumns();
	void ReadInDICOMFiles();
	void SortDICOMFiles();
	void PopulateImageTable();
	void CreateTexturesFromDICOMFiles();
	void ConstructTextureMap();
	void SwitchSliceToDisplay(SliceMap::value_type const& slice);
	void LoadImagePlaneModel();
	void ResizePreviewImage(int width, int height);
	void DisplayImage(Cmiss_texture_id tex);
	void UpdateImageInfoPanel(DICOMPtr const& image);
	void UpdatePatientInfoPanel(DICOMPtr const& image);
	void PutLabelOnSelectedSlice(std::string const& label);
	
	//Event handlers
	void OnImageTableItemSelected(wxListEvent& event);
	void OnCloseImageBrowseWindow(wxCloseEvent& event);
	void OnPlayToggleButtonPressed(wxCommandEvent& event);
	void OnAnimationSliderEvent(wxCommandEvent& event);
	void OnAnimationSpeedControlEvent(wxCommandEvent& event);
	void OnBrightnessSliderEvent(wxCommandEvent& event);
	void OnContrastSliderEvent(wxCommandEvent& event);
	void OnShortAxisButtonEvent(wxCommandEvent& event);
	void OnLongAxisButtonEvent(wxCommandEvent& event);
	void OnNoneButtonEvent(wxCommandEvent& event);
	void OnOKButtonEvent(wxCommandEvent& event);
	void OnCancelButtonEvent(wxCommandEvent& event);
	void OnOrderByRadioBox(wxCommandEvent& event);
	
	DECLARE_EVENT_TABLE();
	
	std::string archiveFilename_;
	CmguiManager const& cmguiManager_;
	ImageBrowseWindowClient& client_;
	
	size_t numberOfDICOMFiles_;
	enum SortingMode
	{
		SERIES_NUMBER,
		SERIES_NUMBER_AND_IMAGE_POSITION
	};
	
	SortingMode sortingMode_;
	
	Cmiss_scene_viewer_id sceneViewer_;
	std::tr1::shared_ptr<CAPMaterial> material_;
	
	wxListCtrl* imageTable_;
	
	SliceMap sliceMap_;
	TextureMap textureMap_; // this could be merged with sliceMap_
	std::vector<Cmiss_texture_id> const* texturesCurrentlyOnDisplay_;
	
//	typedef std::map<std::string, DICOMPtr> DICOMTable;
//	typedef std::map<std::string, Cmiss_texture_id> TextureTable;
	DICOMTable dicomFileTable_; // unsorted list of all dicom files
	TextureTable textureTable_; // unsorted list of all textures
	
	static const std::string IMAGE_PREVIEW;
	
	boost::scoped_ptr<wxProgressDialog> progressDialogPtr_;
};

} // end namespace cap
#endif /* IMAGEBROWSEWINDOW_H_ */
