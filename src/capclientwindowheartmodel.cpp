

#include <iostream>
#include <sstream>

extern "C"
{
#include <configure/cmgui_configure.h>
#include <api/cmiss_core.h>
#include <api/cmiss_status.h>
#include <api/cmiss_context.h>
#include <api/cmiss_field.h>
#include <api/cmiss_region.h>
#include <api/cmiss_stream.h>
#include <api/cmiss_rendition.h>
#include <api/cmiss_graphic.h>
#include <api/cmiss_field_module.h>
#include <api/cmiss_element.h>
#include <api/cmiss_node.h>
}

#ifdef UnitTestModeller
#include "modellercapclientwindow.h"
#include "modellercapclient.h"
#else
#include "capclientwindow.h"
#include "capclient.h"
#endif
#include "cmgui/extensions.h"
#include "hexified/heartmodel.exnode.h"
#include "hexified/globalhermiteparam.exelem.h"
#include "utils/debug.h"
#include "model/heart.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

namespace cap
{

void CAPClientWindow::CreateHeartModel()
{
	if (heartModel_)
		delete heartModel_;

	heartModel_ = new HeartModel(cmissContext_);
}

void CAPClientWindow::InitialiseHeartModel()
{
	unsigned int minNumberOfFrames = mainApp_->GetMinimumNumberOfFrames();
	
	if (minNumberOfFrames == 0) // Make sure some images have been loaded.
		return;

	assert(heartModel_ == 0);
	heartModel_ = new HeartModel(cmissContext_);
	heartModel_->SetNumberOfModelFrames(minNumberOfFrames);
	LoadTemplateHeartModel(minNumberOfFrames);
	//--SetHeartModelTransformation(heartModel_->GetLocalToGlobalTransformation());
}

void CAPClientWindow::SetHeartModelFocalLength(double focalLength)
{
	std::stringstream ss;
	ss << "coor pro focus " << focalLength;
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "heart");
	Cmiss_field_module_define_field(field_module, "coordinates", ss.str().c_str());
	Cmiss_field_module_destroy(&field_module);
}

void CAPClientWindow::SetHeartModelMuFromBasePlaneAtTime(const Plane& basePlane, double time)
{
	const Vector3D& normal = heartModel_->TransformToLocalCoordinateRC(basePlane.normal);
	const Point3D& position = heartModel_->TransformToLocalCoordinateRC(basePlane.position);
	const int numberOfComponents = 3; // lambda, mu and theta
	
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "heart");
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_id coords_ps = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	Cmiss_field_id coords_rc = Cmiss_field_module_find_field_by_name(field_module, "coordinates_rc");

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_cache_set_time(cache, time);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");

	dbg("Plane : (" + toString(time) + ") " + toString(basePlane.normal) + ", " + toString(basePlane.position));
	// EPI nodes [0-19], ENDO nodes [20-39]
	for (int k = 0; k < 40; k += 20)
	{
		double mu[4];
		for (int i=0; i < 4; i++)
		{
			Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, k + i + 1);
			Cmiss_field_cache_set_node(cache, node);
			double loc[3], loc_ps[3];
			Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
			mu[i] = loc_ps[1];
			Cmiss_field_evaluate_real(coords_rc, cache, 3, loc);
			Point3D point(loc[0],loc[1],loc[2]);
			Point3D prevPoint;
			double initial = DotProduct(normal, point - position);
			//do while on the same side and less than pi
			do
			{
				loc_ps[1] += M_PI/180.0;  //one degree increments for mu parameter
				Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
				mu[i] = loc_ps[1];
				Cmiss_field_evaluate_real(coords_rc, cache, 3, loc);
				prevPoint = point;
				point = Point3D(loc[0],loc[1],loc[2]);
			}
			while((initial*DotProduct(normal, point - position) > 0.0) && (mu[i] < M_PI));
			
			//Cmiss_field_evaluate_real(coords_rc, cache, 3, loc);
			//Point3D lastpoint(loc[0],loc[1],loc[2]);

			double z1 = DotProduct(normal, prevPoint-position);
			double z2 = DotProduct(normal, point-position);
			if((z1*z2) < 0.0) 
			{
				double zdiff = z2-z1;
				double s;
				if(fabs(zdiff)<1.0e-05) s=0.5;
				else s = (-z1)/zdiff;
				point = prevPoint + s*(point-prevPoint);
				loc[0] = point.x;
				loc[1] = point.y;
				loc[2] = point.z;
				Cmiss_field_assign_real(coords_rc, cache, 3, loc);
			}

			Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
			mu[i] = loc_ps[1];
			Cmiss_node_destroy(&node);
			
		}
		dbg("mu [" + toString(0) + "] = " + toString(mu[0]));
		for (int j=1;j<5;j++)
		{
			for (int i=0;i<4;i++)
			{
				Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, k + (j * 4) + i + 1);
				double loc_ps[3];
				Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
				loc_ps[1] = mu[i]/4.0 * (4.0- (double)j);
				Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
				Cmiss_node_destroy(&node);
			}
		}
	}
	Cmiss_field_destroy(&coords_ps);
	Cmiss_field_destroy(&coords_rc);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_module_destroy(&field_module);
}

