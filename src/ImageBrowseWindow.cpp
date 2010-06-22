/*
 * ImageBrowseWindow.cpp
 *
 *  Created on: Jun 17, 2010
 *      Author: jchu014
 */
#include "ImageBrowseWindow.h"

#include "Config.h"
//#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "DICOMImage.h"

#include <wx/xrc/xmlres.h>
#include <wx/listctrl.h>
#include <wx/dir.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lambda/lambda.hpp>

#include <iostream>
#include <sstream>
#include <fstream>

extern "C"
{
#include "api/cmiss_context.h"
#include "api/cmiss_scene_viewer.h"
#include "graphics/material.h"
#include "graphics/scene_viewer.h"
#include "graphics/element_group_settings.h"
#include "region/cmiss_region.h"
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

Cmiss_texture_id LoadCmissTexture(Cmiss_context_id context, std::string const& filename)
{
	Cmiss_region_id region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id field_module =  Cmiss_region_get_field_module(region);

	Cmiss_field_id field = Cmiss_field_module_create_image(field_module, NULL, NULL);
	Cmiss_field_image_id image_field = Cmiss_field_cast_image(field);
	
	/* Read image data from a file */
	Cmiss_field_image_read_file(image_field, filename.c_str());
	Cmiss_texture_id texture_id = Cmiss_field_image_get_texture(image_field);
	Cmiss_texture_set_filter_mode(texture_id, CMISS_TEXTURE_FILTER_LINEAR);
	
	return texture_id;
}

const char* TEST_DIR = "./temp/XMLZipTest";
} // end anonyous namespace

namespace cap
{

ImageBrowseWindow::ImageBrowseWindow(std::string const& archiveFilename, Cmiss_context_id context)
:
	archiveFilename_(archiveFilename),
	cmissContext_(context),
	material_(0)
{
	wxXmlResource::Get()->Load("ImageBrowseWindow.xrc");
	wxXmlResource::Get()->LoadFrame(this,(wxWindow *)NULL, _T("ImageBrowseWindow"));
	
	imageTable_ = XRCCTRL(*this, "ImageTable", wxListCtrl);

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
	
	wxPanel* panel = XRCCTRL(*this, "CmguiPanel", wxPanel);
	sceneViewer_ = Cmiss_scene_viewer_create_wx(Cmiss_context_get_default_scene_viewer_package(cmissContext_),
			panel,
			CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
			CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
			/*minimum_colour_buffer_depth*/8,
			/*minimum_depth_buffer_depth*/8,
			/*minimum_accumulation_buffer_depth*/8);
	
//		Cmiss_scene_viewer_view_all(sceneViewer_);
//		Cmiss_scene_viewer_set_perturb_lines(sceneViewer_, 1 );
	// TODO This window should use a separate scene from the default one (the one MainWindow uses)
	
	LoadImagePlaneModel();
	LoadImages();
}

ImageBrowseWindow::~ImageBrowseWindow()
{
	//TODO : destroy textures
}

void ImageBrowseWindow::PopulateImageTable()
{
	std::string dirname = TEST_DIR;
	std::vector<std::string> const& filenames = EnumerateAllFiles(dirname);
	
	std::cout << "num files = " << filenames.size() << '\n';
	
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
		SliceMap::iterator itr = sliceMap_.find(key);
		if (itr != sliceMap_.end())
		{
			itr->second.push_back(file);
		}
		else
		{
			std::vector<DICOMPtr> v(1, file);
			sliceMap_.insert(std::make_pair(key, v));
		}		
	}
	
	int rowNumber = 0;
	BOOST_FOREACH(SliceMap::value_type& value, sliceMap_)
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
		
		imageTable_->SetItemData(itemIndex, reinterpret_cast<long int>(&value)); // Check !! is this safe??!!
		rowNumber++;
	}
}

void ImageBrowseWindow::LoadImages()
{
	using namespace std;
	
	// load some images and display
	
	BOOST_FOREACH(SliceMap::value_type& value, sliceMap_)
	{	
		vector<DICOMPtr>& images = value.second;
		vector<DICOMPtr>::const_iterator itr = images.begin();
		vector<DICOMPtr>::const_iterator end = images.end();
		vector<Cmiss_texture_id> textures;
		
		for (; itr != end; ++itr)
		{
			const string& filename = (*itr)->GetFilename();
//			string texture_path(dir_path);
//			texture_path.append("/");
//			texture_path.append(filename);
	
			Cmiss_texture_id texture_id = LoadCmissTexture(cmissContext_, filename);
			textures.push_back(texture_id);
		}
		
		textureMap_.insert(make_pair(value.first, textures));
	}
	return;
}

void ImageBrowseWindow::SwitchSliceToDisplay(SliceKeyType const& key)
{
//	TextureMap::const_iterator itr = textureMap_.find(key);
	assert(textureMap_.find(key) != textureMap_.end());
	std::cout << __func__ << '\n';
	std::vector<Cmiss_texture_id> const& textures = textureMap_[key];
	
	DisplayImage(textures[0]);
	Cmiss_scene_viewer_view_all(sceneViewer_);
}

