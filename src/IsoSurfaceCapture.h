/*
 * IsoSurfaceCapture.h
 *
 *  Created on: Mar 19, 2011
 *      Author: jchu014
 */

#ifndef ISOSURFACECAPTURE_H_
#define ISOSURFACECAPTURE_H_

#include "ImageSet.h"
#include "CAPModelLVPS4X4.h"
#include "SliceInfo.h"

#include "wx/wxprec.h"
// For compilers that don't support precompilation, include "wx/wx.h";
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

extern "C"
{
//#include "api/cmiss_time_keeper.h"
//#include "api/cmiss_time.h"
#include "command/cmiss.h"
#include "graphics/scene.h"	
#include "graphics/scene_viewer.h"
//#include "three_d_drawing/graphics_buffer.h"
//#include "general/debug.h"
//#include "finite_element/export_finite_element.h"
}

namespace cap
{

class IsoSurfaceCapture :public wxFrame
{
public:
	IsoSurfaceCapture(ImageSet* imageSet, CAPModelLVPS4X4* heartModelPtr, Cmiss_context_id context, Cmiss_time_keeper_id timeKeeper)
	:
		wxFrame(NULL,-1, "Iso", wxPoint(100, 100), wxSize(1000, 1000)),
		imageSet_(imageSet),
		heartModelPtr_(heartModelPtr),
		context_(context),
		timeKeeper_(timeKeeper)
	{
		wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
		SetSizer( mainSizer );
		panel_ = new wxPanel(this, 0, 0, 156, 192);
		mainSizer->Add(panel_, 1, wxEXPAND);
		
//		panel_->SetSize(156);
		sceneViewer_ = Cmiss_scene_viewer_create_wx(Cmiss_context_get_default_scene_viewer_package(context_),
				//panel,
				panel_,
				CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
				CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
				/*minimum_colour_buffer_depth*/8,
				/*minimum_depth_buffer_depth*/8,
				/*minimum_accumulation_buffer_depth*/8);
		
		Cmiss_context_execute_command(context_, "gfx create scene print_temp manual_g_element");
		Cmiss_scene_viewer_set_scene_by_name(sceneViewer_, "print_temp");
		struct Scene *scene = Scene_viewer_get_scene(sceneViewer_);
		
		Cmiss_context_execute_command(context_, "gfx draw as heart group heart scene print_temp");
		// The above doesn't copy the transformation so it has to be done manually
		char* scene_object_name = (char*)"heart";
		
//		RenderIsoSurfaces();
			
		double centre_x, centre_y, centre_z, size_x, size_y, size_z;
		if (!Scene_get_graphics_range(scene, &centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z))
		{
			std::cout << "Error: Scene_get_graphics_range before transformation\n";
		}
		std::cout << "range before: " << centre_x  << ", " << centre_y << ", " 
				<< centre_z << ", " << size_x << ", " << size_y <<", "<<  size_z << std::endl;
		
		struct Scene_object * modelSceneObject=Scene_get_Scene_object_by_name(scene, scene_object_name);
		if (modelSceneObject)
		{
			assert(heartModelPtr_);
			const gtMatrix& patientToGlobalTransform = heartModelPtr_->GetLocalToGlobalTransformation();
			Scene_object_set_transformation(modelSceneObject, const_cast<gtMatrix*>(&patientToGlobalTransform));
		}
		else
		{
			display_message(ERROR_MESSAGE,"No object named '%s' in scene",scene_object_name);
		}
				
		Fit();
		Show(false);
	}
	
	
	double DefineDotProductField(std::string const& fieldName, ImagePlane const& plane) const
	{
		char str[256];
		
		assert(heartModelPtr_);
		const gtMatrix& m = heartModelPtr_->GetLocalToGlobalTransformation();//CAPModelLVPS4X4::

		gtMatrix mInv;
		inverseMatrix(m, mInv);
	//	cout << mInv << endl;
		transposeMatrix(mInv); // gtMatrix is column Major and our matrix functions assume row major FIX!!
	//	cout << mInv << endl;
		
		//Need to transform the image plane using the Local to global transformation matrix of the heart (ie to hearts local coord)
		Vector3D normalTransformed = m * plane.normal;
		
		Point3D pointTLCTransformed = mInv * plane.tlc;
		double d = DotProduct((pointTLCTransformed - Point3D(0,0,0)), normalTransformed);
		
		sprintf((char*)str, "gfx define field /heart/slice_%s coordinate_system rectangular_cartesian dot_product fields heart_rc_coord \"[%f %f %f]\";",
					fieldName.c_str(),
					normalTransformed.x, normalTransformed.y, normalTransformed.z);
		std::cout << str << std::endl;
		Cmiss_context_execute_command(context_, str);
		return d;
	}
	
