
#ifdef UnitTestModeller
#include "modellercapclientwindow.h"
#include "modellercapclient.h"
#else
#include "capclientwindow.h"
#include "capclient.h"
#endif
#include "zinc/extensions.h"
#include "hexified/heartmodel.exnode.h"
#include "hexified/globalhermiteparam.exelem.h"
#include "utils/misc.h"
#include "utils/debug.h"
#include "model/heart.h"

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_core.h>
#include <zn/cmiss_status.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_finite_element.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_stream.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_element.h>
#include <zn/cmiss_node.h>
#include <zn/cmiss_field_composite.h>
#include <zn/cmiss_field_matrix_operators.h>
#include <zn/cmiss_time_keeper.h>
}

#include <iostream>
#include <sstream>

namespace cap
{

void CAPClientWindow::CreateHeartModel()
{
	RemoveHeartModel();

	heartModel_ = new HeartModel(cmissContext_);

	UpdateUI();
}

void CAPClientWindow::RemoveHeartModel()
{
	if (heartModel_ != 0)
	{
		delete heartModel_;
		heartModel_ = 0;
		RemoveMIIGraphics();
        UpdateUI();
	}
}

void CAPClientWindow::RemoveMIIGraphics()
{
	MIIGraphicMap::iterator miiMap_it = miiMap_.begin();
	while (miiMap_it != miiMap_.end())
	{
		Cmiss_graphic_destroy(&(miiMap_it->second.first));
		Cmiss_graphic_destroy(&(miiMap_it->second.second));
		miiMap_.erase(miiMap_it++);
	}
}

void CAPClientWindow::SetHeartModelFocalLength(double focalLength)
{
	assert(heartModel_);
	heartModel_->SetFocalLength(focalLength);
}

void CAPClientWindow::SetHeartModelMuFromBasePlaneAtTime(const Plane& basePlane, double time)
{
	assert(heartModel_);
	heartModel_->SetMuFromBasePlaneAtTime(basePlane, time);
}

void CAPClientWindow::SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time)
{
	assert(heartModel_);
	heartModel_->SetLambdaAtTime(lambdaParams, time);
}

int CAPClientWindow::ComputeHeartModelXi(const Point3D& position, double time, Point3D& xi) const
{
	assert(heartModel_);
	return heartModel_->ComputeXi(position, time, xi);
}

void CAPClientWindow::SetHeartModelTransformation(const gtMatrix& transform)
{
	assert(heartModel_);
	heartModel_->SetLocalToGlobalTransformation(transform);
}

void CAPClientWindow::LoadTemplateHeartModel(unsigned int numberOfModelFrames)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	// Create a heart region to stop Cmgui adding lines to the new rendition
	std::string region_name = "heart";
	Cmiss_region_id heart_region = Cmiss_region_create_child(root_region, region_name.c_str());
	Cmiss_region_destroy(&heart_region);

	LoadHermiteHeartElements();

	// Read in the heart model spaced over the number of model frames
	for (unsigned int i = 0; i < numberOfModelFrames; i++)
	{
		double time = static_cast<double>(i)/numberOfModelFrames;
		Cmiss_stream_information_id stream_information = Cmiss_region_create_stream_information(root_region);
		Cmiss_stream_information_region_id stream_information_region = Cmiss_stream_information_cast_region(stream_information);
		Cmiss_stream_resource_id stream_resource = Cmiss_stream_information_create_resource_memory_buffer(stream_information, heartmodel_exnode, heartmodel_exnode_len);
		Cmiss_stream_information_region_set_resource_attribute_real(stream_information_region, stream_resource, CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, time);

		Cmiss_region_read(root_region, stream_information);

		Cmiss_stream_resource_destroy(&stream_resource);
		Cmiss_stream_information_destroy(&stream_information);
		Cmiss_stream_information_region_destroy(&stream_information_region);
	}

	// Wrap the end point add another set of nodes at time 1.0
//	{
//		Cmiss_stream_information_id stream_information = Cmiss_region_create_stream_information(root_region);
//		Cmiss_stream_information_region_id stream_information_region = Cmiss_stream_information_cast_region(stream_information);
//		Cmiss_stream_resource_id stream_resource = Cmiss_stream_information_create_resource_memory_buffer(stream_information, heartmodel_exnode, heartmodel_exnode_len);
//		int r = Cmiss_stream_information_region_set_resource_attribute_real(stream_information_region, stream_resource, CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, 1.0);

//		Cmiss_region_read(root_region, stream_information);

//		Cmiss_stream_resource_destroy(&stream_resource);
//		Cmiss_stream_information_destroy(&stream_information);
//		Cmiss_stream_information_region_destroy(&stream_information_region);
//	}

	Cmiss_region_destroy(&root_region);
	// The initialisation must take place after the loading of the model as this defines the region and the coordinates field.
	heartModel_->SetNumberOfModelFrames(numberOfModelFrames);
	heartModel_->Initialise(region_name);
}

