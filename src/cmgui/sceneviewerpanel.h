/*
 * cmguipanel.h
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#ifndef CMGUIPANEL_H_
#define CMGUIPANEL_H_

#include <cassert>
#include <string>
#include <vector>

extern "C" {
#include <zn/cmgui_configure.h>
#include <zn/cmiss_scene_viewer.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_field_module.h>
}

#include "math/algebra.h"

class wxPanel;

namespace cap
{

/**
 * \brief This class is an interface to wrap a wxPanel into a Cmgui scene.
 * This class contains a cmiss_scene_viewer handle so that it can control 
 * this scene for the context it belongs to.
 */
class SceneViewerPanel
{
public:
	/**
	 * Constructor that initialises a cmgui scene with
	 * the givn name on the given panel.
	 * 
	 * \param cmissContext the handle to the cmiss context, unreferenced so don't destroy.
	 * \param name the name to give to the scene.
	 * \param panel the panel to display the scene on.
	 */
	SceneViewerPanel(Cmiss_context_id cmissContext, const std::string& name, wxPanel* panel);
	
	/**
	 * Destroys Cmgui handles to the cmiss context and the cmiss scene viewer.
	 */
	~SceneViewerPanel();

	/**
	 * Callback, sets the given callback .
	 *
	 * @param	callback	  	The callback function.
	 * @param	callback_class	(optional) [in,out] If non-null, the callback class.
	 * @param	addFirst	  	(optional) true to add callback first, false otherwise.
	 */
	void SetCallback(Cmiss_scene_viewer_input_callback callback, void *callback_class = 0, bool addFirst = false) const;

	/**
	 * Removes the callback described by callback.
	 *
	 * @param	callback	  	The callback.
	 * @param	callback_class	(optional) [in,out] If non-null, the callback class.
	 */
	void RemoveCallback(Cmiss_scene_viewer_input_callback callback, void *callback_class = 0) const;

	/**
	 * Sets an interactive tool.  The tool of the given name is set active with the optional
	 * given command string.
	 *
	 * @param	tool   	The tool.
	 * @param	command	(optional) the command.
	 */
	void SetInteractiveTool(const std::string& tool, const std::string& command = "") const;

	/**
	 * Look here.  Set the scene viewer to look at the pre-specified location.
	 * This is for looking at the CAPClient glyph icon.
	 */
	void LookHere() const;

	/**
	 * Looking here.  Helpful function for getting the current look at paramters.
	 */
	void LookingHere() const;

	/**
	 * Set the interactive tool spin on if true, otherwise turn free spin off.
	 * 
	 * \param on if true turn free spin on, otherwise turn free spin off.
	 * The default is on [true].
	 */
	void SetFreeSpin(bool on = true);
	
	/**
	 * Set the viewing volume to using the given radius.  Call the ViewAll() function
	 * first and then this will bring the scene slightly closer to the viewer(eye point).
	 * This is useful for viewing 2D planes or surfaces.
	 */
	void SetViewingVolume(double radius);

	/**
	 * Sets the viewing volume to centre on the given viewing plane.
	 *
	 * @param	plane	The plane to centre the view on.
	 */
	void SetViewingPlane(const ImagePlane& plane);
	
	/**
	 * Set tumble rate of the scene to the given speed.  Set the tumble rate to zero
	 * to prevent scene rotation, that is put it into a 2D mode.
	 * 
	 * \param speed the tumble rate
	 */
	void SetTumbleRate(double speed);
	
	/**
	 * Adjust the viewing volume so that everything in the scene can
	 * be viewed
	 */
	void ViewAll() const;

private:
	Cmiss_scene_viewer_id cmissSceneViewer_; /**< the scene viewer for this panel */

};

} // end namespace cap
#endif /* CMGUIPANEL_H_ */
