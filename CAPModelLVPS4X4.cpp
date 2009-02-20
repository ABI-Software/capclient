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

extern "C" {
#include "command/cmiss.h"
#include "api/cmiss_region.h"
#include "graphics/scene_viewer.h"
#include "time/time_keeper.h"
#include "graphics/scene.h"
}

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

CAPModelLVPS4X4::CAPModelLVPS4X4(const std::string& modelName_)
: modelName(modelName_)
{}

using namespace std;

int CAPModelLVPS4X4::readModelFromFiles(const std::string& path)
{	
	stringstream pathStream;	
	pathStream << prefix << path << "/";// << modelName << "_";// << 
	string dir_path = pathStream.str();

	readModelInfo(dir_path); // this will set numberOfModelFrames and transformation Matrix
	
	Cmiss_command_data* command_data = CmguiManager::getInstance().getCmissCommandData();
	
	Cmiss_region* region = Cmiss_command_data_get_root_region(command_data);
	struct Time_keeper* time_keeper = Cmiss_command_data_get_default_time_keeper(command_data);
	
	for (int i = 0; i<numberOfModelFrames; i++)
	{		
		stringstream filenameStream;
		filenameStream << dir_path << path << "_" << i+1 << ".model.exnode" ;
		string filenameString = filenameStream.str().c_str();
		char* filename = const_cast<char*>(filenameString.c_str());
		//DEBUG
		//cout << "DEBUG: i = " << i << ", filename = " << filename << endl;
		float time = static_cast<float>(i)/numberOfModelFrames;
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

	char* scene_object_name = const_cast<char*>(modelName.c_str()); // temporarily remove constness to be compatible with Cmgui

	Scene_object* scene_object;
	if (scene_object_name)
	{
		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
		struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		if (scene_object=Scene_get_Scene_object_by_name(scene,
			scene_object_name))
		{
			//Scene_object_remove_time_dependent_transformation(scene_object);//??????? Why need time dependent transformation??
			Scene_object_set_transformation(scene_object, &patientToGlobalTransform);
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

	return 0;
}

#include "CAPMath.h"

Point3D temporary_helper_function_to_transform_vector(ifstream& in)
{
	string temp;
	float x,y,z;
	in >> temp; // name of the vector
	in >> x;
	in >> temp; // trailing character = i
	in >> y;
	in >> temp; // trailing character = j
	in >> z;
	in >> temp; // trailing character = k
	
	return Point3D(x,y,z); //hopefully RVO will prevent creation of temporary objects 
}

void CAPModelLVPS4X4::readModelInfo(std::string modelInfoFilePath)
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
	
	modelInfoFile >> numberOfModelFrames;
	cout << numberOfModelFrames <<endl;
	
	
	// How to get transformation matrix from the basis vectors of and the translation of the origin
	//
	// ii ji ki  0
	// ij jj kj  0    *  translation
	// ik jk kk  0
	//  0  0  0  1
	// from http://physics.usask.ca/~chang/phys323/notes/lecture1.pdf
	
	// Read in model to world coordinate transformation
	getline(modelInfoFile, line); //prev line
	getline(modelInfoFile, line); //empty line
	cout << line << endl;	
	getline(modelInfoFile, line); //ModelToMagnetTransform:
	cout << line << endl;	
	
	//Point3D i_hat, j_hat, k_hat, translation; //needed for model transformation to world coord
	//i_hat, j_hat, k_hat =  model coord basis vectors in i, j & k directions
	//translation = origin translation
	
	Point3D i_hat = temporary_helper_function_to_transform_vector(modelInfoFile);
	Point3D j_hat = temporary_helper_function_to_transform_vector(modelInfoFile);
	Point3D k_hat = temporary_helper_function_to_transform_vector(modelInfoFile);
	Point3D translation = temporary_helper_function_to_transform_vector(modelInfoFile);
	
	cout << "a1 = " << i_hat <<endl;
	cout << "a2 = " << j_hat <<endl;
	cout << "a3 = " << k_hat <<endl;
	cout << "t  = " << translation <<endl;
	
	Point3D i(1,0,0), j(0,1,0), k(0,0,1); // world coord basis vectors
	gtMatrix temp;
	temp[0][0] = DOT(i,i_hat);
	temp[0][1] = DOT(j,i_hat);
	temp[0][2] = DOT(k,i_hat);

	temp[1][0] = DOT(i,j_hat);
	temp[1][1] = DOT(j,j_hat);
	temp[1][2] = DOT(k,j_hat);

	temp[2][0] = DOT(i,k_hat);
	temp[2][1] = DOT(j,k_hat);
	temp[2][2] = DOT(k,k_hat);

	temp[3][0] = translation.x;
	temp[3][1] = translation.y;
	temp[3][2] = translation.z;

	patientToGlobalTransform[0][0]=temp[0][0];
	patientToGlobalTransform[0][1]=temp[0][1];
	patientToGlobalTransform[0][2]=temp[0][2];
	patientToGlobalTransform[0][3]=0;
	patientToGlobalTransform[1][0]=temp[1][0];
	patientToGlobalTransform[1][1]=temp[1][1];
	patientToGlobalTransform[1][2]=temp[1][2];
	patientToGlobalTransform[1][3]=0;
	patientToGlobalTransform[2][0]=temp[2][0];
	patientToGlobalTransform[2][1]=temp[2][1];
	patientToGlobalTransform[2][2]=temp[2][2];
	patientToGlobalTransform[2][3]=0;
	patientToGlobalTransform[3][0]=temp[3][0];
	patientToGlobalTransform[3][1]=temp[3][1];
	patientToGlobalTransform[3][2]=temp[3][2];
	patientToGlobalTransform[3][3]=1;
	
	return;
}
