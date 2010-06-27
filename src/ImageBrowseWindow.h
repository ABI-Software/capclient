/*
 * ImageBrowseWindow.h
 *
 *  Created on: Jun 16, 2010
 *      Author: jchu014
 */

#ifndef IMAGEBROWSEWINDOW_H_
#define IMAGEBROWSEWINDOW_H_
#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

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

namespace cap
{

class DICOMImage;
class CmguiManager;
class CAPMaterial;
class ImageBrowseWindowClient;

class ImageBrowseWindow : public wxFrame
{
public:
	ImageBrowseWindow(std::string const& archiveFilename, CmguiManager const& manager ,ImageBrowseWindowClient&);
	virtual ~ImageBrowseWindow();
	
private:
	typedef std::pair<int, double> SliceKeyType;
	typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;
	typedef std::map<SliceKeyType, std::vector<DICOMPtr> > SliceMap;
	typedef std::map<SliceKeyType, std::vector<Cmiss_texture_id> > TextureMap;
		
	std::string GetCellContentsString( long row_number, int column );
	void CreateImageTableColumns();
	void SortDICOMFiles();
	void PopulateImageTable();
	void LoadImages();
	void SwitchSliceToDisplay(SliceMap::value_type const& slice);
	void LoadImagePlaneModel();
	void ResizePreviewImage(int width, int width);
	void DisplayImage(Cmiss_texture_id tex);
	void UpdateImageInfoPanel(DICOMPtr const& image);
	void UpdatePatientInfoPanel(DICOMPtr const& image);
	
	void SetInfoField(std::string const& fieldName, std::string const& data);
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
	
	DECLARE_EVENT_TABLE();
	
	std::string archiveFilename_;
	CmguiManager const& cmguiManager_;
	ImageBrowseWindowClient& client_;
	
	size_t numberOfDICOMFiles_;
	Cmiss_scene_viewer_id sceneViewer_;
	std::tr1::shared_ptr<CAPMaterial> material_;
	
	wxListCtrl* imageTable_;
	
	SliceMap sliceMap_;
	TextureMap textureMap_; // this could be merged with sliceMap_
	std::vector<Cmiss_texture_id> const* texturesCurrentlyOnDisplay_;
	
	static const std::string IMAGE_PREVIEW;
};

} // end namespace cap
#endif /* IMAGEBROWSEWINDOW_H_ */