void CAPClientWindow::SetHeartModelLambdaParamsAtTime(const std::vector<double>& lambdaParams, double time)
{

	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "heart");
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_id coords_ps = Cmiss_field_module_find_field_by_name(field_module, "coordinates");
	//Cmiss_field_id coords_rc = Cmiss_field_module_find_field_by_name(field_module, "coordinates_rc");
	Cmiss_field_id d_ds1 = Cmiss_field_module_find_field_by_name(field_module, "d_ds1");
	Cmiss_field_id d_ds2 = Cmiss_field_module_find_field_by_name(field_module, "d_ds2");
	Cmiss_field_id d2_ds1ds2 = Cmiss_field_module_find_field_by_name(field_module, "d2_ds1ds2");

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_cache_set_time(cache, time);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");

	for (int i = 0; i < 40; i ++) // node index starts at 1
	{
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, i+1);
		Cmiss_field_cache_set_node(cache, node);
		double loc_ps[3];
		Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
		loc_ps[0] = lambdaParams[4 * i + 0];
		Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
		double loc_d_ds1[] = {0.0, 0.0, 0.0};
		loc_d_ds1[0] = lambdaParams[4 * i + 1];
		Cmiss_field_assign_real(d_ds1, cache, 1, loc_d_ds1);
		double loc_d_ds2[] = {0.0, 0.0, 0.0};
		loc_d_ds2[0] = lambdaParams[4 * i + 2];
		Cmiss_field_assign_real(d_ds2, cache, 1, loc_d_ds2);
		double loc_d2_ds1ds2[] = {0.0, 0.0, 0.0};
		loc_d2_ds1ds2[0] = lambdaParams[4 * i + 3];
		Cmiss_field_assign_real(d2_ds1ds2, cache, 1, loc_d2_ds1ds2);
		Cmiss_node_destroy(&node);
	}

	Cmiss_field_destroy(&coords_ps);
	//Cmiss_field_destroy(&coords_rc);
	Cmiss_field_destroy(&d_ds1);
	Cmiss_field_destroy(&d_ds2);
	Cmiss_field_destroy(&d2_ds1ds2);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_module_destroy(&field_module);
}

void CAPClientWindow::SetHeartModelTransformation(const gtMatrix& transform)
{
	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "heart");
	Cmiss_field_module_begin_change(field_module);
	std::stringstream ss_mx;
	ss_mx << "constant ";
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < 3; i++)
		{
			ss_mx << transform[i][j] << " ";
		}
	}

	std::stringstream ss_tr;
	ss_tr << "constant " << transform[3][0] << " " << transform[3][1] << " " << transform[3][2];
	Cmiss_field_module_define_field(field_module, "local_to_global_mx", ss_mx.str().c_str());
	Cmiss_field_module_define_field(field_module, "local_to_global_tr", ss_tr.str().c_str());
	Cmiss_field_module_define_field(field_module, "d_ds1", "node_value fe_field coordinates d/ds1");
	Cmiss_field_module_define_field(field_module, "d_ds2", "node_value fe_field coordinates d/ds2");
	Cmiss_field_module_define_field(field_module, "d2_ds1ds2", "node_value fe_field coordinates d2/ds1ds2");
	Cmiss_field_module_define_field(field_module, "coordinates_rc", "coordinate_transformation field coordinates");
	Cmiss_field_module_define_field(field_module, "temp1", "matrix_multiply num 3 fields local_to_global_mx coordinates_rc");
	Cmiss_field_module_define_field(field_module, "patient_rc_coordinates", "add fields temp1 local_to_global_tr");

	Cmiss_field_module_end_change(field_module);

	Cmiss_field_module_destroy(&field_module);
}

void CAPClientWindow::LoadTemplateHeartModel(unsigned int numberOfModelFrames)
{
	heartModel_->SetNumberOfModelFrames(numberOfModelFrames);

	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	// Create a heart region to stop Cmgui adding lines to the new rendition
	Cmiss_region_id heart_region = Cmiss_region_create_child(root_region, "heart");
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
	{
		Cmiss_stream_information_id stream_information = Cmiss_region_create_stream_information(root_region);
		Cmiss_stream_information_region_id stream_information_region = Cmiss_stream_information_cast_region(stream_information);
		Cmiss_stream_resource_id stream_resource = Cmiss_stream_information_create_resource_memory_buffer(stream_information, heartmodel_exnode, heartmodel_exnode_len);
		int r = Cmiss_stream_information_region_set_resource_attribute_real(stream_information_region, stream_resource, CMISS_STREAM_INFORMATION_REGION_ATTRIBUTE_TIME, 1.0);

		Cmiss_region_read(root_region, stream_information);

		Cmiss_stream_resource_destroy(&stream_resource);
		Cmiss_stream_information_destroy(&stream_information);
		Cmiss_stream_information_region_destroy(&stream_information_region);
	}

	Cmiss_region_destroy(&root_region);
}

