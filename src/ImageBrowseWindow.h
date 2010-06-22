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

struct Context;
typedef struct Context* Cmiss_context_id;
struct Cmiss_scene_viewer;
typedef struct Cmiss_scene_viewer *Cmiss_scene_viewer_id;
struct Cmiss_texture;
typedef struct Cmiss_texture* Cmiss_texture_id;
struct Graphical_material;

class wxListCtrl;
class wxListEvent;

namespace cap
{

class DICOMImage;

class ImageBrowseWindow : public wxFrame
{
public:
	ImageBrowseWindow(std::string const& archiveFilename, Cmiss_context_id command_data);
	virtual ~ImageBrowseWindow();
	
private:
	typedef std::pair<int, double> SliceKeyType;
	typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;
	typedef std::map<SliceKeyType, std::vector<DICOMPtr> > SliceMap;
	typedef std::map<SliceKeyType, std::vector<Cmiss_texture_id> > TextureMap;
		
	wxString GetCellContentsString( long row_number, int column );
	void PopulateImageTable();
	void LoadImages();
	void SwitchSliceToDisplay(SliceKeyType const& key);
	void LoadImagePlaneModel();
	void DisplayImage(Cmiss_texture_id tex);
	
	std::string archiveFilename_;
	Cmiss_context_id cmissContext_;
	Cmiss_scene_viewer_id sceneViewer_;
	Graphical_material* material_;
	
	wxListCtrl* imageTable_;
	
	SliceMap sliceMap_;
	TextureMap textureMap_; // this could be merged with sliceMap_
	
	//Event handlers
	void OnImageTableItemSelected(wxListEvent& event);
	
	DECLARE_EVENT_TABLE();
};

} // end namespace cap
#endif /* IMAGEBROWSEWINDOW_H_ */