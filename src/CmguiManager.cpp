/*
 * CmguiManager.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include "CmguiManager.h"
#include "CmguiExtensions.h"

extern "C" {
#include "command/cmiss.h"
}

CmguiManager* CmguiManager::instance_ = 0;

CmguiManager::CmguiManager(Cmiss_context_id context)
	: contextID_(context)
{
	assert(!instance_);
	instance_ = this;
}
	
Cmiss_scene_viewer_id CmguiManager::createSceneViewer(wxPanel* panel)
{
	sceneViewer_ = Cmiss_scene_viewer_create_wx(Cmiss_context_get_default_scene_viewer_package(contextID_),
			panel,
			CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
			CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
			/*minimum_colour_buffer_depth*/8,
			/*minimum_depth_buffer_depth*/8,
			/*minimum_accumulation_buffer_depth*/8);
	
	return sceneViewer_;
}
