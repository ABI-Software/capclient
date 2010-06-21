/*
 * ImageBrowseWindow.cpp
 *
 *  Created on: Jun 17, 2010
 *      Author: jchu014
 */
#include "ImageBrowseWindow.h"

#include "CmguiManager.h"
#include "DICOMImage.h"
#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/dir.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tr1/memory.hpp>
#include <boost/lambda/lambda.hpp>

#include <iostream>
#include <vector>
#include <map>

extern "C"
{
#include "api/cmiss_scene_viewer.h"
}

namespace
{

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

	bool cont = dir.GetFirst(&filename, "*.dcm", wxDIR_FILES);
	while ( cont )
	{
		printf("%s\n", filename.c_str());
		filenames.push_back(filename.c_str());
		cont = dir.GetNext(&filename);
	}
	return filenames;
}

} // end anonyous namespace

namespace cap
{

ImageBrowseWindow::ImageBrowseWindow(std::string const& archiveFilename)
:
	archiveFilename_(archiveFilename)
{
	wxXmlResource::Get()->Load("ImageBrowseWindow.xrc");
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ImageBrowseWindow"));
	
	wxPanel* panel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	
	imageTable_ = XRCCTRL(*this, "m_listCtrl2", wxListCtrl);

	sceneViewer_ = CmguiManager::getInstance().createSceneViewer(panel);
//		Cmiss_scene_viewer_view_all(sceneViewer_);
//		Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );

	assert(imageTable_);
	
	imageTable_->InsertColumn(0, _("Series Number"));
	imageTable_->InsertColumn(1, _("Series Desc"));
	imageTable_->InsertColumn(2, _("Sequence Name"));
	imageTable_->InsertColumn(3, _("Series Time"));
	imageTable_->InsertColumn(4, _("Num Images"));
	imageTable_->InsertColumn(5, _("Label"));
	
//	wxString str = imageTable_->GetItemText(0);
//	std::cout << str << '\n';
//	std::cout << GetCellContentsString(0, 2) << '\n';
	
	PopulateImageTable();
}

ImageBrowseWindow::~ImageBrowseWindow()
{
}

void ImageBrowseWindow::PopulateImageTable()
{
	std::string dirname = "./temp/XMLZipTest";
	std::vector<std::string> const& filenames = EnumerateAllFiles(dirname);
	
	std::cout << "num files = " << filenames.size() << '\n';
	
	typedef std::pair<int, double> SliceKeyType;
	typedef std::tr1::shared_ptr<DICOMImage> DICOMPtr;
	typedef std::map<SliceKeyType, std::vector<DICOMPtr> > SliceMap;
	SliceMap sliceMap;
	
	BOOST_FOREACH(std::string const& filename, filenames)
	{
		std::cout << filename <<'\n';
		std::string fullpath = dirname + "/" + (filename);
		std::cout << fullpath <<'\n';
		DICOMPtr file(new DICOMImage(fullpath));
		int seriesNum = file->GetSeriesNumber();
		std::cout << "Series Num = " << seriesNum << '\n';
		Vector3D v = file->GetPosition() - Point3D(0,0,0);
		double distanceFromOrigin = v.Length();
		
		SliceKeyType key = std::make_pair(seriesNum, distanceFromOrigin);
		SliceMap::iterator itr = sliceMap.find(key);
		if (itr != sliceMap.end())
		{
			itr->second.push_back(file);
		}
		else
		{
			std::vector<DICOMPtr> v(1, file);
			sliceMap.insert(std::make_pair(key, v));
		}		
	}
	
	int rowNumber = 0;
	BOOST_FOREACH(SliceMap::value_type& value, sliceMap)
	{
		using boost::lexical_cast;
		using namespace boost::lambda;
		using std::string;
		
		std::vector<DICOMPtr>& images = value.second;
		std::sort(images.begin(), images.end(), *_1 < *_2);
		DICOMPtr image = images[0];
		long itemIndex = imageTable_->InsertItem(rowNumber, lexical_cast<string>(image->GetSeriesNumber()).c_str());
		imageTable_->SetItem(itemIndex, 1, image->GetSeriesDescription().c_str()); 
		imageTable_->SetItem(itemIndex, 2, image->GetSequenceName().c_str());
		double triggerTime = image->GetTriggerTime();
		std::string seriesTime = triggerTime < 0 ? "" : lexical_cast<string>(image->GetTriggerTime());// fix
		imageTable_->SetItem(itemIndex, 3, seriesTime.c_str());
		imageTable_->SetItem(itemIndex, 4, lexical_cast<string>(value.second.size()).c_str());
		rowNumber++;
	}
}

wxString ImageBrowseWindow::GetCellContentsString( long row_number, int column )
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
	
	return cell_contents_string;
}

BEGIN_EVENT_TABLE(ImageBrowseWindow, wxFrame)
END_EVENT_TABLE()

} // end namespace cap
