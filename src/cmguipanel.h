/*
 * cmguipanel.h
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#ifndef CMGUIPANEL_H_
#define CMGUIPANEL_H_

/** A singleton class to ease the interaction with the Cmgui API
 * 
 */
#include <cassert>
#include <string>
#include <vector>
#include <boost/tr1/memory.hpp>

extern "C" {
#include <configure/cmgui_configure.h>
#include <api/cmiss_scene_viewer.h>
#include <api/cmiss_graphics_material.h>
#include <api/cmiss_context.h>
#include <api/cmiss_field_module.h>
}

#include "DICOMImage.h"

class wxPanel;
//struct Scene_object;	

namespace cap
{

/**
 * \brief This class is an interface to wrap a wxPanel into a Cmgui scene.
 * This class contains a cmiss_scene_viewer handle so that it can control 
 * this scene for the context it belongs to.
 */
class CmguiPanel
{
public:
	/**
	 * Explicit constructor that initialises a cmgui scene with
	 * the givn name on the given panel.
	 * 
	 * \param cmissContext the handle to the cmiss context, unreferenced so don't destroy.
	 * \param name the name to give to the scene.
	 * \param panel the panel to display the scene on.
	 */
	explicit CmguiPanel(Cmiss_context_id cmissContext, const std::string& name, wxPanel* panel);
	
	/**
	 * Destroys Cmgui handles to the cmiss context and the cmiss scene viewer.
	 */
	~CmguiPanel();
	
	void LookHere() const;
	void LookingHere() const;

	/**
	 * Get the cmiss scene viewer for this panel.  Have to make this public so 
	 * that the callbacks can get a handle to it.
	 * 
	 * \returns the cmiss scene viewer for this panel.  Unaccessed.
	 */
	Cmiss_scene_viewer_id GetCmissSceneViewer() const
	{
		return cmissSceneViewer_;
	}

	/**
	 * Set the interactive tool spin on if true, otherwise turn free spin off.
	 * 
	 * \param on if true turn free spin on, otherwise turn free spin off.
	 * The default is on [true].
	 */
	void SetFreeSpin(bool on = true);
	
	/**
	 * Force a redraw of the scene now, used when manipulating widgets that change the
	 * scene and the effect should be seen immediately.
	 */
	void RedrawNow() const;
	
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
	/**
	 * Create the scene from the given context with the given name on the given panel.
	 * The returned handle to the scene viewer must be destroyed.
	 * 
	 * \param cmissContext the context to create the scene for.
	 * \param sceneName the name of the scene.
	 * \param panel the wxWidgets panel to view the scene on.
	 * \returns an accessed cmiss scene viewer.
	 */
	Cmiss_scene_viewer_id CreateSceneViewer(Cmiss_context_id cmissContext, const std::string& sceneName, wxPanel* panel) const;
	
	Cmiss_scene_viewer_id cmissSceneViewer_; /**< the scene viewer for this panel */

};

} // end namespace cap
#endif /* CMGUIPANEL_H_ */
