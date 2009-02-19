#define UNIX
#define DARWIN

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
   #include "wx/wx.h"
#endif
#include "wx/xrc/xmlres.h"

#include "Config.h"
#include "ViewerFrame.h"
#include "DICOMImage.h"
//#include "FileSystem.h"
#include "CmguiExtensions.h"
#include "CmguiManager.h"

#if defined (DARWIN)
#include <ApplicationServices/ApplicationServices.h>
#endif

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_scene_viewer_private.h"
#include "general/debug.h"
#include "graphics/scene_viewer.h"
#include "graphics/transform_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"
#include "graphics/scene.h"

#include "command/cmiss.h"
#include "general/debug.h"
#include "user_interface/message.h"

#include "time/time_keeper.h"

#include "api/cmiss_region.h"
#include "finite_element/import_finite_element.h"
#include "api/cmiss_texture.h"
#include "graphics/material.h"
#include "general/manager.h"
#include "finite_element/finite_element.h"
#include "time/time.h"
#include "user_interface/event_dispatcher.h"
}

#include <iostream>


using namespace std;

int time_callback(struct Time_object *time, double current_time, void *user_data)
{
	//DEBUG
//	cout << "Time_call_back time = " << current_time << endl;
	
	ImageSet* imageSet = reinterpret_cast<ImageSet*>(user_data);
	imageSet->setTime(current_time);
}

