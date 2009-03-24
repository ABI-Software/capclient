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

CmguiManager* CmguiManager::instance = 0;

CmguiManager::CmguiManager(Cmiss_command_data* data)
	: commandData(data)
{
	assert(!instance);
	instance = this;
}
	
Cmiss_scene_viewer_id CmguiManager::createSceneViewer(wxPanel* panel)
{
	sceneViewer = create_Cmiss_scene_viewer_wx(Cmiss_command_data_get_scene_viewer_package(commandData),
			panel,
			CMISS_SCENE_VIEWER_BUFFERING_DOUBLE,
			CMISS_SCENE_VIEWER_STEREO_ANY_MODE,
			/*minimum_colour_buffer_depth*/8,
			/*minimum_depth_buffer_depth*/8,
			/*minimum_accumulation_buffer_depth*/8);
	
	return sceneViewer;
}