	void RenderIsoSurface(std::string const& fieldName, double isoValue) const
	{	
		char str[256];
		sprintf((char*)str, "gfx modify g_element heart iso_surfaces as XC iso_scalar slice_%s iso_values %f use_elements select_on material white selected_material default_selected render_shaded scene print_temp;;",
					fieldName.c_str(),
					isoValue);
		std::cout << str << std::endl;
		Cmiss_context_execute_command(context_, str);
		
//		sprintf((char*)str, "gfx modify g_element heart lines scene print_temp");
//		Cmiss_context_execute_command(context_, str);
	}
	
	void AdjustViewport(ImagePlane const& plane)
	{
		// compute the center of the image plane, eye(camera) position and the up vector
		Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
		Point3D eye = planeCenter + (plane.normal * 500); // this seems to determine the near clip plane
		Vector3D up(plane.yside);
		up.Normalise();
		
		// Set the camera so the image plane fills the screen
		if (!Cmiss_scene_viewer_set_lookat_parameters_non_skew(
				sceneViewer_, eye.x, eye.y, eye.z,
				planeCenter.x, planeCenter.y, planeCenter.z,
				up.x, up.y, up.z
				))
		{
			//Error;
		}
		
		double image_width = (plane.tlc - plane.trc).Length();
		double image_height = (plane.tlc - plane.blc).Length();
		double radius = std::min(image_width, image_height) / 2.0;
		double clip_factor = 10.0;
		int return_code = Scene_viewer_set_view_simple(sceneViewer_, planeCenter.x, planeCenter.y, planeCenter.z
				, radius, 45, clip_factor*radius);	
	}
	
	void OnExportModel(std::string const& dirname)
	{
		std::cout << __func__ << "\n";
		
		double currentTime = Cmiss_time_keeper_get_time(timeKeeper_);
		
		size_t const numberOfFrames = imageSet_->GetNumberOfFrames();
		std::vector<std::string> const& sliceNames = imageSet_->GetSliceNames();
		
		std::stringstream ss;
		ss << dirname << '/' << "FileNameToSopiuidMapping.txt";
		std::ofstream mappingFile(ss.str().c_str());
		
		BOOST_FOREACH(std::string const& sliceName, sliceNames)
		{
			const ImagePlane& plane = imageSet_->GetImagePlane(sliceName);				
			AdjustViewport(plane);
			
			// Define the dot product field used for generating iso surfaces
			std::stringstream ss;
			ss << "ISO_" << sliceName;
			std::string const& fieldName(ss.str().c_str());
			
			double isoValue = DefineDotProductField(fieldName, plane);
			
			SlicesWithImages const& slices = imageSet_->GetSlicesWithImages();
			SlicesWithImages::const_iterator itr = 
					std::find_if(slices.begin(), slices.end(),
								boost::bind(&SliceInfo::GetLabel, _1) == sliceName);
			if (itr==slices.end())
			{
				throw std::logic_error(sliceName + " has no matching entry in SlicesWithImages");
			}
			
			// Render iso surface and take screen dump for each frame of this slice 
			for (size_t i = 0; i < numberOfFrames ; i++)
			{
				DICOMPtr const& dicomPtr = itr->GetDICOMImages().at(i);
				int imageWidth = dicomPtr->GetImageWidth();
				int imageHeight = dicomPtr->GetImageHeight();
				std::string const& patientId = dicomPtr->GetPatientID();
				std::string const& sopiuid = dicomPtr->GetSopInstanceUID();
				panel_->SetSize(imageWidth, imageHeight);
				Fit();
							
				double time = static_cast<double>(i)/static_cast<double>(numberOfFrames);
				Cmiss_time_keeper_set_time(timeKeeper_, time);
				RenderIsoSurface(fieldName, isoValue);
				Cmiss_scene_viewer_redraw_now(sceneViewer_);
				
				int force_onscreen_flag = 0;
				int antialias = 0;
				int transparency_layers = 0;
					
				std::stringstream filenameStream;
//				filenameStream << dirname << "/";
				filenameStream << patientId << "_" << sliceName << "_ph" << i << ".png" ;
				std::string filename = filenameStream.str();
				Cmiss_scene_viewer_write_image_to_file(sceneViewer_, (dirname + "/" + filename).c_str(), force_onscreen_flag,
					imageWidth, imageHeight, antialias, transparency_layers);
				
				mappingFile << filename << " " << sopiuid << '\n';
			}
		}
		
		Cmiss_scene_viewer_destroy(&sceneViewer_);
		Cmiss_context_execute_command(context_, "gfx destroy scene print_temp");
		Close();
	}
	
