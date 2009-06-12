/*
 * CAPModelLVPS4X4.cpp
 *
 *  Created on: Feb 17, 2009
 *      Author: jchu014
 */

#include "Config.h" //should go to header?
#include "CAPModelLVPS4X4.h"
#include "CmguiManager.h"
#include "CmguiExtensions.h"
#include "CAPMath.h"

extern "C" {
#include "command/cmiss.h"
#include "api/cmiss_region.h"
#include "graphics/scene_viewer.h"
#include "time/time_keeper.h"
#include "time/time.h"
#include "graphics/scene.h"
#include "computed_field/computed_field_finite_element.h"
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


struct CAPModelLVPS4X4::HeartModelImpl
{
	Cmiss_command_data* commandData;
	Cmiss_region* region;
	//Scene_object* sceneObject;
	Cmiss_field_id field;
};

CAPModelLVPS4X4::CAPModelLVPS4X4(const std::string& modelName)
:
	modelName_(modelName),
	focalLength_(42.0), // FIX magic number
	pImpl_(new CAPModelLVPS4X4::HeartModelImpl)
{}

CAPModelLVPS4X4::~CAPModelLVPS4X4()
{
	delete pImpl_;
}

using namespace std;

int CAPModelLVPS4X4::ReadModelFromFiles(const std::string& path)
{	
	stringstream pathStream;	
	pathStream << prefix << path << "/";// << modelName << "_";// << 
	string dir_path = pathStream.str();

	ReadModelInfo(dir_path); // this will set numberOfModelFrames and transformation Matrix

	pImpl_->commandData = CmguiManager::getInstance().getCmissCommandData();

	Cmiss_region* region = Cmiss_command_data_get_root_region(pImpl_->commandData);
	struct Time_keeper* time_keeper = Cmiss_command_data_get_default_time_keeper(pImpl_->commandData);
	
	for (int i = 0; i<numberOfModelFrames_; i++)
	{		
		stringstream filenameStream;
		filenameStream << dir_path << path << "_" << i+1 << ".model.exnode" ;
		
		// lifetime of temporaries bound to a reference = lifetime of the reference 
		// note that temporaries can only be bound to const references !
		const string& filenameString = filenameStream.str();
		// Note that with RVO, the above statement is the same as
		// string filenameString = filenameStream.str(); 
		
		char* filename = const_cast<char*>(filenameString.c_str());
		//DEBUG
		//cout << "DEBUG: i = " << i << ", filename = " << filename << endl;
		float time = static_cast<float>(i)/numberOfModelFrames_;
		//std::cout << "time = " << time << endl;
		if (!Cmiss_region_read_file_with_time(region,filename,time_keeper,time))
		{
			std::cout << "Error reading ex file: " << filename << std::endl;
		}
	}

	string exelem_filename(prefix);
	exelem_filename.append("templates/GlobalHermiteParam.exelem");
	if (!Cmiss_region_read_file(region, const_cast<char*>(exelem_filename.c_str())))
	{
		std::cout << "Error reading ex file - exelem" << std::endl;
	}
	
	//Transform the heart model to world coordinate

	char* scene_object_name = const_cast<char*>(modelName_.c_str()); // temporarily remove constness to be compatible with Cmgui

	if (scene_object_name)
	{
		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(pImpl_->commandData);
		struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		if (modelSceneObject_=Scene_get_Scene_object_by_name(scene,
			scene_object_name))
		{
			//Scene_object_remove_time_dependent_transformation(scene_object);//??????? Why need time dependent transformation??
			Scene_object_set_transformation(modelSceneObject_, &patientToGlobalTransform_);
		}
		else
		{
			display_message(ERROR_MESSAGE,"No object named '%s' in scene",scene_object_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Missing graphics object name");
	}

	// Set focal length from the value read in
	Cmiss_region* cmiss_region;
	Cmiss_region_get_region_from_path(region, "heart", &cmiss_region);
	pImpl_->region = cmiss_region;
	Cmiss_field_id field = Cmiss_region_find_field_by_name(cmiss_region, "coordinates");//FIX
	pImpl_->field = field;
	struct Coordinate_system* coordinate_system = Computed_field_get_coordinate_system(field);
	focalLength_ = coordinate_system->parameters.focus;
	
	// define the coord field in RC for MII computation
	// FIX use API calls instead of command line
	// TODO extract method
	char str[256];
	sprintf((char*)str, "gfx define field heart_rc_coord coordinate_system rectangular_cartesian coordinate_transformation field coordinates;");
	Cmiss_command_data_execute_command(pImpl_->commandData, str);

	
	// set the discretizatioin level to be 6 
	// FIX use API calls instead of command line
	// NB This resets the time object associated with the scene object ?? WHY???? possible BUG??
	// NB This maybe because of "clear" in the command - replace with API call GT_element_group_set_element_discretization
	Cmiss_command_data_execute_command(pImpl_->commandData,
	"gfx modify g_element heart general clear circle_discretization 6 default_coordinate coordinates element_discretization \"6*6*6\" native_discretization none;"	
	);//This clears the timer frequency
	
	SetRenderMode(CAPModelLVPS4X4::WIREFRAME); // Since it was cleared, this sets up frequency 
															// but incorrectly at default freq = 10 hz
	
	// Set the timer frequency : NB this has to be done after setting disc & creating surfaces level See above 
	Cmiss_time_notifier* timeNotifier = Scene_object_get_time_object(modelSceneObject_);
	if (timeNotifier) {
		Cmiss_time_notifier_regular_set_frequency(timeNotifier, numberOfModelFrames_);
	}
	
	return 0;
}

void CAPModelLVPS4X4::ReadModelInfo(std::string modelInfoFilePath)
{
	modelInfoFilePath.append("ModelInfo.txt");
	ifstream modelInfoFile(modelInfoFilePath.c_str());
	
	if (!modelInfoFile.is_open())
	{
		cout << "Can't open ModelInfo.txt - " << modelInfoFilePath << endl;
		return; // should throw?
	}
	string line;
	
	getline(modelInfoFile, line); // NumberOfModelFrames:
	cout << line << endl;
	
	modelInfoFile >> numberOfModelFrames_;
	cout << numberOfModelFrames_ <<endl;
	getline(modelInfoFile, line); //the rest of the line
	
	// How to get transformation matrix from the basis vectors of and the translation of the origin
	//
	// ii ji ki  0
	// ij jj kj  0    *  translation
	// ik jk kk  0
	//  0  0  0  1
	// from http://physics.usask.ca/~chang/phys323/notes/lecture1.pdf
	
	// Read in model to world coordinate transformation

	getline(modelInfoFile, line); //empty line
	cout << line << endl;	
	getline(modelInfoFile, line); //ModelToMagnetTransform:
	cout << line << endl;	
	
	//Point3D i_hat, j_hat, k_hat, translation; //needed for model transformation to world coord
	//i_hat, j_hat, k_hat =  model coord basis vectors in i, j & k directions
	//translation = origin translation
	
	Point3D i_hat, j_hat, k_hat, translation;
	modelInfoFile >> i_hat >> j_hat >> k_hat >> translation;
	
	cout << "a1 = " << i_hat <<endl;
	cout << "a2 = " << j_hat <<endl;
	cout << "a3 = " << k_hat <<endl;
	cout << "t  = " << translation <<endl;
	
	Point3D i(1,0,0), j(0,1,0), k(0,0,1); // world coord basis vectors
	gtMatrix temp;
	temp[0][0] = DotProduct(i,i_hat);
	temp[0][1] = DotProduct(j,i_hat);
	temp[0][2] = DotProduct(k,i_hat);

	temp[1][0] = DotProduct(i,j_hat);
	temp[1][1] = DotProduct(j,j_hat);
	temp[1][2] = DotProduct(k,j_hat);

	temp[2][0] = DotProduct(i,k_hat);
	temp[2][1] = DotProduct(j,k_hat);
	temp[2][2] = DotProduct(k,k_hat);

	temp[3][0] = translation.x;
	temp[3][1] = translation.y;
	temp[3][2] = translation.z;

	patientToGlobalTransform_[0][0]=temp[0][0];
	patientToGlobalTransform_[0][1]=temp[0][1];
	patientToGlobalTransform_[0][2]=temp[0][2];
	patientToGlobalTransform_[0][3]=0; //NB this is the first column not row
	patientToGlobalTransform_[1][0]=temp[1][0];
	patientToGlobalTransform_[1][1]=temp[1][1];
	patientToGlobalTransform_[1][2]=temp[1][2];
	patientToGlobalTransform_[1][3]=0;
	patientToGlobalTransform_[2][0]=temp[2][0];
	patientToGlobalTransform_[2][1]=temp[2][1];
	patientToGlobalTransform_[2][2]=temp[2][2];
	patientToGlobalTransform_[2][3]=0;
	patientToGlobalTransform_[3][0]=temp[3][0];
	patientToGlobalTransform_[3][1]=temp[3][1];
	patientToGlobalTransform_[3][2]=temp[3][2];
	patientToGlobalTransform_[3][3]=1;
	
	return;
}

void CAPModelLVPS4X4::SetLocalToGlobalTransformation(const gtMatrix& transform)
{
	for (int i = 0; i<4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			patientToGlobalTransform_[i][j] = transform[i][j];//REVISE use a wrapper class
		}
	}
	
	Scene_object_set_transformation(modelSceneObject_, &patientToGlobalTransform_);
}

void CAPModelLVPS4X4::SetRenderMode(RenderMode mode)
{
	if (mode == CAPModelLVPS4X4::WIREFRAME)
	{
		Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
		
		//FIX use api calls
		Cmiss_command_data_execute_command(command_data,
		"gfx mod g_el heart surfaces exterior face xi3_0 no_select material green selected_material default_selected render_wireframe;"
		);
		
		Cmiss_command_data_execute_command(command_data,
		"gfx mod g_el heart surfaces exterior face xi3_1 no_select material red selected_material default_selected render_wireframe;"
		);
		
	}
}

void CAPModelLVPS4X4::SetMIIVisibility(bool visibility)
{
	GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(modelSceneObject_);
	
	int numSettings = GT_element_group_get_number_of_settings(gt_element_group);
	
	int visible = visibility? 1:0;
	
	for (int i = 3; i< (numSettings+1) ; i++) // FIX magic numbers
	{
		GT_element_settings* settings = get_settings_at_position_in_GT_element_group(gt_element_group,i);
		if (!settings)
		{
			cout << "Can't find settings by position" << endl;
		}
		GT_element_settings_set_visibility(settings, visible);
	}
	GT_element_group_modify(gt_element_group, gt_element_group);
}

void CAPModelLVPS4X4::SetMIIVisibility(bool visibility, int index)
{
	GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(modelSceneObject_);
	
	int numSettings = GT_element_group_get_number_of_settings(gt_element_group);
	
	int visible = visibility? 1:0;
	
	const int indexOffset = 3;
	GT_element_settings* settings = get_settings_at_position_in_GT_element_group(gt_element_group,index + indexOffset);
	if (!settings)
	{
		cout << "Can't find settings by position" << endl;
		assert(settings);
	}
	GT_element_settings_set_visibility(settings, visible);

	GT_element_group_modify(gt_element_group, gt_element_group);
}

void CAPModelLVPS4X4::SetModelVisibility(bool visibility)
{

	GT_element_group* gt_element_group = Scene_object_get_graphical_element_group(modelSceneObject_);
	
//	int numSettings = GT_element_group_get_number_of_settings(gt_element_group);
//	cout <<  numSettings << endl;
	
	int visible = visibility? 1:0;
	
	for (int i = 1; i < 3 ; i++) // FIX magic numbers
	{
		GT_element_settings* settings = get_settings_at_position_in_GT_element_group(gt_element_group,i);
		if (!settings)
		{
			cout << "Can't find GT element settings by position" << endl;
		}
		GT_element_settings_set_visibility(settings, visible);
	}
	GT_element_group_modify(gt_element_group, gt_element_group);
}

Point3D CAPModelLVPS4X4::TransformToLocalCoordinateRC(const Point3D& global) const
{
	// FIX inefficient to compute mInv every time
	gtMatrix mInv;
	inverseMatrix(patientToGlobalTransform_, mInv);
	transposeMatrix(mInv);// gtMatrix is column Major and our matrix functions assume row major FIX
	
	return mInv * global; // hopefully RVO will kick in
}

Vector3D CAPModelLVPS4X4::TransformToLocalCoordinateRC(const Vector3D& global) const
{
	// FIX inefficient to compute mInv every time
	gtMatrix mInv;
	inverseMatrix(patientToGlobalTransform_, mInv);
	transposeMatrix(mInv);// gtMatrix is column Major and our matrix functions assume row major FIX
	
	return mInv * global; // Vector transformation - doesn't include translation
}

Point3D CAPModelLVPS4X4::TransformToProlateSheroidal(const Point3D& rc) const
{
	float lambda, mu, theta;
		
	cartesian_to_prolate_spheroidal(rc.x, rc.y, rc.z, focalLength_, &lambda, &mu, &theta,0);
	//cout << "lambda: " << lambda << ", mu: " << mu << ", theta: " << theta << ", focalLength = " << focalLength_ << endl;
	
	return Point3D(lambda, mu, theta);
}

int CAPModelLVPS4X4::ComputeXi(const Point3D& coord, Point3D& xi_coord, float time) const
{
	//1. Transform to model coordinate 	
	const Point3D& coordLocal = TransformToLocalCoordinateRC(coord);
	
//	cout << "Local coord = " << coordLocal << endl;
	
	//2. Transform to Prolate Spheroidal
	float lambda, mu, theta;
	
	cartesian_to_prolate_spheroidal(coordLocal.x,coordLocal.y,coordLocal.z, focalLength_, 
			&lambda,&mu, &theta,0);
//	cout << "lambda: " << lambda << ", mu: " << mu << ", theta: " << theta << ", focalLength = " << focalLength_ << endl;
	
	//3. Project on to model surface and obtain the material coordinates
	Cmiss_region* cmiss_region = pImpl_->region;
	
	Cmiss_field_id field = Cmiss_region_find_field_by_name(cmiss_region, "heart_rc_coord");//FIX
	
	FE_value point[3], xi[3];
//	point[0] = lambda, point[1] = mu, point[2] = theta;
	point[0] = coordLocal.x, point[1] = coordLocal.y, point[2] = coordLocal.z;
	FE_element* element = 0;
//	int return_code = Computed_field_find_element_xi(field,
//		point, /*number_of_values*/3, &element /*FE_element** */, 
//		xi, /*element_dimension*/3, cmiss_region
//		, /*propagate_field*/0, /*find_nearest_location*/1);
	
	point[0] = lambda, point[1] = mu, point[2] = theta;
	field = Cmiss_region_find_field_by_name(cmiss_region, "coordinates");
	int return_code = Computed_field_find_element_xi(field,
		point, /*number_of_values*/3, time, &element /*FE_element** */, 
		xi, /*element_dimension*/3, cmiss_region
		, /*propagate_field*/0, /*find_nearest_location*/1);	
	if (return_code)
	{
//		if (xi[2] < 0.5)
//		{
//			xi[2] = 0.0f; // projected on endocardium
//		}
//		else
//		{
//			xi[2] = 1.0f; // projected on epicardium
//		}
#ifndef NDEBUG
		cout << "Data Point : " << point[0] << ", " << point[1] << ", " << point[2] << endl;
		cout << "PS xi : " << xi[0] << ", " << xi[1] << ", " << xi[2] << endl;
		
		FE_value values[3], derivatives[9];
		Computed_field_evaluate_in_element(field, element, xi,
									/*time*/0, (struct FE_element *)NULL, values, derivatives);
		cout << "Projected : " << values[0] << ", " << values[1] << ", " << values[2] << endl;
		cout << "elem : " << Cmiss_element_get_identifier(element)<< endl;
#endif
	}
	else
	{
		cout << "Can't find xi" << endl;
	}
	
	
	
	if (return_code)
	{
		xi_coord = xi;
		return Cmiss_element_get_identifier(element);
	}
	
	return -1;
}

void CAPModelLVPS4X4::SetLambda(const std::vector<float>& lambdaParams, float time)
{
	for (int i = 1; i <= NUMBER_OF_NODES; i ++) // node index starts at 1
	{
	
		// Cmiss_field_set_values_at_node() doesnt work for derivatives
		// set_FE_nodal_FE_value_value looks promising
		// see node_viewer_setup_components -> node_viewer_add_textctrl -> OnNodeViewerTextCtrlEntered
		// -> NodeViewerTextEntered
		
		int version = 0;
		int component_number = 0;
		
		FE_region* fe_region;
		struct FE_node *node;
		if (fe_region = Cmiss_region_get_FE_region(pImpl_->region))
		{
			node = ACCESS(FE_node)(FE_region_get_FE_node_from_identifier(fe_region, i));
			if (!node)
			{
				//error!
				cout << "Error: no such node: " << i << endl;
			}
		}
		
		struct FE_field *fe_field;
		Computed_field_get_type_finite_element(pImpl_->field,&fe_field);
		for (int value_number = 0; value_number < 4; ++value_number)
		{
			const FE_nodal_value_type types[4] = {
				FE_NODAL_VALUE,
				FE_NODAL_D_DS1,
				FE_NODAL_D_DS2,
				FE_NODAL_D2_DS1DS2
			};
//			cout << "node = " << node << ", value_type = " << types[value_number] << endl;
			set_FE_nodal_FE_value_value(node,
				 fe_field, component_number,
				 version,
				 types[value_number], time, lambdaParams[(i-1)*4 + value_number]);
		}
		
		DEACCESS(FE_node)(&node);
	}
}

void CAPModelLVPS4X4::SetLambdaForFrame(const std::vector<float>& lambdaParams, int frameNumber)
{
	SetLambda(lambdaParams, static_cast<float>(frameNumber)/numberOfModelFrames_);
}

const std::vector<float> GetLambda(int frame)
{
	std::vector<float> lambdas;
	for (int i=0; i < 134; i++)
	{
		//TODO we need to get Bezier coefficients. Cmgui only has Hermite.
	}
}

void CAPModelLVPS4X4::SetMuFromBasePlaneForFrame(const Plane& basePlane, int frameNumber)
{
	float time = static_cast<float>(frameNumber)/numberOfModelFrames_;
	const Vector3D& normal = TransformToLocalCoordinateRC(basePlane.normal);
	const Point3D& position = TransformToLocalCoordinateRC(basePlane.position);
	const int numberOfComponents = 3; // lambda, mu and theta
	
	//Epi
	
	float mu[4];
	for (int i=0;i<4;i++)
	{
		FE_region* fe_region;
		struct FE_node *node;
		if (fe_region = Cmiss_region_get_FE_region(pImpl_->region))
		{
			node = ACCESS(FE_node)(FE_region_get_FE_node_from_identifier(fe_region, i+1));
			if (!node)
			{
				//error!
				cout << "Error: no such node: " << i << endl;
			}
		}
		
		float values[3];
		if (!Cmiss_field_evaluate_at_node(pImpl_->field, node,time,numberOfComponents , values))
		{
			//Error
		}
		float lambda = values[0];
		float theta = values[2];
		mu[i] = values[1] = 0.0;
		
		float x, y, z;
		if (!prolate_spheroidal_to_cartesian(lambda, mu[i], theta, focalLength_, &x, &y, &z, (float*)0))
		{
			//Error
		}

//#ifdef CIM_ALGO
		Point3D point(x,y,z);
		double initial = DotProduct(normal, point - position);
		//do while on the same side and less than pi
		do
		{
			mu[i] += M_PI/180.0;  //one degree increments
			if (!prolate_spheroidal_to_cartesian(lambda, mu[i], theta, focalLength_, &x, &y, &z,0))
			{
			//Error
			}		
			point = Point3D(x,y,z);
		}
		while((initial*DotProduct(normal, point - position) > 0.0) && (mu[i] < M_PI));
		
		if (!prolate_spheroidal_to_cartesian(lambda, mu[i] - M_PI/180.0, theta, focalLength_, &x, &y, &z,0))
		{
			//Error
		}
		Point3D lastpoint(x,y,z);
	    
		double z1 = DotProduct(normal, lastpoint-position);
		double z2 = DotProduct(normal, point-position);
		if((z1*z2) < 0.0) 
		{
			double zdiff = z2-z1;
			double s;
			if(fabs(zdiff)<1.0e-05) s=0.5;
			else s = (-z1)/zdiff;
			Point3D interpoint = lastpoint + s*(point-lastpoint);
			cartesian_to_prolate_spheroidal( interpoint.x,interpoint. y,interpoint. z, focalLength_, &lambda,&mu[i],&theta,0);
		}
		
//		std::cout << "frameNumber = " << frameNumber << ", node = " << i +1  << ", mu :CIM_way = " << mu[i] ;
//#endif
	    
#ifdef MY_ALGO
		double a = normal.x;
		double b = normal.y;
		double c = normal.z;
		double d = DotProduct(normal, position - Point3D(0,0,0));
		
		double c1 = focalLength_ * std::sinh(lambda) * std::cos(theta);
		double c2 = focalLength_ * std::sinh(lambda) * std::sin(theta);
		double c3 = focalLength_ * std::cosh(lambda);
		
		double alpha = a*c1 + b*c2;
		double beta = c*c3;
		double gamma = d;
		
		mu[i] = SolveASinXPlusBCosXIsEqualToC(alpha, beta, gamma);
		
		std::cout << " :My_way = " << mu[i];
#endif 
	    
		std::cout << std::endl ;
		
		struct FE_field *fe_field;
		Computed_field_get_type_finite_element(pImpl_->field,&fe_field);
		
		const FE_nodal_value_type type = FE_NODAL_VALUE;
		const int version = 0;
		const int component_number = 1; //MU
		
		FE_value value = mu[i];
		set_FE_nodal_FE_value_value(node,
				fe_field, component_number,
				version,
				type, time, value);
		
		DEACCESS(FE_node)(&node);
	} // i
	  
	for (int j=1;j<5;j++)
	{
		for (int i=0;i<4;i++)
		{
			//modelParams[1](j*4+i) = mu[i]/4.0 * (4.0- (double)j);
			float value = mu[i]/4.0 * (4.0- (double)j);
			
			FE_region* fe_region;
			struct FE_node *node;
			if (fe_region = Cmiss_region_get_FE_region(pImpl_->region))
			{
				node = ACCESS(FE_node)(FE_region_get_FE_node_from_identifier(fe_region, (j*4) + i+1));
				if (!node)
				{
					//error!
					cout << "Error: no such node: " << i << endl;
				}
			}
			struct FE_field *fe_field;
			Computed_field_get_type_finite_element(pImpl_->field,&fe_field);
			
			const FE_nodal_value_type type = FE_NODAL_VALUE;
			const int version = 0;
			const int component_number = 1; //MU
			
			set_FE_nodal_FE_value_value(node,
					fe_field, component_number,
					version,
					type, time, value);
			
			DEACCESS(FE_node)(&node);
		}
	}

	//Endocardium

	for (int i=0;i<4;i++)
	{

		FE_region* fe_region;
		struct FE_node *node;
		if (fe_region = Cmiss_region_get_FE_region(pImpl_->region))
		{
			node = ACCESS(FE_node)(FE_region_get_FE_node_from_identifier(fe_region, i+20+1));
			if (!node)
			{
				//error!
				cout << "Error: no such node: " << i << endl;
			}
		}

		float values[3];
		if (!Cmiss_field_evaluate_at_node(pImpl_->field, node,time,numberOfComponents , values))
		{
			//Error
		}
		float lambda = values[0];
		float theta = values[2];
		mu[i] = values[1] = 0.0;

		float x, y, z;
		if (!prolate_spheroidal_to_cartesian(lambda, mu[i], theta, focalLength_, &x, &y, &z, (float*)0))
		{
			//Error
		}

//#ifdef CIM_ALGO
		Point3D point(x,y,z);
		double initial = DotProduct(normal, point - position);
		//do while on the same side and less than pi
		do
		{
			mu[i] += M_PI/180.0;  //one degree increments
			if (!prolate_spheroidal_to_cartesian(lambda, mu[i], theta, focalLength_, &x, &y, &z,0))
			{
				//Error
			}		
			point = Point3D(x,y,z);
		}
		while((initial*DotProduct(normal, point - position) > 0.0) && (mu[i] < M_PI));

		if (!prolate_spheroidal_to_cartesian(lambda, mu[i] - M_PI/180.0, theta, focalLength_, &x, &y, &z,0))
		{
			//Error
		}
		Point3D lastpoint(x,y,z);

		double z1 = DotProduct(normal, lastpoint-position);
		double z2 = DotProduct(normal, point-position);
		if((z1*z2) < 0.0) 
		{
			double zdiff = z2-z1;
			double s;
			if(fabs(zdiff)<1.0e-05) s=0.5;
			else s = (-z1)/zdiff;
			//printf ("epi:  s = %10.7f\n", s);
			Point3D interpoint = lastpoint + s*(point-lastpoint);
			cartesian_to_prolate_spheroidal( interpoint.x,interpoint. y,interpoint. z, focalLength_, &lambda,&mu[i],&theta,0);
		}

		std::cout << "frameNumber = " << frameNumber << ", node = " << i +1  << ", mu :CIM_way = " << mu[i] ;
//#endif

		std::cout << std::endl ;

		struct FE_field *fe_field;
		Computed_field_get_type_finite_element(pImpl_->field,&fe_field);

		const FE_nodal_value_type type = FE_NODAL_VALUE;
		const int version = 0;
		const int component_number = 1; //MU

		FE_value value = mu[i];
		set_FE_nodal_FE_value_value(node,
				fe_field, component_number,
				version,
				type, time, value);

		DEACCESS(FE_node)(&node);
	} // i

	for (int j=1;j<5;j++)
	{
		for (int i=0;i<4;i++)
		{
			//modelParams[1](j*4+i) = mu[i]/4.0 * (4.0- (double)j);
			float value = mu[i]/4.0 * (4.0- (double)j);

			FE_region* fe_region;
			struct FE_node *node;
			if (fe_region = Cmiss_region_get_FE_region(pImpl_->region))
			{
				node = ACCESS(FE_node)(FE_region_get_FE_node_from_identifier(fe_region, 20 + (j*4) + i+1));
				if (!node)
				{
					//error!
					cout << "Error: no such node: " << i << endl;
				}
			}
			struct FE_field *fe_field;
			Computed_field_get_type_finite_element(pImpl_->field,&fe_field);

			const FE_nodal_value_type type = FE_NODAL_VALUE;
			const int version = 0;
			const int component_number = 1; //MU

			set_FE_nodal_FE_value_value(node,
					fe_field, component_number,
					version,
					type, time, value);

			DEACCESS(FE_node)(&node);
		}
	}
	  
	return;
}
	
void CAPModelLVPS4X4::SetTheta(int frame)
{
	float time = (float)frame / GetNumberOfModelFrames();
	const FE_value thetas[4] = { 0, M_PI_2, M_PI, M_PI_2 * 3.0};
	
	for (int i = 1; i <= NUMBER_OF_NODES; i ++) // node index starts at 1
	{	
		
		FE_region* fe_region;
		struct FE_node *node;
		if (fe_region = Cmiss_region_get_FE_region(pImpl_->region))
		{
			node = ACCESS(FE_node)(FE_region_get_FE_node_from_identifier(fe_region, i));
			if (!node)
			{
				//error!
				cout << "Error: no such node: " << i << endl;
			}
		}
		
		struct FE_field *fe_field;
		Computed_field_get_type_finite_element(pImpl_->field,&fe_field);

		const FE_nodal_value_type type = FE_NODAL_VALUE;
		const int version = 0;
		const int component_number = 2; //THETA
		
		FE_value value = thetas[(i-1)%4];
		set_FE_nodal_FE_value_value(node,
			 fe_field, component_number,
			 version,
			 type, time, value);
		
		DEACCESS(FE_node)(&node);
	}
}

float CAPModelLVPS4X4::MapToModelFrameTime(float time) const
{
	float frameDuration = (float) 1.0 / numberOfModelFrames_;
	int indexPrevFrame = time / frameDuration;
	if ((time - (float)frameDuration*indexPrevFrame) < (frameDuration/2))
	{
		return (float) indexPrevFrame/numberOfModelFrames_;
	}
	else
	{
		return (float) (indexPrevFrame+1)/numberOfModelFrames_;
	}
}

int CAPModelLVPS4X4::MapToModelFrameNumber(float time) const
{
	//edge cases
	if (time < 0)
	{
		time = 0;
	}
	else if (time > 1)
	{
		time = 1.0; //REVISE
	}
	
	float frameDuration = (float) 1.0 / numberOfModelFrames_;
	
	return std::min(static_cast<int>(time / frameDuration), numberOfModelFrames_);
}

float CAPModelLVPS4X4::ComputeVolume(SurfaceType surface, float time)
{
	const int numElements = 16;
	const int nx = 7, ny = 7;

	Point3D b[numElements][nx];
	Point3D p[numElements*nx*ny];
	Point3D temp;
	float vol_sum = 0;
	Point3D origin(0,0,0);

	// initialise arrays
	for (int ne=0;ne<numElements;ne++)
	{
		for(int i=0;i<nx;i++)
		{
			b[ne][i] = Point3D(0,0,0);
		}
	}

	Cmiss_region* cmiss_region = pImpl_->region;
	Cmiss_field_id field = Cmiss_region_find_field_by_name(cmiss_region, "coordinates");
	FE_region* fe_region = Cmiss_region_get_FE_region(cmiss_region);
	struct CM_element_information identifier;
	identifier.type = CM_ELEMENT;
	
	//do for all elements
	for (int ne=0;ne<numElements;ne++)
	{
		identifier.number = ne+1;
		Cmiss_element* element = FE_region_get_FE_element_from_identifier(fe_region, &identifier);
		if (!element)
		{
			std::cout << __func__ << " Error: can't find element from id = " << ne << std::endl;
		}
		//calculate vertex coordinates for element subdivision
		for (int i=0;i<nx;i++)
		{
			for (int j=0;j<ny;j++)
			{
				//calculate lamda mu and theta at this point
				FE_value values[3], xi[3];
				xi[0] = (float) i/(nx-1);
				xi[1] = (float) j/(ny-1);
				xi[2] = (surface == ENDOCARDIUM) ? 0.0f : 1.0f;
				Computed_field_evaluate_in_element(field, element, xi,
					time, (struct FE_element *)NULL, values, (FE_value*)0 /*derivatives*/);

				prolate_spheroidal_to_cartesian(values[0],values[1],values[2],
					focalLength_, &temp.x, &temp.y, &temp.z, (float*)0);

//				std::cout << __func__ << ": " << temp.x << " " << temp.y << " " << temp.z << endl;

				p[(j*nx+i)] = temp;
			} // j
		} // I
		//do for all quads
		//note vertices must be ordered ccw viewed from outside
		for(int i=0;i<nx-1;i++){
			for(int j=0;j<ny-1;j++){
				int n1 = j*(nx) + i ;
				int n2 = n1 + 1;
				int n3 = n2 + (nx);
				float vol = ComputeVolumeOfTetrahedron(p[n1],p[n2],p[n3],origin);
				vol_sum += vol;

				n1 = n1;
				n2 = n3;
				n3 = n2-1;
				vol = ComputeVolumeOfTetrahedron(p[n1],p[n2],p[n3],origin);
				vol_sum += vol;
			} /* j */
		} /* i */
		//store the base ring
		if(ne<4)
		{
			for(int k=0;k<nx;k++)
			{
				b[ne][k] = p[(ny-1)*nx + k];
			}
		}

	} /* ne */

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
	c *= (1/(float)num);
	for(int ne=0;ne<4;ne++){
		for(int i=0;i<nx-1;i++){
			int n1 = i ;
			int n2 = n1+1;
			float vol = ComputeVolumeOfTetrahedron(b[ne][n1], b[ne][n2], c, origin);
			vol_sum += vol;
		}
	}

	return (vol_sum/6000.0);
	// (6*1000), 6 times volume of tetrahedron & for ml
}

void CAPModelLVPS4X4::SetFocalLengh(float focalLength)
{
	struct Coordinate_system* coordinate_system = Computed_field_get_coordinate_system(pImpl_->field);
	focalLength_ = focalLength;
	coordinate_system->parameters.focus = focalLength_;
}
