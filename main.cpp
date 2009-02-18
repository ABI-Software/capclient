#define UNIX
#define DARWIN

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
   #include "wx/wx.h"
#endif
#include "wx/xrc/xmlres.h"

#include "ViewerFrame.h"
#include "DICOMImage.h"
#include "FileSystem.h"
#include "CmguiExtensions.h"

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

static const char* prefix = "/Users/jchu014/cmiss/api_test2/Data/";


void loadImagePlane(const string& name, Cmiss_command_data* command_data)
{
	char filename[256];
	sprintf(filename, "%s%s.exnode", prefix, name.c_str());

	Cmiss_region* region = Cmiss_command_data_get_root_region(command_data);
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exnode" << std::endl;
	}
	sprintf(filename, "%s%s.exelem", prefix, name.c_str());
	if (!Cmiss_region_read_file(region,filename))
	{
		std::cout << "Error reading ex file - ImagePlane.exelem" << std::endl;
	}

//	struct Cmiss_texture_manager* manager = Cmiss_command_data_get_texture_manager(command_data);
//	struct IO_stream_package* io_stream_package = Cmiss_command_data_get_IO_stream_package(command_data);
//
//	sprintf(filename, "%s%s/%s", prefix, name.c_str(), name.c_str()); // HACK FIX
//	Cmiss_texture_id texture_id = Cmiss_texture_manager_create_texture_from_file(
//		manager, name.c_str(), io_stream_package, filename);
	
#define ADJUST_BRIGHTNESS
#ifdef ADJUST_BRIGHTNESS
//	gfx define field tex sample_texture coordinates xi texture LA1;
	
//	gfx define field rescaled_tex rescale_intensity_filter field tex output_min 0 output_max 1;
//	gfx cre spectrum monochrome clear;
//	gfx modify spectrum monochrome linear range 0 1 extend_above extend_below monochrome colour_range 0 1 ambient diffuse component 1;
//	#create a texture using the rescaled sample_texture field
//	gfx create texture tract linear;
//	gfx modify texture tract width 1 height 1 distortion 0 0 0 colour 0 0 0 alpha 0 decal linear_filter resize_nearest_filter clamp_wrap specify_number_of_bytes 2 evaluate field rescaled_tex element_group LA1 spectrum monochrome texture_coordinate xi fail_material transparent_gray50;
//
//	# create a material containing the texture so that we can select along
//	# with appropriate texture coordinates to visualise the 3D image
//	gfx create material tract texture tract
#endif //ADJUST_BRIGHTNESS
	
	Graphical_material* material = create_Graphical_material(name.c_str());
//	if (!Graphical_material_set_texture(material,texture_id))
//	{
//		//Error
//		cout << "Error: Graphical_material_set_texture()" << endl;
//	}

	Material_package* material_package = Cmiss_command_data_get_material_package(command_data);
	Material_package_manage_material(material_package, material);
	
	GT_element_group* gt_element_group;
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	if (!scene)
	{
		cout << "Can't find scene" << endl;
	}

	Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
	//Got to find the child region first!!
	if(!Cmiss_region_get_region_from_path(root_region, name.c_str(), &region))
	{
		//error
		std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
	}

	GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
	//hack
	GT_element_settings_set_selected_material(settings, material);

	if(!GT_element_settings_set_material(settings, material))
	{
		//Error;
	}
	else
	{
		manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(root_region);
		Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);

		GT_element_settings_set_texture_coordinate_field(settings,c_field);

		int Cmiss_region_modify_g_element(struct Cmiss_region *region,
			struct Scene *scene, struct GT_element_settings *settings,
			int delete_flag, int position);  // should add this to a header file somewhere

		 if (!Cmiss_region_modify_g_element(region, scene,settings,
			/*delete_flag*/0, /*position*/-1))
		 {
			 //error
		 }
	}

	return;
}

