/*
 * HeartModel.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#include "model/heart.h"

//#include "zinc/sceneviewerpanel.h"
#include "math/algebra.h"
#include "zinc/extensions.h"
#include "utils/misc.h"
#include "utils/debug.h"

extern "C" {
#include <zn/cmiss_field.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_field_composite.h>
#include <zn/cmiss_field_matrix_operators.h>
#include <zn/cmiss_field_finite_element.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_element.h>
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace cap
{

class HeartModel::HeartModelImpl
{
public:
	explicit HeartModelImpl(Cmiss_context_id context)
		: context_(Cmiss_context_access(context))
		, region_(0)
		, field_(0)
		, field_module_(0)
		, coordinates_ps_(0)
		, coordinates_rc_(0)
		, coordinates_patient_rc_(0)
		, transform_mx_(0)
		, epi_surface_(0)
		, endo_surface_(0)
	{}
	
	Cmiss_context_id context_;
	Cmiss_region_id region_;
	//Scene_object* sceneObject;
	Cmiss_field_id field_;
	Cmiss_field_module_id field_module_;
	Cmiss_field_id coordinates_ps_;
	Cmiss_field_id coordinates_rc_;
	Cmiss_field_id coordinates_patient_rc_;
	Cmiss_field_id transform_mx_;
	Cmiss_graphic_id epi_surface_;
	Cmiss_graphic_id endo_surface_;
};

HeartModel::HeartModel(Cmiss_context_id context)
	: modelName_("heart")
	, pImpl_(new HeartModel::HeartModelImpl(context))
{
}

HeartModel::~HeartModel()
{
	Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(pImpl_->context_, modelName_.c_str());
	if(rendition != 0 && pImpl_->epi_surface_ != 0)
	{
		Cmiss_rendition_remove_graphic(rendition, pImpl_->epi_surface_);
		Cmiss_graphic_destroy(&(pImpl_->epi_surface_));
	}
	if(rendition != 0 && pImpl_->endo_surface_ != 0)
	{
		Cmiss_rendition_remove_graphic(rendition, pImpl_->endo_surface_);
		Cmiss_graphic_destroy(&(pImpl_->endo_surface_));
	}
    Cmiss_rendition_destroy(&rendition);
    Cmiss_field_destroy(&(pImpl_->coordinates_ps_));
	Cmiss_field_destroy(&(pImpl_->coordinates_rc_));
	Cmiss_field_destroy(&(pImpl_->coordinates_patient_rc_));
	Cmiss_field_destroy(&(pImpl_->transform_mx_));
	Cmiss_field_module_destroy(&(pImpl_->field_module_));
	Cmiss_context_destroy(&(pImpl_->context_));
	delete pImpl_;
}

void HeartModel::Initialise(const std::string& name)
{
	modelName_ = name;
	// initialize patientToGlobalTransform_ to identity matrix
	double mx[16];
	for (int i = 0; i < 4 ;++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (i == j)
			{
				patientToGlobalTransform_[i][j] = 1.0;
			}
			else
			{
				patientToGlobalTransform_[i][j] = 0.0;
			}
			mx[4*i+j] = patientToGlobalTransform_[i][j];
		}
	}
	pImpl_->field_module_ = Cmiss_context_get_field_module_for_region(pImpl_->context_, modelName_.c_str());
	if (pImpl_->field_module_ != 0)
	{
		Cmiss_field_module_begin_change(pImpl_->field_module_);

		// 'coordinates' is an assumed field from the element template file.  It is also assumed to be
		// a prolate spheriodal coordinate system.
		pImpl_->coordinates_ps_ = Cmiss_field_module_find_field_by_name(pImpl_->field_module_, "coordinates");

		// Cannot make all these fields via the Cmgui API yet.
		Cmiss_field_module_define_field(pImpl_->field_module_, "d_ds1", "node_value fe_field coordinates d/ds1");
		Cmiss_field_module_define_field(pImpl_->field_module_, "d_ds2", "node_value fe_field coordinates d/ds2");
		Cmiss_field_module_define_field(pImpl_->field_module_, "d2_ds1ds2", "node_value fe_field coordinates d2/ds1ds2");
		Cmiss_field_module_define_field(pImpl_->field_module_, "coordinates_rc", "coordinate_transformation field coordinates");
		pImpl_->coordinates_rc_ = Cmiss_field_module_find_field_by_name(pImpl_->field_module_, "coordinates_rc");
		pImpl_->transform_mx_ = Cmiss_field_module_create_constant(pImpl_->field_module_, 16, mx);
		pImpl_->coordinates_patient_rc_ = Cmiss_field_module_create_projection(pImpl_->field_module_, pImpl_->coordinates_rc_, pImpl_->transform_mx_);
		Cmiss_field_set_name(pImpl_->coordinates_patient_rc_, "coordinates_patient_rc");

		Cmiss_field_module_end_change(pImpl_->field_module_);
	}
	else
		dbg("--- No field module for heart region!!!");

	Cmiss_rendition_id rendition = Cmiss_context_get_rendition_for_region(pImpl_->context_, modelName_.c_str());
	if (rendition != 0)
	{
		// Match the initial state of the render type for the surface with that shown in the gui combo box.
		pImpl_->epi_surface_ = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_SURFACES);
        Cmiss_graphic_define(pImpl_->epi_surface_, "coordinate coordinates_patient_rc exterior face xi3_0 no_select material green_surface render_wireframe");
		pImpl_->endo_surface_ = Cmiss_rendition_create_graphic(rendition, CMISS_GRAPHIC_SURFACES);
        Cmiss_graphic_define(pImpl_->endo_surface_, "coordinate coordinates_patient_rc exterior face xi3_1 no_select material red_surface render_wireframe");
		Cmiss_rendition_destroy(&rendition);
	}
	else
		dbg("--- No rendition for heart region!!!");
}

void HeartModel::SetRenderMode(RenderMode mode)
{
	if (mode == WIREFRAME)
	{
		Cmiss_graphic_define(pImpl_->epi_surface_, "render_wireframe;");
		Cmiss_graphic_define(pImpl_->endo_surface_, "render_wireframe");
	}
	else if (mode == SHADED)
	{
		Cmiss_graphic_define(pImpl_->epi_surface_, "render_shaded");
		Cmiss_graphic_define(pImpl_->endo_surface_, "render_shaded");
	}
}

void HeartModel::SetVisibility(bool visible)
{
	Cmiss_graphic_set_visibility_flag(pImpl_->epi_surface_, visible ? 1 : 0);
	Cmiss_graphic_set_visibility_flag(pImpl_->endo_surface_, visible ? 1 : 0);
}

bool HeartModel::IsVisible() const
{
    return Cmiss_graphic_get_visibility_flag(pImpl_->epi_surface_) == 1;
}

void HeartModel::SetLocalToGlobalTransformation(const gtMatrix& transform)
{
	double mx[16];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// gtMatrix is column-major and Cmgui wants Row-major we transpose the matrix here.
			mx[i+4*j] = patientToGlobalTransform_[i][j] = transform[i][j];
		}
	}

	Cmiss_field_module_begin_change(pImpl_->field_module_);
	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(pImpl_->field_module_);
	Cmiss_field_assign_real(pImpl_->transform_mx_, cache, 16, mx);
	Cmiss_field_cache_destroy(&cache);
	Cmiss_field_module_end_change(pImpl_->field_module_);
}

int HeartModel::ComputeXi(const Point3D& position, double time, Point3D& xi) const
{
	Cmiss_field_module_id field_module = pImpl_->field_module_;
	Cmiss_field_module_begin_change(field_module);

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_cache_set_time(cache, time);
	double values[3];
	values[0] = position.x;
	values[1] = position.y;
	values[2] = position.z;
	Cmiss_field_id const_position = Cmiss_field_module_create_constant(field_module, 3, values);
	Cmiss_mesh_id mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, 3);
	Cmiss_field_id mesh_location_field = Cmiss_field_module_create_find_mesh_location(field_module, const_position, pImpl_->coordinates_patient_rc_, mesh);
	Cmiss_field_find_mesh_location_id find_mesh_location_field = Cmiss_field_cast_find_mesh_location(mesh_location_field);
    Cmiss_field_find_mesh_location_set_search_mode(find_mesh_location_field, CMISS_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST);
	Cmiss_field_find_mesh_location_destroy(&find_mesh_location_field);

	double xi_values[3];
	Cmiss_element_id el = Cmiss_field_evaluate_mesh_location(mesh_location_field, cache, 3, xi_values);
	xi.x = xi_values[0];
	xi.y = xi_values[1];
	xi.z = xi_values[2];
	int element_id = Cmiss_element_get_identifier(el);
	//dbg("element : " + ToString(element_id) + " xi [ " + ToString(xi) + " ]");
	Cmiss_element_destroy(&el);

	Cmiss_field_module_end_change(field_module);

	Cmiss_mesh_destroy(&mesh);
	Cmiss_field_destroy(&mesh_location_field);
	Cmiss_field_destroy(&const_position);
	Cmiss_field_cache_destroy(&cache);

	return element_id;
}
const std::vector<double> HeartModel::GetLambdaAtTime(double time) const
{
    Cmiss_field_module_id field_module = pImpl_->field_module_;
    Cmiss_field_id coords_ps = pImpl_->coordinates_ps_;

    Cmiss_field_module_begin_change(field_module);

    Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
    Cmiss_field_cache_set_time(cache, time);
    Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");


    std::vector<double> lambdas;
    for (int i = 0; i < 40; i++) // node index starts at 1
    {
        Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, i+1);
        Cmiss_field_cache_set_node(cache, node);
        double loc_ps[3];
        Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
        lambdas.push_back(loc_ps[0]);
        Cmiss_node_destroy(&node);
    }

    Cmiss_field_cache_destroy(&cache);
    Cmiss_nodeset_destroy(&nodeset);

    Cmiss_field_module_end_change(field_module);

    return lambdas;
}

const std::vector<double> HeartModel::GetMuAtTime(double time) const
{
    Cmiss_field_module_id field_module = pImpl_->field_module_;
    Cmiss_field_id coords_ps = pImpl_->coordinates_ps_;

    Cmiss_field_module_begin_change(field_module);

    Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
    Cmiss_field_cache_set_time(cache, time);
    Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");


    std::vector<double> mus;
    for (int i = 0; i < 40; i++) // node index starts at 1
    {
        Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, i+1);
        Cmiss_field_cache_set_node(cache, node);
        double loc_ps[3];
        Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
        mus.push_back(loc_ps[1]);
        Cmiss_node_destroy(&node);
    }

    Cmiss_field_cache_destroy(&cache);
    Cmiss_nodeset_destroy(&nodeset);

    Cmiss_field_module_end_change(field_module);

    return mus;
}

void HeartModel::SetLambdaAtTime(const std::vector<double>& lambdaParams, double time)
{
	Cmiss_field_module_id field_module = pImpl_->field_module_;
	Cmiss_field_module_begin_change(field_module);
	Cmiss_field_id coords_ps = pImpl_->coordinates_ps_;
	Cmiss_field_id d_ds1 = Cmiss_field_module_find_field_by_name(field_module, "d_ds1");
	Cmiss_field_id d_ds2 = Cmiss_field_module_find_field_by_name(field_module, "d_ds2");
	Cmiss_field_id d2_ds1ds2 = Cmiss_field_module_find_field_by_name(field_module, "d2_ds1ds2");

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(field_module);
	Cmiss_field_cache_set_time(cache, time);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");

	for (int i = 0; i < 40; i++) // node index starts at 1
	{
		Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, i+1);
        Cmiss_field_cache_set_node(cache, node);
        double loc_ps[3];
		Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
        loc_ps[0] = lambdaParams[4 * i + 0];
		//if (i == 11)
		//	dbg("node loc 12 : [" + ToString(loc_ps[0]) + ", " + ToString(loc_ps[1]) + ", " + ToString(loc_ps[2]) + "]");
		Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
		double loc_d_ds1[] = {0.0, 0.0, 0.0};
		loc_d_ds1[0] = lambdaParams[4 * i + 1];
		Cmiss_field_assign_real(d_ds1, cache, 3, loc_d_ds1);
		double loc_d_ds2[] = {0.0, 0.0, 0.0};
		loc_d_ds2[0] = lambdaParams[4 * i + 2];
		Cmiss_field_assign_real(d_ds2, cache, 3, loc_d_ds2);
		double loc_d2_ds1ds2[] = {0.0, 0.0, 0.0};
		loc_d2_ds1ds2[0] = lambdaParams[4 * i + 3];
		Cmiss_field_assign_real(d2_ds1ds2, cache, 3, loc_d2_ds1ds2);
		Cmiss_node_destroy(&node);
	}
	Cmiss_field_destroy(&d_ds1);
	Cmiss_field_destroy(&d_ds2);
	Cmiss_field_destroy(&d2_ds1ds2);
	Cmiss_field_module_end_change(field_module);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_nodeset_destroy(&nodeset);
}

void HeartModel::SetMuFromBasePlaneAtTime(const Plane& basePlane, double time)
{
	const Vector3D& normal = basePlane.normal;//--heartModel_->TransformToLocalCoordinateRC(basePlane.normal);
	const Point3D& position = basePlane.position;//--heartModel_->TransformToLocalCoordinateRC(basePlane.position);
    //const int numberOfComponents = 3; // lambda, mu and theta
	
	Cmiss_field_module_begin_change(pImpl_->field_module_);
	Cmiss_field_id coords_ps = pImpl_->coordinates_ps_;
	Cmiss_field_id coords_patient = pImpl_->coordinates_patient_rc_;

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(pImpl_->field_module_);
	Cmiss_field_cache_set_time(cache, time);
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_name(pImpl_->field_module_, "cmiss_nodes");

    //dbg("Plane : (" + ToString(time) + ") " + ToString(basePlane.normal) + ", " + ToString(basePlane.position));
	// EPI nodes [0-19], ENDO nodes [20-39], node identifiers are 1-based.
	// This method follows the CIM method of calculating the model position from the base plane.
	for (int k = 0; k < 40; k += 20)
	{
		double mu[4];
		for (int i=0; i < 4; i++)
		{
			Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, k + i + 1);
            Cmiss_field_cache_set_node(cache, node);
            double loc[3], loc_ps[3], loc_pat[3];
            Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
            mu[i] = loc_ps[1] = 0.0;
            Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
            //r1 = Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
            Cmiss_field_evaluate_real(coords_patient, cache, 3, loc_pat);
			Point3D point(loc_pat[0],loc_pat[1],loc_pat[2]);
			Point3D prevPoint;
			double initial = DotProduct(normal, point - position);
			//do while on the same side and less than pi
			do
			{
				loc_ps[1] += M_PI/180.0;  //one degree increments for mu parameter
                Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
				mu[i] = loc_ps[1];
                Cmiss_field_evaluate_real(coords_patient, cache, 3, loc_pat);
				prevPoint = point;
				point = Point3D(loc_pat[0],loc_pat[1],loc_pat[2]);
			}
			while((initial*DotProduct(normal, point - position) > 0.0) && (mu[i] < M_PI));
			
			// We have stepped over the base plane (or we have reached pi), now interpolate between the 
			// current point and the previous point
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
				Cmiss_field_assign_real(coords_patient, cache, 3, loc);
			}

			// Just making sure we keep mu inside it's bounds.
			Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
			mu[i] = loc_ps[1];
			if (mu[i] > M_PI)
			{
				mu[i] = M_PI;
				loc_ps[1] = M_PI;
			}
			Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
			Cmiss_node_destroy(&node);
			
		}

		// Set the remaining mu paramater of the nodes equidistant between mu[i] and 0.0
		for (int j=1;j<5;j++)
		{
			for (int i=0;i<4;i++)
			{
				Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset, k + (j * 4) + i + 1);
                Cmiss_field_cache_set_node(cache, node);
                double loc_ps[3];
				Cmiss_field_evaluate_real(coords_ps, cache, 3, loc_ps);
                loc_ps[1] = mu[i]/4.0 * (4.0- static_cast<double>(j));
				Cmiss_field_assign_real(coords_ps, cache, 3, loc_ps);
				Cmiss_node_destroy(&node);
			}
		}
	}
	Cmiss_field_module_end_change(pImpl_->field_module_);

	Cmiss_field_cache_destroy(&cache);
	Cmiss_nodeset_destroy(&nodeset);
}
	
double HeartModel::ComputeVolume(HeartSurfaceEnum surface, double time) const
{
	const int numElements = 16;
	const int nx = 7, ny = 7;

	Point3D b[numElements][nx];
	Point3D p[numElements*nx*ny];
	Point3D temp;
	double vol_sum = 0.0;
	Point3D origin(0,0,0);

	// initialise arrays
	for (int ne=0;ne<numElements;ne++)
	{
		for(int i=0;i<nx;i++)
		{
			b[ne][i] = Point3D(0,0,0);
		}
	}

	Cmiss_field_module_id field_module = pImpl_->field_module_;
	if (field_module != 0)
	{
		Cmiss_field_cache_id field_cache = Cmiss_field_module_create_cache(field_module);
        Cmiss_field_cache_set_time(field_cache, time);
		Cmiss_field_id rc_coordinate_field = pImpl_->coordinates_patient_rc_;
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
	}

	// (6*1000), 6 times volume of tetrahedron & for ml
	return (vol_sum/6000.0);
}

void HeartModel::SetFocalLength(double focalLength)
{
	Cmiss_field_set_attribute_real(pImpl_->coordinates_ps_, CMISS_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS, focalLength);
}

double HeartModel::GetFocalLength() const
{
	return Cmiss_field_get_attribute_real(pImpl_->coordinates_ps_, CMISS_FIELD_ATTRIBUTE_COORDINATE_SYSTEM_FOCUS);
}

} // end namespace cap
