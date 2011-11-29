

#ifndef _CMGUICALLBACKS_H
#define _CMGUICALLBACKS_H

extern "C"
{
#include <api/cmiss_time.h>
}

struct Scene_viewer;

namespace cap
{

/**
 * Input callback modelling setup.  This callback sets up the node tool and deals with any key
 * presses.  The node tool needs to know the output region and coordinate field for any created
 * nodes this information is best determined when the mouse is pressed when in modelling mode.
 * Key presses are dealt with here to acheive a better user experience.  Key presses understood
 * are 'e' for ending modelling mode, 'a' for accepting the current input for the current
 * modelling mode (this is dependent on which modelling mode you are currenly in) and 'd' which
 * deletes the currently selected node.
 *
 * @param	scene_viewer					The scene viewer.
 * @param [in,out]	input					If non-null, the input.
 * @param [in,out]	capclientwindow_void	If non-null, the capclientwindow void.  This may be
 *  cast to a CAPClientWindow pointer.
 *
 * @return	An integer 0 or 1, zero return values indicates not to continue processing anymore
 * 			callbacks.
 */
int input_callback_modelling_setup(Cmiss_scene_viewer_id scene_viewer, 
	struct Cmiss_scene_viewer_input *input, void *capclientwindow_void);

/**
 * Input callback modelling.  This callback passes the actions dealt with from the node tool to
 * the CAP Client window, this in turn passes it onto the modeller where the appropriate action
 * is taken.  Typically this is adding, moving or deleting a modelling point.
 *
 * @param	scene_viewer					The scene viewer.
 * @param [in,out]	input					If non-null, the input.
 * @param [in,out]	capclientwindow_void	If non-null, the capclientwindow void.  This may be
 *  cast to a CAPClientWindow pointer.
 *
 * @return	An integer 0 or 1, zero return values indicates not to continue processing anymore
 * 			callbacks.
 */
int input_callback_modelling(Cmiss_scene_viewer_id scene_viewer, 
	struct Cmiss_scene_viewer_input *input, void *capclientwindow_void);

/**
 * Input callback image shifting.  This callback facillitates image plane shifting by updating
 * the nodes from the change in mouse position for the selected surface (image).
 *
 * @param	scene_viewer					The scene viewer.
 * @param [in,out]	input					If non-null, the input.
 * @param [in,out]	capclientwindow_void	If non-null, the capclientwindow void.  This may be
 *  cast to a CAPClientWindow pointer.
 *
 * @return	An integer 0 or 1, zero return values indicates not to continue processing anymore
 * 			callbacks.
 */
int input_callback_image_shifting(Cmiss_scene_viewer_id scene_viewer, 
	struct Cmiss_scene_viewer_input *input, void *capclientwindow_void);

/**
 * Input callback control modifier switch.  Switch the control modifier so that the Cmgui node
 * tool operates in the opposite manner to normal.  That is the scene will be orientated when
 * the ctrl key is not pressed and will perform node tool actions when it is.  It is therefore
 * important to make sure that this callback comes before any other callback to acheive the
 * desired effect.
 *
 * @param	scene_viewer	 	The scene viewer.
 * @param [in,out]	input	 	If non-null, the input.
 * @param [in,out]	null_void	Always null in for this callback, needed to match the callback function parameter list.
 *
 * @return	An integer 0 or 1, zero return values indicates not to continue processing anymore
 * 			callbacks.
 */
int input_callback_ctrl_modifier_switch(Cmiss_scene_viewer_id scene_viewer, 
	struct Cmiss_scene_viewer_input *input, void *null_void);

/**
 * Callback, called when the time is changed.
 *
 * @param	time							The time notifier object.
 * @param	current_time					The current time.
 * @param [in,out]	capclientwindow_void	If non-null, the capclientwindow void.  This may be
 *  cast to a CAPClientWindow pointer.
 *
 * @return	An integer 0 or 1, zero return values indicates not to continue processing anymore
 * 			callbacks.
 */
int time_callback(Cmiss_time_notifier_id time, double current_time, void *capclientwindow_void);

}

#endif /* _CMGUICALLBACKS_H */