void transformImagePlane(const string& filename, const string& name, Cmiss_command_data* command_data)
{
	// Now get the necessary info from the DICOM header
	//string DICOMFilename("/Users/jchu014/cmiss/api_test2/Data/68708398");
	DICOMImage dicomImage(filename);
	ImagePlane* plane = dicomImage.getImagePlaneFromDICOMHeaderInfo();
	//ImagePlane* plane = getImagePlaneFromDICOMHeaderInfo(filename);

	if (!plane)
	{
		cout << "ERROR !! plane is null"<<endl;
	}
	else
	{
		cout << plane->tlc << endl;
	}

	int nodeNum = 81; // HACK
	if (name=="SA4")
	{
		nodeNum += 500;
	}
	else if (name =="LA1")
	{
		nodeNum += 1100;
	}

	Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
	//Got to find the child region first!!
	Cmiss_region* region;
	if(!Cmiss_region_get_region_from_path(root_region, name.c_str(), &region))
	{
		//error
		std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
	}
	
	char nodeName[256];
	sprintf(nodeName,"%d", nodeNum);
	Cmiss_node* node = Cmiss_region_get_node(region, nodeName);
	if (node) {
		FE_node_set_position_cartesian(node, 0, plane->blc.x, plane->blc.y, plane->blc.z);
	}
	else
	{
		cout << nodeName << endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node = Cmiss_region_get_node(region, nodeName))
	{
		FE_node_set_position_cartesian(node, 0, plane->brc.x, plane->brc.y, plane->brc.z);
	}
	else
	{
		cout << nodeName << endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node = Cmiss_region_get_node(region, nodeName))
	{
		FE_node_set_position_cartesian(node, 0, plane->tlc.x, plane->tlc.y, plane->tlc.z);
	}
	else
	{
		cout << nodeName << endl;
	}

	nodeNum++;
	sprintf(nodeName,"%d", nodeNum);
	if (node = Cmiss_region_get_node(region, nodeName))
	{
		FE_node_set_position_cartesian(node, 0, plane->trc.x, plane->trc.y, plane->trc.z);
	}
	else
	{
		cout << nodeName << endl;
	}
}

Cmiss_texture** loadTextureImagesAndTransformImagePlane(Cmiss_command_data* command_data, const string& dir_path)
{
	FileSystem fs(dir_path);
	
	vector<string> filenames = fs.getAllFileNames();
	
	Cmiss_texture** tex = new Cmiss_texture*[28];
	Cmiss_texture** pptr = tex;
	
	struct Cmiss_texture_manager* manager = Cmiss_command_data_get_texture_manager(command_data);
	struct IO_stream_package* io_stream_package = Cmiss_command_data_get_IO_stream_package(command_data);

	char filename[256];

	for (int i = 0; i < 28; i++)
	{
		sprintf(filename, "%s/%s", dir_path.c_str(),  filenames[i].c_str()); 
		Cmiss_texture_id texture_id = Cmiss_texture_manager_create_texture_from_file(
			manager, filenames[i].c_str(), io_stream_package, filename);
		*pptr = texture_id;
		pptr++;
	}
	
	int len = dir_path.length();
	transformImagePlane(filename, dir_path.substr(len-3,3), command_data);
	
	return tex;
}

struct HackyDataType
{
	vector<Cmiss_texture**> vector_of_tex;
	Cmiss_command_data* command_data;
	vector<string> sliceNames;
};

int texture_animation(void* data)
{
	static double prev_time = 0;
	double time;
	
	HackyDataType* args = ((HackyDataType*)data);
	vector<Cmiss_texture**>& vector_of_tex = args->vector_of_tex;
	Cmiss_command_data* command_data = args->command_data;
	vector<string>& slice_names = args->sliceNames;
	
	Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
	struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
	if (Scene_object* scene_object=Scene_get_Scene_object_by_name(scene,"heart"))
	{
		time = Scene_object_get_time(scene_object);
	
		if(prev_time == time)
		{
			return 1;
		}
	}
	else
	{
		cout <<"-_-" <<endl;
		return 0;
	}
	prev_time = time;

	int index = (time * 28)-1;
	if (index==-1)
	{
		index = 27;
	}

	vector<string>::iterator iter = slice_names.begin();
	vector<string>::iterator end = slice_names.end();
	
	for (int i = 0;iter != end; ++iter, i++)
	{
		string& slice_name = *iter;
		Cmiss_texture** tex= vector_of_tex[i];
		
		Material_package* material_package = Cmiss_command_data_get_material_package(command_data);
		MANAGER(Graphical_material)* mm = Material_package_get_material_manager(material_package);
		
		Graphical_material* material;
		if (material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,
									name)(slice_name.c_str(),mm))
		{
			if (!Graphical_material_set_texture(material,*(tex + index)))
			{
				//Error
				cout << "Error: Graphical_material_set_texture()" << endl;
			}
			
		}
		else
		{
			cout << "Error: cant find material" << endl;
		}
		
		GT_element_group* gt_element_group;
	
		Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
		//Got to find the child region first!!
		Cmiss_region* region;
		if(!Cmiss_region_get_region_from_path(root_region, slice_name.c_str(), &region))
		{
			//error
			std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
		}
	
		GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
		//hack
		GT_element_settings_set_selected_material(settings, material);
	
		if(!GT_element_settings_set_material(settings, material))
		{
			//Error;
		}
		else
		{
			manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(root_region);
			Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);
	
			GT_element_settings_set_texture_coordinate_field(settings,c_field);
	
			int Cmiss_region_modify_g_element(struct Cmiss_region *region,
				struct Scene *scene, struct GT_element_settings *settings,
				int delete_flag, int position);  // should add this to a header file somewhere
	
			 if (!Cmiss_region_modify_g_element(region, scene,settings,
				/*delete_flag*/0, /*position*/-1))
			 {
				 //error
			 }
		}
	}
	return 1;
}

