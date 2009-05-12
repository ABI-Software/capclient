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

Point3D CAPModelLVPS4X4::TransformToProlateSheroidal(const Point3D& rc) const
{
	float lambda, mu, theta;
		
	cartesian_to_prolate_spheroidal(rc.x, rc.y, rc.z, focalLength_, &lambda, &mu, &theta,0);
	//cout << "lambda: " << lambda << ", mu: " << mu << ", theta: " << theta << ", focalLength = " << focalLength_ << endl;
	
	return Point3D(lambda, mu, theta);
}

int CAPModelLVPS4X4::ComputeXi(const Point3D& coord, Point3D& xi_coord) const
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
		point, /*number_of_values*/3, &element /*FE_element** */, 
		xi, /*element_dimension*/3, cmiss_region
		, /*propagate_field*/0, /*find_nearest_location*/1);	
	if (return_code)
	{
		if (xi[2] < 0.5)
		{
			xi[2] = 0.0f; // projected on endocardium
		}
		else
		{
			xi[2] = 1.0f; // projected on epicardium
		}
#ifndef NDEBUG
		cout << "Data Point : " << point[0] << ", " << point[1] << ", " << point[2] << endl;
		cout << "PS xi : " << xi[0] << ", " << xi[1] << ", " << xi[2] << endl;
		
		FE_value values[3], derivatives[10];
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
		//TODO we need to get Bezier coefficients. Cmgui only has Hermite. What do we do?
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
	float frameDuration = (float) 1.0 / numberOfModelFrames_;
	int indexPrevFrame = time / frameDuration;
	if ((time - (float)frameDuration*indexPrevFrame) < (frameDuration/2))
	{
		return indexPrevFrame;
	}
	else
	{
		return indexPrevFrame+1;
	}
}
