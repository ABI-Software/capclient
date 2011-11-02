

#ifndef _CMGUICALLBACKS_H
#define _CMGUICALLBACKS_H

extern "C"
{
#include <api/cmiss_time.h>
}

struct Scene_viewer;
struct Graphics_buffer_input;

namespace cap
{

int input_callback(Cmiss_scene_viewer_id scene_viewer, 
						  struct Cmiss_scene_viewer_input *input, void *viewer_frame_void);

int input_callback_image_shifting(Cmiss_scene_viewer_id scene_viewer, 
										 struct Cmiss_scene_viewer_input *input, void *viewer_frame_void);

int time_callback(Cmiss_time_notifier_id time, double current_time, void *user_data);

}

#endif /* _CMGUICALLBACKS_H */
