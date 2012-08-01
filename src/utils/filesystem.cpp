/*
 * FileSystem.cpp
 *
 *  Created on: Feb 15, 2009
 *      Author: jchu014
 */

#include <stdlib.h>
#if defined (_MSC_VER)
	#include <crtdbg.h>
#endif /* defined (_MSC_VER) */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
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
#include "logmsg.h"
#include "utils/debug.h"

namespace cap
{

    using namespace std;

	const std::vector<std::string> GetAllFileNames(const std::string& dirname)
	{
		//should we use boost FileSystem? No, we don't want the extra dependency.
		std::vector<std::string> filenames;
		
		DIR *dir;
		struct dirent *ent;
		dir = opendir(dirname.c_str());
		if (!dir)
		{
			LOG_MSG(LOGERROR) << "can't open the directory '" << dirname << "'";
            dbg("Error: can't open the directory: " + dirname);
		}
		else
		{
			while ((ent = readdir(dir)) != 0) {
				string filename(ent->d_name);
				if (filename[0] != '.' && IsFile(dirname + "/" + filename)) //takes care of ".", ".." and all other files that start with a . (e.g .DS_Store)
				{
					filenames.push_back(filename);
				}
			}
			
			closedir(dir);
		}
		
		return filenames;
	}

	const std::vector<std::string> GetAllFileNamesRecursive(const std::string& dirpath)
	{
		std::vector<std::string> filenames;
		
		DIR *dir;
		struct dirent *ent;
		dir = opendir(dirpath.c_str());
		if (!dir)
		{
			LOG_MSG(LOGERROR) << "can't open the directory '" << dirpath << "'";
			dbg("Error: can't open the directory: " + dirpath);
		}
		else
		{
			while ((ent = readdir(dir)) != 0) {
				string entityname(ent->d_name);
				if (entityname[0] != '.' && IsFile(dirpath + "/" + entityname)) //takes care of ".", ".." and all other files that start with a . (e.g .DS_Store)
				{
					filenames.push_back(entityname);
				}
				else if (entityname[0] != '.' && IsDirectory(dirpath + "/" + entityname))
				{
					std::vector<std::string> subfiles = GetAllFileNamesRecursive(dirpath + "/" + entityname);
					std::vector<std::string>::const_iterator cit = subfiles.begin();
					for (; cit != subfiles.end(); ++cit)
					{
						filenames.push_back(entityname + "/" + *cit);
					}
				}
			}
			
			closedir(dir);
		}
		
		return filenames;
	}

	bool MakeDirectory(const std::string& dirname)
	{
	#ifdef _MSC_VER 
		int ret = _mkdir(dirname.c_str());
	#else
		int ret = mkdir(dirname.c_str(), 0777);
	#endif

		return (ret == 0);
	}

	bool RemoveFile(const std::string& filename)
	{
		return remove(filename.c_str()) == 0;
	}

	bool DeleteDirectory(const std::string& dirname)
	{
		return rmdir(dirname.c_str()) == 0;
	}

	bool IsFile(const std::string& filename)
	{
		struct stat statBuf;

		if( stat( filename.c_str(), &statBuf ) < 0 ) 
			return false;

		return ((S_IFREG & statBuf.st_mode) > 0);
	}

	bool IsDirectory(const std::string& dirname)
	{
		struct stat statBuf;

		if( stat( dirname.c_str(), &statBuf ) < 0 )
			return false;

		return ((S_IFDIR & statBuf.st_mode) > 0);
	}

	std::string CreateTemporaryEmptyFile(const std::string& directory)
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
			close(fd);
		}
		std::string filename(tmpl);
		free(tmpl);

		return filename;
	}

	bool WriteCharBufferToFile(const std::string& filename, 
			unsigned char data[], unsigned int len)
	{
		FILE *f = fopen(filename.c_str(), "wb");
		for (unsigned int i = 0; i < len; i++)
			fputc(data[i], f);

		return fclose(f) == 0;
	}

	std::string WriteCharBufferToString(unsigned char data[], unsigned int len)
	{
		std::string out;
		for (unsigned int i = 0; i < len; i++)
			out += data[i];

		return out;
	}

	std::string GetFileName(const std::string& name)
	{
		if (IsDirectory(name))
			return "";
		
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

	std::string GetFileNameWOE(const std::string& name)
	{
		if (IsDirectory(name))
			return "";

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

	std::string GetPath(const std::string& path)
	{
		if (IsDirectory(path))
			return path;

		std::string dirpath = path;
		size_t found;
		found = dirpath.find_first_of('\\');
		while (found!=std::string::npos)
		{
			dirpath[found] = DIR_SEPERATOR;
			found = dirpath.find_first_of('\\', found+1);
		}

		do
		{
			found = dirpath.find_last_of(DIR_SEPERATOR);
			if (IsDirectory(dirpath.substr(0, found)))
				return dirpath.substr(0, found);

			dirpath = dirpath.substr(0, found);
		}
		while (found != std::string::npos);

		return "";
	}

} // end namespace cap