int time_callback(struct Time_object *time, double current_time, void *user_data)
{
//	cout << "TIME!!! = " << current_time << endl;

	HackyDataType* args = ((HackyDataType*)user_data);
	vector<Cmiss_texture**>& vector_of_tex = args->vector_of_tex;
	Cmiss_command_data* command_data = args->command_data;
	vector<string>& slice_names = args->sliceNames;

	int index = (current_time * 28)-1;
	if (index==-1)
	{
		index = 27;
	}
	
	vector<string>::const_iterator iter = slice_names.begin();
	vector<string>::const_iterator end = slice_names.end();
	
	for (int i = 0;iter != end; ++iter, i++)
	{
		const string& slice_name = *iter;
		Cmiss_texture** tex= vector_of_tex[i];
		
		Material_package* material_package = Cmiss_command_data_get_material_package(command_data);
		MANAGER(Graphical_material)* mm = Material_package_get_material_manager(material_package);
		
		Graphical_material* material;
		if (material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,
									name)(slice_name.c_str(),mm))
		{
			if (!Graphical_material_set_texture(material,*(tex + index)))
			{
				//Error
				cout << "Error: Graphical_material_set_texture()" << endl;
			}
			
		}
		else
		{
			cout << "Error: cant find material" << endl;
		}
		
		GT_element_group* gt_element_group;
	
		Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
		//Got to find the child region first!!
		Cmiss_region* region;
		if(!Cmiss_region_get_region_from_path(root_region, slice_name.c_str(), &region))
		{
			//error
			std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
		}
	
		GT_element_settings* settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
		//hack
		GT_element_settings_set_selected_material(settings, material);
	
		if(!GT_element_settings_set_material(settings, material))
		{
			//Error;
		}
		else
		{
			manager_Computed_field* cfm = Cmiss_region_get_Computed_field_manager(root_region);
			Computed_field* c_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)("xi",cfm);
	
			GT_element_settings_set_texture_coordinate_field(settings,c_field);
	
			Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
			struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
			
			int Cmiss_region_modify_g_element(struct Cmiss_region *region,
				struct Scene *scene, struct GT_element_settings *settings,
				int delete_flag, int position);  // should add this to a header file somewhere
	
			 if (!Cmiss_region_modify_g_element(region, scene,settings,
				/*delete_flag*/0, /*position*/-1))
			 {
				 //error
			 }
		}
	}
	return 1;
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

		if (!Cmiss_region_read_file_with_time(region,"/Users/jchu014/cmiss/api_test2/Data/test_1.model.exnode",time_keeper,0))
//		if (!Cmiss_region_read_file(region,"/Users/jchu014/cmiss/api_test2/Data/test_1.model.exnode"))
		{
			std::cout << "Error reading ex file - test_1.model.exnode, 0" << std::endl;
		}
		if (!Cmiss_region_read_file(region,"/Users/jchu014/cmiss/api_test2/Data/GlobalHermiteParam.exelem"))
		{
			std::cout << "Error reading ex file - exelem" << std::endl;
		}

#define HEART_ANIMATION
#ifdef HEART_ANIMATION
		for (int i = 2; i<29; i++)
		{
			char filename[100];
			sprintf(filename, "/Users/jchu014/cmiss/api_test2/Data/test_%d.model.exnode",i);
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
		vector<Cmiss_texture**> vector_of_tex;
		
		vector<string>::const_iterator itr = sliceNames.begin();
		for (;itr != sliceNames.end();++itr)
		{
			const string& name = *itr;
			string dir_path(prefix);
			dir_path.append(name);
			
			loadImagePlane(name, command_data);
			
			Cmiss_texture** tex = loadTextureImagesAndTransformImagePlane(command_data,dir_path);
			vector_of_tex.push_back(tex);
		}
		
		HackyDataType hacky_data;
		hacky_data.command_data = command_data;
		hacky_data.vector_of_tex = vector_of_tex;
		hacky_data.sliceNames = sliceNames;
		
//		User_interface* ui = Cmiss_command_data_get_user_interface(command_data);
//		Event_dispatcher* ed = User_interface_get_event_dispatcher(ui);		
//		Event_dispatcher_add_idle_callback(ed, texture_animation, (void*)&hacky_data, EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
		
#define TIME_OBJECT_CALLBACK_TEST
#ifdef TIME_OBJECT_CALLBACK_TEST
		Time_object* time_object = create_Time_object("Texture_animation_timer");
		Time_object_add_callback(time_object,time_callback,(void*)&hacky_data);
		Time_object_set_time_keeper(time_object, time_keeper);
		Time_object_set_update_frequency(time_object,28);//BUG?? doesnt actually update 28 times -> only 27 
#endif		

#endif
		
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
