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
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


struct CAPModelLVPS4X4::HeartModelImpl
{
	Cmiss_command_data* commandData;
	Cmiss_region* region;
	Scene_object* sceneObject;
};

CAPModelLVPS4X4::CAPModelLVPS4X4(const std::string& modelName)
:
	modelName_(modelName),
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

	// define the coord field in RC for MII computation
	// FIX use API calls instead of command line
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
	Time_object* timeObject = Scene_object_get_time_object(modelSceneObject_);
	if (timeObject) {
		Time_object_set_update_frequency(timeObject, numberOfModelFrames_);
		//Time_object_set_update_frequency(timeObject, 5);
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
		"gfx mod g_el heart surfaces exterior face xi3_0 select_on material green selected_material default_selected render_wireframe;"
		);
		
		Cmiss_command_data_execute_command(command_data,
		"gfx mod g_el heart surfaces exterior face xi3_1 select_on material red selected_material default_selected render_wireframe;"
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

int CAPModelLVPS4X4::ComputeXi(const Point3D& coord, Point3D& xi_coord)
{
	//1. Transform to model coordinate
	gtMatrix mInv;
	inverseMatrix(patientToGlobalTransform_, mInv);
	transposeMatrix(mInv);// gtMatrix is column Major and our matrix functions assume row major FIX
	
	Point3D coordLocal = mInv * coord;
	
	cout << "Local coord = " << coordLocal << endl;
	
	//2. Transform to Prolate Spheroidal
	float lambda, mu, theta;
	cartesian_to_prolate_spheroidal(coordLocal.x,coordLocal.y,coordLocal.z, 38.6449, 
			&lambda,&mu, &theta,0);
	cout << "lambda: " << lambda << ", mu: " << mu << ", theta: " << theta << endl;
	
	//3. Project on to model surface and obtain the material coordinates
	Cmiss_region* root_region = Cmiss_command_data_get_root_region(pImpl_->commandData);
	Cmiss_region* cmiss_region;
	Cmiss_region_get_region_from_path(root_region, "heart", &cmiss_region);
	
	Cmiss_field_id field = Cmiss_region_find_field_by_name(cmiss_region, "heart_rc_coord");//FIX
	
	FE_value point[3], xi[3];
//	point[0] = lambda, point[1] = mu, point[2] = theta;
	point[0] = coordLocal.x, point[1] = coordLocal.y, point[2] = coordLocal.z;
	FE_element* element = 0;
	int return_code = Computed_field_find_element_xi(field,
		point, /*number_of_values*/3, &element /*FE_element** */, 
		xi, /*element_dimension*/3, cmiss_region
		, /*propagate_field*/0, /*find_nearest_location*/1);
	
	if (return_code)
	{
		cout << "PS xi : " << xi[0] << ", " << xi[1] << ", " << xi[2] << endl;
		cout << "elem : " << Cmiss_element_get_identifier(element)<< endl;
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
