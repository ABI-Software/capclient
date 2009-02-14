#define UNIX
#define DARWIN

//extern "C" {
//#include <math.h>
//#include <stdio.h>
//#include <general/time.h>
//#include "general/compare.h"
//#include "general/debug.h"
//#include "general/list.h"
//#include "general/indexed_list_private.h"
//#include "general/object.h"
//#include "user_interface/message.h"
//#include "user_interface/event_dispatcher.h"
//}
//
///* After the event_dispatcher.h has set up these variables */
//#include <wx/wx.h>
//#include <wx/apptrait.h>
//extern "C" {
//#include "user_interface/user_interface.h"
//#include "command/cmiss.h"
//}

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
   #include "wx/wx.h"
#endif

#include "ViewerFrame.h"
#include "DICOMImage.h"

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

// should go to cmiss.h
struct Material_package* Cmiss_command_data_get_material_package(
	struct Cmiss_command_data *command_data
);

Cmiss_scene_viewer_id create_Cmiss_scene_viewer_wx(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	wxPanel* panel,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified
<port> window handle.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_Carbon);
	/* Not implemented yet */
//	USE_PARAMETER(minimum_colour_buffer_depth);
//	USE_PARAMETER(minimum_accumulation_buffer_depth);
//	USE_PARAMETER(minimum_depth_buffer_depth);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_wx(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			panel,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth, (Graphics_buffer *)NULL);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_Carbon.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
}

int Viewer_view_all(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
{

	int return_code;


	struct Scene* scene = Scene_viewer_get_scene(scene_viewer);

	if (scene_viewer && scene)
	{
		return_code = 1;
		double centre_x,centre_y,centre_z,clip_factor,radius,
			size_x,size_y,size_z,width_factor;

		Scene_get_graphics_range(scene,
			&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z);
		radius = 0.5*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);

		double left, right, bottom, top, near_plane, far_plane;
		if (0 == radius)
		{
			/* get current "radius" from first scene viewer */
			Scene_viewer_get_viewing_volume(scene_viewer,
				&left, &right, &bottom, &top, &near_plane, &far_plane);
			radius = 0.5*(right - left);
		}
		else
		{
			/*???RC width_factor should be read in from defaults file */
			width_factor = 1.05;
			/* enlarge radius to keep image within edge of window */
			radius *= width_factor;
		}

		/*???RC clip_factor should be read in from defaults file: */
		clip_factor = 10.0;

		return_code = Scene_viewer_set_view_simple(
			scene_viewer, centre_x, centre_y, centre_z,
			radius, 40, clip_factor*radius);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_all.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}

int Cmiss_region_read_file_with_time(struct Cmiss_region *region, char *file_name, struct Time_keeper* time_keeper, float time)
/*******************************************************************************
LAST MODIFIED : 23 May 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_region *temp_region;
	struct IO_stream_package *io_stream_package;
	struct FE_import_time_index time_index;
	double maximum, minimum;
	time_index.time = time;

	ENTER(Cmiss_region_read_file);
	return_code = 0;
	if (region && file_name && (io_stream_package=CREATE(IO_stream_package)()))
	{
		temp_region = Cmiss_region_create_share_globals(region);
		if (read_exregion_file_of_name(temp_region,file_name,io_stream_package, &time_index))
		{
			if (Cmiss_regions_FE_regions_can_be_merged(region,temp_region))
			{
				return_code=Cmiss_regions_merge_FE_regions(region,temp_region);
				if (return_code)
				{
					/* Increase the range of the default time keepeer and set the
					   minimum and maximum if we set anything */
					maximum = Time_keeper_get_maximum(time_keeper);
					minimum = Time_keeper_get_minimum(time_keeper);
					if (time < minimum)
					{
						Time_keeper_set_minimum(time_keeper, time);
						Time_keeper_set_maximum(time_keeper, maximum);
					}
					if (time > maximum)
					{
						Time_keeper_set_minimum(time_keeper, minimum);
						Time_keeper_set_maximum(time_keeper, time);
					}
				}
			}
		}

		DEACCESS(Cmiss_region)(&temp_region);
		DESTROY(IO_stream_package)(&io_stream_package);
	}
	LEAVE;

	return(return_code);
}

