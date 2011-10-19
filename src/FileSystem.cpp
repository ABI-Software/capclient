/*
 * FileSystem.cpp
 *
 *  Created on: Feb 15, 2009
 *      Author: jchu014
 */

#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#ifdef _MSC_VER
#include <direct.h>
# define mkdir _mkdir
#endif

#include "Config.h"
#include "FileSystem.h"

namespace cap
{

using namespace std;

FileSystem::FileSystem()
{
}

const std::vector<std::string> FileSystem::GetAllFileNames(const std::string& dirname)
{
	// It is better to construct the list here rather than in the constructor 
	// since the contents of the dir can change at any time between the constructor and here
	// (by other processes)
	
	//should we use boost FileSystem?
	std::vector<std::string> filenames;
	
	DIR *dir;
	struct dirent *ent;
	dir = opendir(dirname.c_str());
	if (!dir)
	{
		cout << "Error: can't open the directory: " << dirname << endl; //-- TODO: Add logging output window
	}
	else
	{
		while ((ent = readdir(dir)) != 0) {
			string filename(ent->d_name);
			if (filename[0] != '.') //takes care of ".", ".." and all other files that start with a . (e.g .DS_Store)
			{
//				cout << "Filename - " << ent->d_name <<endl;
				filenames.push_back(ent->d_name);
			}
		}
		
		closedir(dir);
	}
	
	return filenames;
}

bool FileSystem::MakeDirectory(const std::string& dirname)
{
#ifdef WIN32 
	int ret = mkdir(dirname.c_str());
#else
	int ret = mkdir(dirname.c_str(), 0777);
#endif

	return (ret == 0);
}

std::string FileSystem::GetFileName(const std::string& name)
{
	
	#ifdef WIN32 
	char directory_marker = '\\';
	#else
	char directory_marker = '/';
	#endif
	
	if (name.find_last_of(directory_marker) != std::string::npos)
		return name.substr(name.find_last_of(directory_marker) + 1);
	
	return name;
}

std::string FileSystem::GetFileNameWOE(const std::string& name)
{
	
	#ifdef WIN32 
	char directory_marker = '\\';
	#else
	char directory_marker = '/';
	#endif
	char extension_marker = '.';
	
	std::string filename = name;
	if (name.find_last_of(directory_marker) != std::string::npos)
		filename = name.substr(name.find_last_of(directory_marker) + 1);
	
	if (filename.find_last_of(extension_marker) != std::string::npos)
		return filename.erase(filename.find_last_of(extension_marker));
	
	return filename;
}

} // end namespace cap
