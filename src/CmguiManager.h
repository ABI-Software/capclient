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
#include "api/cmiss_context.h"
}

class wxPanel;

namespace cap
{

class CmguiManager
{
public:
	CmguiManager(Cmiss_context_id data);
	
	static CmguiManager& getInstance()
	{
		assert(instance_);
		return *instance_;
	}
	
	Cmiss_context_id getCmissContext() const
	{
		return contextID_;
	}
	
	Cmiss_scene_viewer_id createSceneViewer(wxPanel* panel);
	
	Cmiss_scene_viewer_id getSceneViewer() const
	{
		return sceneViewer_;
	}
	
private:
	static CmguiManager* instance_;
	Cmiss_context_id contextID_;
	Cmiss_scene_viewer_id sceneViewer_;
};

} // end namespace cap
#endif /* CMGUIMANAGER_H_ */
