/*
 * platforminfo.h
 *
 *  Created on: Sep 8, 2010
 *      Author: jchu014
 */

#ifndef PLATFORMINFO_H_
#define PLATFORMINFO_H_

#include <string>

namespace cap
{

class PlatformInfo
{
public:
	
	static std::string GetOSVersion();

	static std::string GetPlatform();
};

}

#endif /* PLATFORMINFO_H_ */