void ImageBrowseWindow::LoadImagePlaneModel()
{
	using namespace std;
	
	string name("LA1"); // change
	char filename[256];
	Cmiss_region* region = Cmiss_context_get_default_region(cmissContext_);
	
	// Read in ex files that define the element used to represent the image slice
	// TODO these should be done programatically
	sprintf(filename, "%stemplates/%s.exnode", CAP_DATA_DIR, name.c_str()); 
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exnode" << std::endl;
	}
	
	sprintf(filename, "%stemplates/%s.exelem", CAP_DATA_DIR, name.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exelem" << std::endl;
	}
		
	material_ = create_Graphical_material("ImageBrowseWindow");

	// Initialize shaders that are used for adjusting brightness and contrast
	
	stringstream vp_stream, fp_stream;
	ifstream is;
	is.open("Data/shaders/vp.txt");
	vp_stream << is.rdbuf();
	is.close();
	
	is.open("Data/shaders/fp.txt");
	fp_stream << is.rdbuf();
	is.close();
	
	if (!Material_set_material_program_strings(material_, 
			(char*) vp_stream.str().c_str(), (char*) fp_stream.str().c_str())
			)
	{
		cout << "Error: cant set material program strings" << endl;
	}
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(cmissContext_);
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	if (!scene)
	{
		cout << "Can't find scene" << endl;
	}

	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	//Got to find the child region first!!
	cout << "Subregion name = " << name << "\n";
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, name.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	// use the same material for selected material
	GT_element_settings_set_selected_material(settings, material_);

	if(!GT_element_settings_set_material(settings, material_))
	{
		//Error;
		std::cout << __func__ << " :Error setting material\n";
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		if (!Cmiss_region_modify_g_element(region, scene,settings,
			/*delete_flag*/0, /*position*/-1))
		{
			 //error
			std::cout << __func__ << " :Error modifying g element\n";
		}
	}

	
	// cache the sceneObject for convenience
//	sceneObject_ = Scene_get_scene_object_with_Cmiss_region(scene, region);
	return;
}

void ImageBrowseWindow::DisplayImage(Cmiss_texture_id tex)
{
	using namespace std;
	
	if (material_)
	{
		if (!Graphical_material_set_texture(material_,tex))//Bug this never returns 1 (returns garbage) - always returns 0 on windows
		{
			//Error
			//cout << "Error: Graphical_material_set_texture()" << endl;
		}
//		if (!Graphical_material_set_second_texture(material_, brightnessAndContrastTexture_))
//		{
//			//Error
//		}
	}
	else
	{
		cout << "Error: cant find material" << endl;
	}
	
	Cmiss_region* root_region = Cmiss_context_get_default_region(cmissContext_);
	//Got to find the child region first!!
	Cmiss_region* region;
	
	string name("LA1"); //temporary!!
	if(!(region = Cmiss_region_find_subregion_at_path(root_region, name.c_str())))
	{
		//error
		std::cout << "Cmiss_region_find_subregion_at_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	// use the same material for selected material
	GT_element_settings_set_selected_material(settings, material_);

	if(!GT_element_settings_set_material(settings, material_))
	{
		//Error;
		cout << "GT_element_settings_set_material() returned 0" << endl;
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_context_get_default_scene_viewer_package(cmissContext_);
		struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		

		if (!Cmiss_region_modify_g_element(region, scene,settings,
			/*delete_flag*/0, /*position*/-1))
		{
			 //error
			cout << "Cmiss_region_modify_g_element() returned 0" << endl;
		}
	}

	return;
}

wxString ImageBrowseWindow::GetCellContentsString( long row_number, int column )
{
	wxListItem     row_info;  
	wxString       cell_contents_string;
	
	// Set what row it is
	row_info.SetId(row_number);
	// Set what column of that row we want to query for information.
	row_info.SetColumn(column);
	// Set text mask
	row_info.SetState(wxLIST_MASK_TEXT);
	
	// Get the info and store it in row_info variable.   
	imageTable_->GetItem( row_info );
	
	// Extract the text out that cell
	cell_contents_string = row_info.GetText(); 
	
	return cell_contents_string;
}

void ImageBrowseWindow::OnImageTableItemSelected(wxListEvent& event)
{
	std::cout << __func__ << '\n';
//	std::cout << "id = " << event.GetId() << '\n';
	std::cout << "index = " << event.GetIndex() << '\n';
//	std::cout << "text = " << event.GetText() << '\n';
//	std::cout << "item.text = " << event.GetItem().GetText() << '\n';
//	std::cout << "item.id = " << event.GetItem().GetId() << '\n';
	
	SliceMap::value_type* const sliceValuePtr = reinterpret_cast<SliceMap::value_type* const>(event.GetItem().GetData());
	std::cout << "Series Num = " << (*sliceValuePtr).first.first << '\n';
	std::cout << "Distance to origin = " << (*sliceValuePtr).first.second << '\n';
	
	std::cout << "Image filename = " << (*sliceValuePtr).second[0]->GetFilename() << '\n';
	
	// Display the images from the selected row.
	SwitchSliceToDisplay((*sliceValuePtr).first);
}

BEGIN_EVENT_TABLE(ImageBrowseWindow, wxFrame)
	EVT_LIST_ITEM_SELECTED(XRCID("ImageTable"), ImageBrowseWindow::OnImageTableItemSelected)
END_EVENT_TABLE()

} // end namespace cap