	void OnExportModelToBinaryVolume(std::string const& dirname, double apexMargin, double baseMargin, double spacing)
	{
		std::cout << __func__ << "\n";
		
		double currentTime = Cmiss_time_keeper_get_time(timeKeeper_);
		
		size_t const numberOfFrames = imageSet_->GetNumberOfFrames();
		
		std::string sliceName("SA1");
		const ImagePlane& plane = imageSet_->GetImagePlane(sliceName);
		AdjustViewport(plane);
		
		std::string infoFilename = dirname;
		infoFilename += "/Info.txt";
		std::ofstream infoFile(infoFilename.c_str());
		//write position of volume to file mappingFile NB need to use orig. pos
		infoFile << "Position: " << plane.tlc - plane.normal * apexMargin << '\n';
		double image_width = (plane.tlc - plane.trc).Length();
		double image_height = (plane.tlc - plane.blc).Length();
		infoFile << "Width: " << image_width << '\n';
		infoFile << "Height: " << image_height << '\n';
		infoFile << "Normal: " << plane.normal << '\n';
		// Define the dot product field used for generating iso surfaces
		std::string fieldName("ISO_");
		fieldName += sliceName;
			
		double apexIsoValue = DefineDotProductField(fieldName, plane);
			
		SlicesWithImages const& slices = imageSet_->GetSlicesWithImages();
		SlicesWithImages::const_iterator itr = 
				std::find_if(slices.begin(), slices.end(),
							boost::bind(&SliceInfo::GetLabel, _1) == sliceName);
		if (itr==slices.end())
		{
			throw std::logic_error(sliceName + " has no matching entry in SlicesWithImages");
		}
		DICOMPtr const& dicomPtr = itr->GetDICOMImages().at(0);
		int imageWidth = dicomPtr->GetImageWidth();
		int imageHeight = dicomPtr->GetImageHeight();
		std::string const& patientId = dicomPtr->GetPatientID();
		panel_->SetSize(imageWidth, imageHeight);
		Fit();
		
		std::vector<std::string> const& sliceNames = imageSet_->GetSliceNames();
		std::vector<std::string>::const_reverse_iterator revItr = 
				std::find_if(sliceNames.rbegin(), sliceNames.rend(), boost::bind(&std::string::substr, _1, 0, 2) == "SA");
		
		sliceName = *revItr; // last short axis slide
		const ImagePlane& planeBase = imageSet_->GetImagePlane(sliceName);
		double baseIsoValue = apexIsoValue + plane.normal * (planeBase.tlc - plane.tlc);
		
		double start = apexIsoValue + apexMargin;
		double end = baseIsoValue - baseMargin;
		int counter = 0;
		
		std::cout << "start = " << start << ", end = " << end << '\n';
		for (double d = start; d > end; d -= spacing, counter++)
		{
			// Render iso surface and take screen dump for each frame of this slice 
			for (size_t i = 0; i < numberOfFrames ; i++)
			{						
				double time = static_cast<double>(i)/static_cast<double>(numberOfFrames);
				Cmiss_time_keeper_set_time(timeKeeper_, time);
				RenderIsoSurface(fieldName, d);
				Cmiss_scene_viewer_redraw_now(sceneViewer_);
				
				int force_onscreen_flag = 0;
				int antialias = 0;
				int transparency_layers = 0;
					
				std::stringstream filenameStream;
//				filenameStream << dirname << "/";
				filenameStream << patientId << "_" << counter << "_ph" << i << ".png" ;
				std::string filename = filenameStream.str();
				Cmiss_scene_viewer_write_image_to_file(sceneViewer_, (dirname + "/" + filename).c_str(), force_onscreen_flag,
					imageWidth, imageHeight, antialias, transparency_layers);
			}
		}
		
		Cmiss_scene_viewer_destroy(&sceneViewer_);
		Cmiss_context_execute_command(context_, "gfx destroy scene print_temp");
		Close();
	}
	
private:
	ImageSet* imageSet_;
	CAPModelLVPS4X4* heartModelPtr_;
	Cmiss_context_id context_;
	wxPanel* panel_;
	Cmiss_scene_viewer_id sceneViewer_;
	Cmiss_time_keeper_id timeKeeper_;
};

} // namespace cap
#endif /* ISOSURFACECAPTURE_H_ */