void CAPClientWindow::LoadHeartModel(std::string fullExelemFileName, std::vector<std::string> fullExnodeFileNames)
{
	Cmiss_region_id root_region = Cmiss_context_get_default_region(cmissContext_);
	// Create a heart region to stop Cmgui adding lines to the new rendition
	Cmiss_region_id heart_region = Cmiss_region_create_child(root_region, "heart");
	Cmiss_region_destroy(&heart_region);

	LoadHermiteHeartElements(fullExelemFileName);

	unsigned int numberOfModelFrames = fullExnodeFileNames.size();
	heartModel_->SetNumberOfModelFrames(numberOfModelFrames);

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

double CAPClientWindow::ComputeHeartVolume(SurfaceType surface, double time) const
{
	//dbg("CAPClientWindow::ComputeHeartVolume");

	const int numElements = 16;
	const int nx = 7, ny = 7;

	Point3D b[numElements][nx];
	Point3D p[numElements*nx*ny];
	Point3D temp;
	double vol_sum = 0;
	Point3D origin(0,0,0);

	// initialise arrays
	for (int ne=0;ne<numElements;ne++)
	{
		for(int i=0;i<nx;i++)
		{
			b[ne][i] = Point3D(0,0,0);
		}
	}

	Cmiss_field_module_id field_module = Cmiss_context_get_field_module_for_region(cmissContext_, "heart");
	Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_id rc_coordinate_field = Cmiss_field_module_find_field_by_name(field_module, "patient_rc_coordinates");
	//struct CM_element_information identifier;
	//identifier.type = CM_ELEMENT;
	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, 3);
	Cmiss_element_iterator_id element_iterator = Cmiss_mesh_create_element_iterator(mesh);
	Cmiss_element_id element = Cmiss_element_iterator_next(element_iterator);
	//do for all elements
	while (element != 0)
	{
		//calculate vertex coordinates for element subdivision
		for (int i=0;i<nx;i++)
		{
			for (int j=0;j<ny;j++)
			{
				//calculate lamda mu and theta at this point
				//--double values[3], xi[3];
				double xi[3], values[3];
				xi[0] = (double) i/(nx-1);
				xi[1] = (double) j/(ny-1);
				xi[2] = (surface == ENDOCARDIUM) ? 0.0f : 1.0f;

				Cmiss_field_cache_set_mesh_location(field_cache, element, 3, xi);

				Cmiss_field_evaluate_real(rc_coordinate_field, field_cache, 3, values);

				p[(j*nx+i)] = Point3D(values);
			} // j
		} // I
		
		//do for all quads
		//note vertices must be ordered ccw viewed from outside
		for(int i=0;i<nx-1;i++){
			for(int j=0;j<ny-1;j++){
				int n1 = j*(nx) + i ;
				int n2 = n1 + 1;
				int n3 = n2 + (nx);
				double vol = ComputeVolumeOfTetrahedron(p[n1],p[n2],p[n3],origin);
				vol_sum += vol;

				n1 = n1;
				n2 = n3;
				n3 = n2-1;
				vol = ComputeVolumeOfTetrahedron(p[n1],p[n2],p[n3],origin);
				vol_sum += vol;
			} /* j */
		} /* i */
		//store the base ring
		int ne = Cmiss_element_get_identifier(element);
		if( ne < 4)
		{
			for(int k=0;k<nx;k++)
			{
				b[ne][k] = p[(ny-1)*nx + k];
			}
		}
		Cmiss_element_destroy(&element);
		element = Cmiss_element_iterator_next(element_iterator);
	}

	Cmiss_element_iterator_destroy(&element_iterator);
	Cmiss_field_cache_destroy(&field_cache);
	Cmiss_field_destroy(&rc_coordinate_field);
	Cmiss_field_module_destroy(&field_module);
	/* now close the top */
	/* find centroid of the top ring */
	int num=0;
	Point3D c(0,0,0);
	for(int ne=0;ne<4;ne++){
		for(int i=0;i<nx;i++){
			c += b[ne][i];
			num++;
		}
	}
	c *= (1/(double)num);
	for(int ne=0;ne<4;ne++){
		for(int i=0;i<nx-1;i++){
			int n1 = i ;
			int n2 = n1+1;
			double vol = ComputeVolumeOfTetrahedron(b[ne][n1], b[ne][n2], c, origin);
			vol_sum += vol;
		}
	}

	if (surface == ENDOCARDIUM)
		SetStatusTextString("heartvolumeendo", "ED Volume(ENDO) = " + toString(vol_sum/6000.0) + " ml");
	else
		SetStatusTextString("heartvolumeepi", "ED Volume(EPI) = " + toString(vol_sum/6000.0) + " ml");

	return (vol_sum/6000.0);
	// (6*1000), 6 times volume of tetrahedron & for ml
}

}