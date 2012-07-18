/*
 * platforminfo.cpp
 *
 *  Created on: Sep 8, 2010
 *      Author: jchu014
 */

#include "platforminfo.h"

namespace cap
{

std::string PlatformInfo::GetPlatform()
{
#ifdef DARWIN
	const std::string PLATFORM("Mac OS X");
#elif WIN32
	const std::string PLATFORM("Windows");
#else
	const std::string PLATFORM("Linux");
#endif

	return PLATFORM;
}

}

#ifdef DARWIN
#include <CoreServices/CoreServices.h>
//#include <Gestalt.h>
#include <sstream>
#include <stdint.h>

namespace cap
{

std::string PlatformInfo::GetOSVersion()
{
	int32_t major_version;
	int32_t minor_version;
	int32_t bugfix_version;
	Gestalt(gestaltSystemVersionMajor,
			reinterpret_cast<SInt32*>(&major_version));
	Gestalt(gestaltSystemVersionMinor,
			reinterpret_cast<SInt32*>(&minor_version));
	Gestalt(gestaltSystemVersionBugFix,
			reinterpret_cast<SInt32*>(&bugfix_version));
	
	std::stringstream ss;
	ss << major_version << "." << minor_version << "." << bugfix_version;
	return ss.str();
}

}

#elif WIN32
#include <windows.h>
#include <iostream>

namespace cap
{

std::string PlatformInfo::GetOSVersion()
{
	HKEY hKey; // Declare a key to store the result
	DWORD buffersize = 1024; // Declare the size of the data buffer
	char* lpData = new char[buffersize];// Declare the buffer

	RegOpenKeyEx (HKEY_LOCAL_MACHINE,
	"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",NULL,KEY_READ,&hKey);
	
	// Query the registry value
	RegQueryValueEx(hKey,"ProductName",NULL,NULL,(LPBYTE) lpData,&buffersize);
	
	// Print out the registry value
//	std::cout << "Registry Key Open: memory location=" << hKey << "\n";
//	std::cout << "Your Operating System is " << lpData << "\n\n";
	std::string version(lpData);
	
	// Close the Registry Key
	RegCloseKey (hKey);
	delete lpData;
	
	return version;   // Program successfully completed.
}

}
#else //Linux

#include <sys/utsname.h>

namespace cap
{

std::string PlatformInfo::GetOSVersion()
{
	struct utsname platformInfo;
	if (0 <= uname(&platformInfo))
	{
//		cout << "sysname = " << platformInfo.sysname << '\n';
//		cout << "nodename = " << platformInfo.nodename << "\n";
//		cout << "release = " << platformInfo.release <<"\n";
//		cout << "version = " << platformInfo.version << "\n";
//		cout << "machine = " << platformInfo.machine << "\n";
		return std::string(platformInfo.release);
	}
	return std::string();
}

}
#endif
