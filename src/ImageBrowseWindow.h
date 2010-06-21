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

#include <string>

struct Cmiss_scene_viewer;
typedef struct Cmiss_scene_viewer *Cmiss_scene_viewer_id;
class wxListCtrl;

namespace cap
{

class ImageBrowseWindow : public wxFrame
{
public:
	ImageBrowseWindow(std::string const& archiveFilename);
	virtual ~ImageBrowseWindow();
	
private:
	wxString GetCellContentsString( long row_number, int column );
	void PopulateImageTable();
	
	std::string archiveFilename_;
	Cmiss_scene_viewer_id sceneViewer_;
	wxListCtrl* imageTable_;
	//Event handlers
	DECLARE_EVENT_TABLE();
};

} // end namespace cap
#endif /* IMAGEBROWSEWINDOW_H_ */
