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
{
	//should we use boost FileSystem?
	DIR *dir;
	struct dirent *ent;
	dir = opendir(path.c_str());
	if (!dir)
	{
		cout << "Error: can't open the directory: " << path << endl;
	}
	else
	{
		while ((ent = readdir(dir)) != 0) {
			cout << "Filename - " << ent->d_name <<endl;
			filenames.push_back(ent->d_name);
		}
		
		closedir(dir);
	}
	
	return;
}