void CAPClientWindow::LoadHeartModel(std::string fullExelemFileName, std::vector<std::string> fullExnodeFileNames)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	// Create a heart region to stop Cmgui adding lines to the new rendition
	std::string region_name = "heart";
	Cmiss_region_id heart_region = Cmiss_region_create_child(root_region, region_name.c_str());
	Cmiss_region_destroy(&heart_region);

	LoadHermiteHeartElements(fullExelemFileName);

	unsigned int numberOfModelFrames = fullExnodeFileNames.size();

	Cmiss_stream_information_id stream_information = Cmiss_region_create_stream_information(root_region);
	Cmiss_stream_information_region_id stream_information_region = Cmiss_stream_information_cast_region(stream_information);
	//Cmiss_stream_resource_id stream_resources[] = new Cmiss_stream_resource_id[numberOfModelFrames+1];
	std::vector<Cmiss_stream_resource_id> stream_resources(numberOfModelFrames + 1);
	for (unsigned int i = 0; i < numberOfModelFrames; i++)
	{
		double time = static_cast<double>(i)/numberOfModelFrames;
		stream_resources[i] = Cmiss_stream_information_create_resource_file(stream_information, fullExnodeFileNames.at(i).c_str());

		Cmiss_stream_information_region_set_resource_attribute_real(stream_information_region, stream_resources[i], CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, time);
	}

	// Wrap the end point add another set of nodes at time 1.0
	{
		stream_resources[numberOfModelFrames] = Cmiss_stream_information_create_resource_file(stream_information, fullExnodeFileNames.at(0).c_str());
		Cmiss_stream_information_region_set_resource_attribute_real(stream_information_region, stream_resources[numberOfModelFrames], CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, 1.0);
	}
	Cmiss_region_read(root_region, stream_information);

	std::vector<Cmiss_stream_resource_id>::iterator it = stream_resources.begin();
	while (it != stream_resources.end())
	{
		Cmiss_stream_resource_destroy(&(*it));
		it++;
	}

	Cmiss_stream_information_region_destroy(&stream_information_region);
	Cmiss_stream_information_destroy(&stream_information);
	Cmiss_region_destroy(&root_region);

	// The initialisation must take place after the loading of the model as this defines the region and the coordinates field.
	heartModel_->Initialise(region_name);
	heartModel_->SetNumberOfModelFrames(numberOfModelFrames);
}

void CAPClientWindow::LoadHermiteHeartElements(std::string exelemFileName)
{
	// Read in the Hermite elements
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	Cmiss_stream_information_id stream_information = Cmiss_region_create_stream_information(root_region);
	Cmiss_stream_resource_id stream_resource = 0;
	if (exelemFileName.size() > 0)
		stream_resource = Cmiss_stream_information_create_resource_file(stream_information, exelemFileName.c_str());
	else
		stream_resource = Cmiss_stream_information_create_resource_memory_buffer(stream_information, globalhermiteparam_exelem, globalhermiteparam_exelem_len);

	Cmiss_region_read(root_region, stream_information);

	Cmiss_stream_resource_destroy(&stream_resource);
	Cmiss_stream_information_destroy(&stream_information);
	Cmiss_region_destroy(&root_region);
}

Point3D CAPClientWindow::ConvertToHeartModelProlateSpheriodalCoordinate(int node_id, const std::string& region_name) const
{
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, region_name);
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_id inv_projection_mx = Cmiss_field_module_find_field_by_name(field_module, "inv_projection_mx");
	Cmiss_field_id coordinates = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	//Cmiss_field_get_attribute_integer(coordinates, CMISS_FIELD_ATT
	Cmiss_field_id coordinates_ps = Cmiss_field_module_find_field_by_name(field_module, "coordinates_ps");
	if (!inv_projection_mx)
	{
		// Create the field pipeline
		double proj_mx_values[16];
		const gtMatrix& transform_mx = heartModel_->GetLocalToGlobalTransformation();
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				// Transposing from gtMatrix Column-major to Cmgui Row-major
				proj_mx_values[i+4*j] = transform_mx[i][j];
			}
		}
		Cmiss_field_id projection_mx = Cmiss_field_module_create_constant(field_module, 16, proj_mx_values);
		inv_projection_mx = Cmiss_field_module_create_matrix_invert(field_module, projection_mx);
		Cmiss_field_set_name(inv_projection_mx, "inv_projection_mx");
		Cmiss_field_id heart_template_rc = Cmiss_field_module_create_projection(field_module, coordinates, inv_projection_mx);
		Cmiss_field_set_name(heart_template_rc, "heart_rc");
		std::string command = "coordinate_system prolate_spheroidal focus " + ToString(heartModel_->GetFocalLength()) + " coordinate_transformation field heart_rc";
		coordinates_ps = Cmiss_field_module_create_field(field_module, "coordinates_ps", command.c_str());
		Cmiss_field_set_attribute_integer(coordinates_ps, CMISS_FIELD_ATTRIBUTE_IS_MANAGED, 1);

		Cmiss_field_destroy(&projection_mx);
		Cmiss_field_destroy(&heart_template_rc);
	}
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
	Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, node_id);
    Cmiss_field_cache_set_node(cache, node);
    Cmiss_nodeset_destroy(&nodeset);

    double values_ps[3];
    Cmiss_field_evaluate_real(coordinates_ps, cache, 3, values_ps);

	Cmiss_node_destroy(&node);

	Cmiss_field_destroy(&inv_projection_mx);
	Cmiss_field_destroy(&coordinates);
	Cmiss_field_destroy(&coordinates_ps);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_field_module_end_change(field_module);
	Cmiss_field_module_destroy(&field_module);

	return Point3D(values_ps);
}

double CAPClientWindow::ComputeHeartVolume(HeartSurfaceEnum surface, double time) const
{
	if (time < 0.0)
		time = Cmiss_time_keeper_get_attribute_real(timeKeeper_, CMISS_TIME_KEEPER_ATTRIBUTE_TIME);

	std::string volume_str = "--";
	double volume = -1.0;
	if (heartModel_ != 0)
	{
		volume = heartModel_->ComputeVolume(surface, time);
		volume_str = ToString(volume);
	}

	if (surface == ENDOCARDIUM)
		SetStatusTextString("heartvolumeendo", "ED Volume(ENDO) = " + volume_str + " ml");
	else if (surface == EPICARDIUM)
		SetStatusTextString("heartvolumeepi", "ED Volume(EPI) = " + volume_str + " ml");

	return volume;
}

}
