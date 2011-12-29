/*
 * SceneViewerPanel.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include <fstream>
#include <iostream>

#include <wx/app.h>

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_status.h>
#include <zn/cmiss_field_image.h>
#include <zn/cmiss_scene_viewer.h>
#include <zn/cmiss_stream.h>
#include <zn/cmiss_interactive_tool.h>
#include <zn/cmiss_rendition.h>
#include <zn/cmiss_graphic.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_field.h>
//#include <zn/cmiss_node.h>
#include <zn/cmiss_field_finite_element.h>
}

//#include "utils/debug.h" /* Uncomment this to enable debug messages */
#include "cmgui/sceneviewerpanel.h"
#include "cmgui/extensions.h"
#ifdef _MSC_VER
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#define new DEBUG_NEW
#endif

namespace cap
{

SceneViewerPanel::SceneViewerPanel(Cmiss_context_id cmissContext, const std::string& name, wxPanel* panel)
	: cmissSceneViewer_(Cmiss_context_create_scene_viewer(cmissContext, name, panel))
{
	SetFreeSpin(false);

	// The following crashes on my Windows 7 machine.
	//Cmiss_scene_viewer_set_transparency_mode(cmissSceneViewer_, CMISS_SCENE_VIEWER_TRANSPARENCY_ORDER_INDEPENDENT);
	//Cmiss_scene_viewer_set_transparency_layers(cmissSceneViewer_, 4);
}
	
SceneViewerPanel::~SceneViewerPanel()
{
#if defined _CAP_DEBUG_H
	dbg("SceneViewerPanel::~SceneViewerPanel()");
#endif
	Cmiss_scene_viewer_destroy(&cmissSceneViewer_);
}

void SceneViewerPanel::LookHere() const
{
	Cmiss_scene_viewer_set_lookat_parameters_non_skew(
		cmissSceneViewer_, 105.824, 164.155, 5.39987,
		11.3427, -2.31351, -10.3133,
		-0.0184453, 0.104329, -0.994372 );
	//Cmiss_scene_viewer_set_near_and_far_plane(cmissSceneViewer_, 1.92056, 686.342);
	Cmiss_scene_viewer_set_view_angle(cmissSceneViewer_, 0.220906);
	Cmiss_scene_viewer_set_viewport_mode(cmissSceneViewer_, CMISS_SCENE_VIEWER_VIEWPORT_RELATIVE);
	Cmiss_scene_viewer_set_viewing_volume(cmissSceneViewer_, -15.0613, 15.0613, -15.0613, 15.0613, 1.92056, 686.342);
}

void SceneViewerPanel::SetCallback(Cmiss_scene_viewer_input_callback callback, void *callback_class, bool addFirst) const
{
	Cmiss_scene_viewer_add_input_callback(cmissSceneViewer_, callback, callback_class, addFirst ? 1 : 0);
}

void SceneViewerPanel::RemoveCallback(Cmiss_scene_viewer_input_callback callback, void *callback_class) const
{
	Cmiss_scene_viewer_remove_input_callback(cmissSceneViewer_, callback, callback_class);
}

void SceneViewerPanel::SetInteractiveTool(const std::string& tool, const std::string& command) const
{
	Cmiss_scene_viewer_set_interactive_tool_by_name(cmissSceneViewer_, tool.c_str());
	if (command.size() > 0)
	{
		Cmiss_interactive_tool_id i_tool = Cmiss_scene_viewer_get_current_interactive_tool(cmissSceneViewer_);
		Cmiss_interactive_tool_execute_command(i_tool, command.c_str());
		Cmiss_interactive_tool_destroy(&i_tool);
	}
}

void SceneViewerPanel::LookingHere() const
{
	double dof, fd, eyex, eyey, eyez, lx, ly, lz, upx, upy, upz, va, left, right, bottom, top, np, fp;
	Cmiss_scene_viewer_get_depth_of_field(cmissSceneViewer_, &dof, &fd);
	Cmiss_scene_viewer_get_lookat_parameters(cmissSceneViewer_, &eyex, &eyey, &eyez, &lx, &ly, &lz, &upx, &upy, &upz);
	Cmiss_scene_viewer_get_view_angle(cmissSceneViewer_, &va);
	Cmiss_scene_viewer_get_viewing_volume(cmissSceneViewer_, &left, &right, &bottom, &top, &np, &fp);

#if defined _CAP_DEBUG_H
	dbg("dof : " + ToString(dof) + ", " + ToString(fd));
	dbg("eye : " + ToString(eyex) + ", " + ToString(eyey) + ", " + ToString(eyez));
	dbg("loo : " + ToString(lx) + ", " + ToString(ly) + ", " + ToString(lz));
	dbg("up  : " + ToString(upx) + ", " + ToString(upy) + ", " + ToString(upz));
	dbg("va  : " + ToString(va));
	dbg("vvo : " + ToString(left) + ", " + ToString(right) + ", " + ToString(bottom)+ ", " + ToString(top) + ", " + ToString(np) + ", " + ToString(fp));
#endif
}

void SceneViewerPanel::SetFreeSpin(bool on)
{
	Cmiss_scene_viewer_set_interactive_tool_by_name(cmissSceneViewer_, "transform_tool");
	Cmiss_interactive_tool_id i_tool = Cmiss_scene_viewer_get_current_interactive_tool(cmissSceneViewer_);
	if (on)
		Cmiss_interactive_tool_execute_command(i_tool, "free_spin");
	else
		Cmiss_interactive_tool_execute_command(i_tool, "no_free_spin");
	
	Cmiss_interactive_tool_destroy(&i_tool);
}

void SceneViewerPanel::SetViewingPlane(const ImagePlane& plane)
{
	// compute the center of the image plane, eye(camera) position and the up vector
	Point3D planeCenter =  plane.blc + (0.5 * (plane.trc - plane.blc));
	Point3D eye = planeCenter + (plane.normal * 500); // this seems to determine the near clip plane
	Vector3D up(plane.yside);
	up.Normalise();
	
	//Hack :: perturb direction vector a little
	eye.x *= 1.01; //HACK 1.001 makes the iso lines partially visible
	
	if (Cmiss_scene_viewer_set_lookat_parameters_non_skew(
		cmissSceneViewer_, eye.x, eye.y, eye.z,
		planeCenter.x, planeCenter.y, planeCenter.z,
		up.x, up.y, up.z) != CMISS_OK)
	{
		//Error;
	}
}

void SceneViewerPanel::SetViewingVolume(double radius)
{
	const double view_angle = 40.0, width_factor = 1.05, clip_factor = 10.0;
	double eye_distance, near_plane, far_plane;
	
	assert(cmissSceneViewer_);
	radius *= width_factor;
	eye_distance = sqrt(2.0)*radius/tan(view_angle*3.141592/360.0);
	
	far_plane = eye_distance+clip_factor*radius;
	near_plane = eye_distance-clip_factor*radius;
	if (clip_factor*radius >= eye_distance)
		near_plane = 0.01*eye_distance;
	
	Cmiss_scene_viewer_set_viewing_volume(cmissSceneViewer_, -radius, radius, -radius, radius, near_plane, far_plane);
	Cmiss_scene_viewer_redraw_now(cmissSceneViewer_);
}

void SceneViewerPanel::SetTumbleRate(double speed)
{
	Cmiss_scene_viewer_set_tumble_rate(cmissSceneViewer_, speed);
}

void SceneViewerPanel::ViewAll() const
{
	Cmiss_scene_viewer_view_all(cmissSceneViewer_);
}

} // end namespace cap