int main(int argc,char *argv[])
{
	struct Cmiss_command_data *command_data;


#if defined (DARWIN)
	ProcessSerialNumber PSN;
	GetCurrentProcess(&PSN);
	TransformProcessType(&PSN,kProcessTransformToForegroundApplication);
#endif

	if (command_data = create_Cmiss_command_data(argc, argv, "0,0"))
	{

		CmguiManager cmguiManager(command_data);
		
		wxXmlResource::Get()->Load("ViewerFrame.xrc");
#ifdef HARD_CODED_GUI
//		ViewerFrame *frame = new ViewerFrame(
//			   wxT("Test Viewer"), 100, 100, 600, 600);
#endif
		ViewerFrame *frame = new ViewerFrame(command_data);
		frame->Show(TRUE);

		wxPanel *panel = frame->getPanel();

		if (!panel)
		{
			printf("panel is null");
			return 0;
		}

		Cmiss_region* region = Cmiss_command_data_get_root_region(command_data);
		struct Time_keeper* time_keeper = Cmiss_command_data_get_default_time_keeper(command_data);

		string filename(prefix);
		filename.append("/test_1.model.exnode");
		if (!Cmiss_region_read_file_with_time(region,const_cast<char*>(filename.c_str()),time_keeper,0))
//		if (!Cmiss_region_read_file(region,"/Users/jchu014/cmiss/api_test2/Data/test_1.model.exnode"))
		{
			std::cout << "Error reading ex file - test_1.model.exnode, 0" << std::endl;
		}
		filename = prefix;
		filename.append("/GlobalHermiteParam.exelem");
		if (!Cmiss_region_read_file(region, const_cast<char*>(filename.c_str())))
		{
			std::cout << "Error reading ex file - exelem" << std::endl;
		}

#define HEART_ANIMATION
#ifdef HEART_ANIMATION
		for (int i = 2; i<29; i++)
		{
			char filename[100];
			sprintf(filename, "%s/test_%d.model.exnode",prefix, i);
			float time = ((float)(i-1))/28.0f;
			//std::cout << "time = " << time << endl;
			if (!Cmiss_region_read_file_with_time(region,filename,time_keeper,time))
			{
				std::cout << "Error reading ex file: " << string(filename) << std::endl;
			}
		}

		//Transform the heart model to world coordinate

		char* scene_object_name = "heart";
		gtMatrix transformation_matrix, temp;
		// ii ji ki  0
		// ij jj kj  0    *  translation
		// ik jk kk  0
		//  0  0  0  1
		// from http://physics.usask.ca/~chang/phys323/notes/lecture1.pdf

		Point3D i(1,0,0), j(0,1,0), k(0,0,1);
		Point3D i_hat(0.614465, -0.714418, -0.334724), j_hat( -0.442694, -0.663401, 0.603259),
			k_hat(-0.653035, -0.222501, -0.723905);
		Point3D translation(42.3143, -38.2987, 11.0703);

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



		transformation_matrix[0][0]=temp[0][0];
		transformation_matrix[0][1]=temp[0][1];
		transformation_matrix[0][2]=temp[0][2];
		transformation_matrix[0][3]=0;
		transformation_matrix[1][0]=temp[1][0];
		transformation_matrix[1][1]=temp[1][1];
		transformation_matrix[1][2]=temp[1][2];
		transformation_matrix[1][3]=0;
		transformation_matrix[2][0]=temp[2][0];
		transformation_matrix[2][1]=temp[2][1];
		transformation_matrix[2][2]=temp[2][2];
		transformation_matrix[2][3]=0;
		transformation_matrix[3][0]=temp[3][0];
		transformation_matrix[3][1]=temp[3][1];
		transformation_matrix[3][2]=temp[3][2];
		transformation_matrix[3][3]=1;

		Scene_object* scene_object;
		if (scene_object_name)
		{
			Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
			struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
			if (scene_object=Scene_get_Scene_object_by_name(scene,
				scene_object_name))
			{
				//Scene_object_remove_time_dependent_transformation(scene_object);//??????? Why need time dependent transformation??
				Scene_object_set_transformation(scene_object, &transformation_matrix);
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
		
		Time_object* time = Scene_object_get_time_object(scene_object);
//		Time_object_set_update_frequency(time, 5);
#endif HEART_ANIMATION
		
#define TEXTURE_ANIMATION
#ifdef TEXTURE_ANIMATION
		vector<string> sliceNames;
		sliceNames.push_back("SA1");
		sliceNames.push_back("SA4");
		sliceNames.push_back("LA1");
		
		ImageSet imageSet(sliceNames);
		
#define TIME_OBJECT_CALLBACK_TEST
#ifdef TIME_OBJECT_CALLBACK_TEST
		Time_object* time_object = create_Time_object("Texture_animation_timer");
		
		Time_object_add_callback(time_object,time_callback,(void*)&imageSet);
		Time_object_set_time_keeper(time_object, time_keeper);
		Time_object_set_update_frequency(time_object,28);//BUG?? doesnt actually update 28 times -> only 27 
#endif		

#endif //TEXTURE_ANIMATION
		
		Cmiss_scene_viewer_id sceneViewer = create_Cmiss_scene_viewer_wx(Cmiss_command_data_get_scene_viewer_package(command_data),
				panel,
				CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
				CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
				/*minimum_colour_buffer_depth*/8,
				/*minimum_depth_buffer_depth*/8,
				/*minimum_accumulation_buffer_depth*/8);

//#define HARD_CODED_GUI
#ifdef HARD_CODED_GUI
		panel->GetContainingSizer()->SetMinSize(600, 600);
		panel->GetContainingSizer()->SetDimension(-1, -1, 600, 600);
		frame->GetSizer()->SetSizeHints(frame);		
//		struct Scene* scene = Scene_viewer_get_scene(sceneViewer);
		//struct Time_keeper* time_keeper = Scene_get_default_time_keeper(scene); // get it directly from command_data instead

		Time_keeper_play(time_keeper,TIME_KEEPER_PLAY_FORWARD);
		Time_keeper_set_play_loop(time_keeper);
		Time_keeper_set_play_every_frame(time_keeper);
#endif 
		
		Cmiss_scene_viewer_view_all(sceneViewer);
		
		Cmiss_command_data_main_loop(command_data);//app.OnRun()
		//DESTROY(Cmiss_command_data)(&command_data);
		return 1;
	}

	return 0;
} /* main */
