/*
 * CmguiManager.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: jchu014
 */

#include "CmguiManager.h"

CmguiManager* CmguiManager::instance = 0;

CmguiManager::CmguiManager(Cmiss_command_data* data)
	: commandData(data)
{
	assert(!instance);
	instance = this;
}
	
