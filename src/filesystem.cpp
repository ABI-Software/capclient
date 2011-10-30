/*
 * FileSystem.cpp
 *
 *  Created on: Feb 15, 2009
 *      Author: jchu014
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
//#include <unistd.h>
#ifdef _MSC_VER
# include <direct.h>
# include <io.h>
extern "C"
{
# include "win32/linuxutils.h"
}
# define rmdir _rmdir
# define close _close
# define S_IFREG _S_IFREG
# define S_IFDIR _S_IFDIR
# define DIR_SEPERATOR '/'
#else
# include <unistd.h>
# define DIR_SEPERATOR '/'
#endif
#define MKS_TEMPLATE_NAME "CAPXXXXXX"

#include "capclientconfig.h"
#include "filesystem.h"

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
#ifdef _MSC_VER 
	int ret = _mkdir(dirname.c_str());
#else
	int ret = mkdir(dirname.c_str(), 0777);
#endif

	return (ret == 0);
}

bool FileSystem::RemoveFile(const std::string& filename)
{
	return remove(filename.c_str()) == 0;
}

bool FileSystem::DeleteDirectory(const std::string& dirname)
{
	return rmdir(dirname.c_str()) == 0;
}

bool FileSystem::FileExists(const std::string& filename)
{
	struct stat statBuf;

	if( stat( filename.c_str(), &statBuf ) < 0 ) 
		return false;

	return ((_S_IFREG & statBuf.st_mode) > 0);
}

bool FileSystem::DirectoryExists(const std::string& dirname)
{
	struct stat statBuf;

	if( stat( dirname.c_str(), &statBuf ) < 0 )
		return false;

	return ((_S_IFDIR & statBuf.st_mode) > 0);
}

std::string FileSystem::CreateTemporaryEmptyFile(const std::string& directory)
{
	int fd, totalSize = 0;
	char dirSep = '/';
	int tmplSize  = strlen( (const char *) MKS_TEMPLATE_NAME );
	totalSize += tmplSize;
	int dirSize = strlen(directory.c_str());
	if (dirSize > 0)
		totalSize += dirSize +1;
	char *tmpl = (char *) malloc ((size_t) ((sizeof(char)) * (totalSize + 1)));
	tmpl[totalSize] = '\0';
	if (dirSize == 0)
	{
		strncpy(tmpl, (const char *) MKS_TEMPLATE_NAME, tmplSize);
	}
	else
	{
		strncpy(tmpl, (const char *) directory.c_str(), dirSize);
		strncpy(tmpl + dirSize, (const char *) &dirSep, 1);
		strncpy(tmpl + dirSize + 1, (const char *) MKS_TEMPLATE_NAME, tmplSize);
	}
	if ((fd = mkstemp(tmpl)) < 0 ) {
		throw std::exception();//"Could not create temporary file!");
	} else {
		_close(fd);
	}
	std::string filename(tmpl);
	free(tmpl);

	return filename;
}

bool FileSystem::WriteCharBufferToFile(const std::string& filename, 
		unsigned char data[], unsigned int len)
{
	FILE *f = fopen(filename.c_str(), "wb");
	for (unsigned int i = 0; i < len; i++)
		fputc(data[i], f);

	return fclose(f) == 0;
}

std::string FileSystem::WriteCharBufferToString(unsigned char data[], unsigned int len)
{
	std::string out;
	for (unsigned int i = 0; i < len; i++)
		out += data[i];

	return out;
}

std::string FileSystem::GetFileName(const std::string& name)
{
	std::string temp = name;
	size_t found;
	found = temp.find_first_of('\\');
	while (found!=std::string::npos)
	{
		temp[found]='/';
		found = temp.find_first_of('\\', found+1);
	}
	
	if (temp.find_last_of(DIR_SEPERATOR) != std::string::npos)
		return temp.substr(temp.find_last_of(DIR_SEPERATOR) + 1);
	
	return name;
}

std::string FileSystem::GetFileNameWOE(const std::string& name)
{
	char extension_marker = '.';
	
	std::string filename = name;
	size_t found;
	found = filename.find_first_of('\\');
	while (found!=std::string::npos)
	{
		filename[found]='/';
		found = filename.find_first_of('\\', found+1);
	}
	
	if (filename.find_last_of(DIR_SEPERATOR) != std::string::npos)
		filename = filename.substr(filename.find_last_of(DIR_SEPERATOR) + 1);
	
	if (filename.find_last_of(extension_marker) != std::string::npos)
		return filename.erase(filename.find_last_of(extension_marker));
	
	return filename;
}

} // end namespace cap
