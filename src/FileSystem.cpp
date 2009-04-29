/*
 * FileSystem.cpp
 *
 *  Created on: Feb 15, 2009
 *      Author: jchu014
 */

#include "FileSystem.h"
#include <dirent.h>
#include <iostream>

using namespace std;

FileSystem::FileSystem(const string& path)
: dir_path(path)
{
}

const std::vector<std::string>& FileSystem::getAllFileNames()
{
	// It is better to construct the list here rather than in the constructor 
	// since the contents of the dir can change at any time between the constructor and here
	// (by other processes)
	
	//should we use boost FileSystem?
	if (!filenames.empty())
	{
		filenames.clear();
	}
	
	DIR *dir;
	struct dirent *ent;
	dir = opendir(dir_path.c_str());
	if (!dir)
	{
		cout << "Error: can't open the directory: " << dir_path << endl;
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