void loadImagePlane(const string& name, Cmiss_command_data* command_data)
{
	char filename[256];
	const char* prefix = "/Users/jchu014/cmiss/api_test2/Data/";
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

	struct Cmiss_texture_manager* manager = Cmiss_command_data_get_texture_manager(command_data);
	struct IO_stream_package* io_stream_package = Cmiss_command_data_get_IO_stream_package(command_data);

	sprintf(filename, "%s%s/%s", prefix, name.c_str(), name.c_str()); // HACK FIX
	Cmiss_texture_id texture_id = Cmiss_texture_manager_create_texture_from_file(
		manager, name.c_str(), io_stream_package, filename);
	
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
	if (!Graphical_material_set_texture(material,texture_id))
	{
		//Error
		cout << "Error: Graphical_material_set_texture()" << endl;
	}

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

	// Now get the necessary info from the DICOM header
	//string DICOMFilename("/Users/jchu014/cmiss/api_test2/Data/68708398");
	ImagePlane* plane = getImagePlaneFromDICOMHeaderInfo(filename);

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

	char nodeName[256];
	sprintf(nodeName,"%d", nodeNum);
	Cmiss_node* node = Cmiss_region_get_node(region, nodeName);
	if (node) {
//		FE_node_set_position_cartesian(node, 0, plane->tlc.x, plane->tlc.y, plane->tlc.z);
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
//		FE_node_set_position_cartesian(node, 0, plane->trc.x, plane->trc.y, plane->trc.z);
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
//		FE_node_set_position_cartesian(node, 0, plane->blc.x, plane->blc.y, plane->blc.z);
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
//		FE_node_set_position_cartesian(node, 0, plane->brc.x, plane->brc.y, plane->brc.z);
		FE_node_set_position_cartesian(node, 0, plane->trc.x, plane->trc.y, plane->trc.z);
	}
	else
	{
		cout << nodeName << endl;
	}
}

int time_callback(struct Time_object *time, double current_time, void *user_data)
{
//	cout << "TIME!!! = " << current_time << endl;
	
	return 1;
}

Cmiss_texture** texture_animation_prepare(Cmiss_command_data* command_data)
{
	char* names[] = 
	{
		"68704743", 
		"68704758", 
		"68704773", 
		"68704788", 
		"68704803", 
		"68704818", 
		"68704833", 
		"68704848", 
		"68704863", 
		"68704878", 
		"68704893", 
		"68704908", 
		"68704923", 
		"68704938", 
		"68704953", 
		"68704968", 
		"68704983", 
		"68704998", 
		"68705013", 
		"68705028",
		"68705043", 
		"68705058", 
		"68705073", 
		"68705088", 
		"68705103", 
		"68705118", 
		"68705133", 
		"68705148"
	};
	
	Cmiss_texture** tex = new Cmiss_texture*[28];
	Cmiss_texture** pptr = tex;
	
	struct Cmiss_texture_manager* manager = Cmiss_command_data_get_texture_manager(command_data);
	struct IO_stream_package* io_stream_package = Cmiss_command_data_get_IO_stream_package(command_data);

	char filename[256];
	const char* prefix = "/Users/jchu014/cmiss/api_test2/Data/LA1/";
	for (int i = 0; i < 28; i++)
	{
		sprintf(filename, "%s%s", prefix,  names[i]); // HACK FIX
		Cmiss_texture_id texture_id = Cmiss_texture_manager_create_texture_from_file(
			manager, names[i], io_stream_package, filename);
		*pptr = texture_id;
		pptr++;
	}
	
	return tex;
}

struct HackyDataType
{
	Cmiss_texture** tex;
	Cmiss_command_data* command_data;
};

