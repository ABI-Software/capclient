/*
 * CmguiManager.h
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#ifndef CMGUIMANAGER_H_
#define CMGUIMANAGER_H_

/** A singleton class to ease the interaction with the Cmgui API
 * 
 */
#include <cassert>

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_command_data.h"
}

struct Cmiss_command_data;
class wxPanel;

class CmguiManager
{
public:
	CmguiManager(Cmiss_command_data* data);
	
	static CmguiManager& getInstance()
	{
		assert(instance);
		return *instance;
	}
	
	Cmiss_command_data* getCmissCommandData() const
	{
		return commandData;
	}
	
	Cmiss_scene_viewer_id createSceneViewer(wxPanel* panel);
	
	Cmiss_scene_viewer_id getSceneViewer() const
	{
		return sceneViewer;
	}
	
private:
	static CmguiManager* instance;
	Cmiss_command_data* commandData;
	Cmiss_scene_viewer_id sceneViewer;
};

#endif /* CMGUIMANAGER_H_ */
