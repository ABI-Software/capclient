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

struct Cmiss_command_data;
class CmguiManager
{
public:
	CmguiManager(Cmiss_command_data* data);
	
	static CmguiManager& getInstance()
	{
		assert(instance);
		return *instance;
	}
	
	Cmiss_command_data* getCmissCommandData()
	{
		return commandData;
	}
	
private:
	static CmguiManager* instance;
	Cmiss_command_data* commandData;
};

#endif /* CMGUIMANAGER_H_ */