int texture_animation(void* data)
{
	static double prev_time = 0;
	double time;
	
	Cmiss_texture** tex = ((HackyDataType*)data)->tex;
	Cmiss_command_data* command_data = ((HackyDataType*)data)->command_data;
	
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

	Material_package* material_package = Cmiss_command_data_get_material_package(command_data);
	MANAGER(Graphical_material)* mm = Material_package_get_material_manager(material_package);
	
	//Graphical_material* material = FIND_BY_IDENTIFIER_IN_LIST(Graphical_material, name)
	//									("LA1", struct LIST(Graphical_material));
	
	Graphical_material* material;
	if (material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,
								name)("LA1",mm))
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
	if(!Cmiss_region_get_region_from_path(root_region, "LA1", &region))
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
	return 1;
}

int idle_callback(void* user_data)
{
	Scene_object* tk = (Scene_object*)user_data;
	
	static double prev_time;
	double time = Scene_object_get_time(tk);
	
	if(prev_time == time)
	{
		return 1;
	}
	
	//cout << time << endl;
	prev_time = time;
//	static int count = 0;
//	cout << "my count: " << count ++ <<endl;
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

		ViewerFrame *frame = new ViewerFrame(
			   wxT("Test Viewer"), 100, 100, 600, 600);
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
		{
			std::cout << "Error reading ex file - test_1.model.exnode, 0" << std::endl;
		}
		if (!Cmiss_region_read_file(region,"/Users/jchu014/cmiss/api_test2/Data/GlobalHermiteParam.exelem"))
		{
			std::cout << "Error reading ex file - exelem" << std::endl;
		}

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

		loadImagePlane("SA1", command_data);
		loadImagePlane("SA4", command_data);
		loadImagePlane("LA1", command_data);
		
		Time_object* time_object = create_Time_object("test");
		Time_object_add_callback(time_object,time_callback, 0);
		Time_object_set_time_keeper(time_object, time_keeper);
//		Time_object_set_update_frequency(time_object, 0.5);
		
		User_interface* ui = Cmiss_command_data_get_user_interface(command_data);
		Event_dispatcher* ed = User_interface_get_event_dispatcher(ui);
		
		//Event_dispatcher_add_idle_callback(ed, idle_callback, (void*)scene_object, EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
		
		Cmiss_texture** tex = texture_animation_prepare(command_data);
		HackyDataType hacky_data;
		hacky_data.command_data = command_data;
		hacky_data.tex = tex;
		Event_dispatcher_add_idle_callback(ed, texture_animation, (void*)&hacky_data, EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
		
		Time_object* time = Scene_object_get_time_object(scene_object);
		Time_object_set_update_frequency(time, 28);
		
		
		
#ifdef TEXTURE_TEST
		if (!Cmiss_region_read_file(region,"/Users/jchu014/cmiss/api_test2/Data/ImagePlane.exnode"))
		{
			std::cout << "Error reading ex file - ImagePlane.exnode" << std::endl;
		}
		if (!Cmiss_region_read_file(region,"/Users/jchu014/cmiss/api_test2/Data/ImagePlane.exelem"))
		{
			std::cout << "Error reading ex file - ImagePlane.exelem" << std::endl;
		}

		struct Cmiss_texture_manager* manager = Cmiss_command_data_get_texture_manager(command_data);
		struct IO_stream_package* io_stream_package = Cmiss_command_data_get_IO_stream_package(command_data);
		Cmiss_texture_id texture_id = Cmiss_texture_manager_create_texture_from_file(
			manager, "image_1", io_stream_package, "/Users/jchu014/cmiss/api_test2/Data/68708398");
		Graphical_material* material = create_Graphical_material("mat_1");
		if (!Graphical_material_set_texture(material,texture_id))
		{
			//Error
		}

		GT_element_group* gt_element_group;
		Cmiss_scene_viewer_package* scene_viewer_package = Cmiss_command_data_get_scene_viewer_package(command_data);
		struct Scene* scene = Cmiss_scene_viewer_package_get_default_scene(scene_viewer_package);
		if (scene)
		{
//			std::cout << "scene found" << std::endl;
		}

		GT_element_settings* settings;
		Cmiss_region* root_region = Cmiss_command_data_get_root_region(command_data);
		//Got to find the child region first!!
		if(!Cmiss_region_get_region_from_path(root_region, "ImagePlane", &region))
		{
			//error
			std::cout << "Cmiss_region_get_region_from_path() returned 0 : "<< region <<endl;
		}
		else
		{
//			std::cout << "root_region: " << root_region << ", region: " << region << endl;
		}

		//Following only applies when settings_name is specified using "as" in command line
//		if (gt_element_group = Scene_get_graphical_element_group(scene, region))
//		{
//			if (settings = first_settings_in_GT_element_group_that(
//				gt_element_group, GT_element_settings_has_name, (void *)""/*"surface"*/))
//			{
//				ACCESS(GT_element_settings)(settings);
//			}
//			else
//			{
//				std::cout << "No settings found!!" << std::endl;
//			}
//		}
//		else
//		{
//			std::cout << "No element group found!!" << std::endl;
//		}
		settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);

//		Material_package* material_package;
//		if (!GT_element_settings_get_material(settings))
//		{
//			GT_element_settings_set_material(settings,
//					Material_package_get_default_material(material_package));
//		}
//		if (!GT_element_settings_get_selected_material(settings))
//		{
//			GT_element_settings_set_selected_material(settings,
//				FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
//					"default_selected",
//					Material_package_get_material_manager(materical_package)));
//		}
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
				int delete_flag, int position);

			 if (!Cmiss_region_modify_g_element(region, scene,settings,
				/*delete_flag*/0, /*position*/-1))
			 {
				 //error
			 }
		}

		// Now get the necessary info from the DICOM header
		ImagePlane* plane = getImagePlaneFromDICOMHeaderInfo("/Users/jchu014/cmiss/api_test2/Data/68708398");

		if (!plane)
		{
			cout << "ERROR !! plane is null"<<endl;
		}
		else
		{
			cout << plane->tlc << endl;
		}

		Cmiss_node* node = Cmiss_region_get_node(region, "81");
		if (node) {
			FE_node_set_position_cartesian(node, 0, plane->tlc.x, plane->tlc.y, plane->tlc.z);
		}
		else
		{
			cout << "81!!!!!" << endl;
		}

		if (node = Cmiss_region_get_node(region, "82"))
		{
			FE_node_set_position_cartesian(node, 0, plane->trc.x, plane->trc.y, plane->trc.z);
		}
		else
		{
			cout << "82!!!!!" << endl;
		}

		if (node = Cmiss_region_get_node(region, "83"))
		{
			FE_node_set_position_cartesian(node, 0, plane->blc.x, plane->blc.y, plane->blc.z);
		}
		else
		{
			cout << "83!!!!!" << endl;
		}

		if (node = Cmiss_region_get_node(region, "84"))
		{
			FE_node_set_position_cartesian(node, 0, plane->brc.x, plane->brc.y, plane->brc.z);
		}
		else
		{
			cout << "84!!!!!" << endl;
		}
#endif //TEXTURE_TEST

		Cmiss_scene_viewer_id sceneViewer = create_Cmiss_scene_viewer_wx(Cmiss_command_data_get_scene_viewer_package(command_data),
				panel,
				CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
				CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
				/*minimum_colour_buffer_depth*/8,
				/*minimum_depth_buffer_depth*/8,
				/*minimum_accumulation_buffer_depth*/8);

		panel->GetContainingSizer()->SetMinSize(600, 600);
		panel->GetContainingSizer()->SetDimension(-1, -1, 600, 600);
		frame->GetSizer()->SetSizeHints(frame);

//		struct Scene* scene = Scene_viewer_get_scene(sceneViewer);
		//struct Time_keeper* time_keeper = Scene_get_default_time_keeper(scene); // get it directly from command_data instead

		Time_keeper_play(time_keeper,TIME_KEEPER_PLAY_FORWARD);
		Time_keeper_set_play_loop(time_keeper);
		Time_keeper_set_play_every_frame(time_keeper);

		Viewer_view_all(sceneViewer);

		Cmiss_command_data_main_loop(command_data);//app.OnRun()
		//DESTROY(Cmiss_command_data)(&command_data);
		return 1;
	}

	return 0;
} /* main */
