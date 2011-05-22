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
#include <string>
#include <boost/tr1/memory.hpp>

extern "C" {
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_material.h"
#include "api/cmiss_context.h"
}

class wxPanel;
struct Scene_object;	

namespace cap
{

class CAPMaterial;

class CmguiManager
{
public:
	explicit CmguiManager(Cmiss_context_id context);
	
	Cmiss_context_id GetCmissContext() const
	{
		return cmissContext_;
	}
	
	Cmiss_scene_viewer_id CreateSceneViewer(wxPanel* panel, std::string const& sceneName = "") const;
	
	Cmiss_texture_id LoadCmissTexture(std::string const& filename) const;
	
	void ReadRectangularModelFiles(std::string const& modelName, std::string const& sceneName = "") const;
	
	/**
	 *  This method creates a cmiss material that uses shaders
	 */
	std::tr1::shared_ptr<CAPMaterial> CreateCAPMaterial(std::string const& materialName) const;
	
	// TODO move the following method out to a more suitable class
	Scene_object* AssignMaterialToObject(Cmiss_scene_viewer_id scene_viewer,
			Cmiss_material_id material, std::string const& regionName) const;

	void DestroyTexture(Cmiss_texture_id) const;
private:
	Cmiss_context_id cmissContext_;

};

} // end namespace cap
#endif /* CMGUIMANAGER_H_ */
